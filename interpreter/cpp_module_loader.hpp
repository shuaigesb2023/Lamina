#pragma once
#include "lamina_api/value.hpp"
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>  // 新增：用于uintptr_t（32位地址转换需用）  // 关键修复1
#include <cstring>  // 用于 strstr/strcmp


// 平台宏适配
#ifdef _WIN32
#include <windows.h>
#include <imagehlp.h>  // Windows 符号表解析
#include <tlhelp32.h>
// 链接 ImageHlp 库（Windows 必需）
#pragma comment(lib, "imagehlp.lib")

#define DYLIB_HANDLE HMODULE
#define DYLIB_LOAD(path) LoadLibraryA(path.c_str())
#define DYLIB_GETSYM(handle, name) reinterpret_cast<void*>(GetProcAddress(handle, name))
#define DYLIB_UNLOAD(handle) FreeLibrary(handle)
#define DYLIB_ERROR() GetLastError()

#else

#ifndef STT_DEBUG
#define STT_DEBUG 0x11
#endif

#define DYLIB_HANDLE void*
#define DYLIB_LOAD(path) dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD)  // 加载后不重复加载
#define DYLIB_GETSYM(handle, name) dlsym(handle, name)
#define DYLIB_UNLOAD(handle) dlclose(handle)
#define DYLIB_ERROR() dlerror()

#endif

#ifndef _WIN32
#include <dlfcn.h>

// 新增：区分 Linux 和 macOS 头文件
#if defined(__linux__)
#include <elf.h>
#include <link.h>  // Linux 保留
#elif defined(__APPLE__)
#include <mach-o/dyld.h>  // macOS 用 Mach-O 相关头文件替代
#endif

// 目标函数指针类型
using ModuleFunc = Value (*)(const std::vector<Value>&);

#ifdef _WIN32
// Windows：获取动态库的所有导出符号名
inline std::vector<std::string> get_win_dylib_symbols(DYLIB_HANDLE hModule) {
    std::vector<std::string> symbols;
    if (!hModule) return symbols;

    // 获取模块文件路径
    char module_path[MAX_PATH] = {0};
    GetModuleFileNameA(hModule, module_path, MAX_PATH);

    // 初始化符号解析器
    LOADED_IMAGE loaded_img = {nullptr};
    if (!MapAndLoad(module_path, nullptr, &loaded_img, FALSE, TRUE)) {
        std::cerr << "Failed to map module: " << GetLastError() << std::endl;
        return symbols;
    }

    // 获取导出表（PE 文件结构：IMAGE_EXPORT_DIRECTORY）
    const PIMAGE_NT_HEADERS nt_headers = ImageNtHeader(loaded_img.MappedAddress);
    const auto export_dir =
        static_cast<PIMAGE_EXPORT_DIRECTORY>(
            ImageRvaToVa(nt_headers, loaded_img.MappedAddress, 
                        nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, 
                        nullptr)
        );
    if (!export_dir) {
        std::cerr << "No export directory found" << std::endl;
        UnMapAndLoad(&loaded_img);
        return symbols;
    }

    // 遍历导出符号名（AddressOfNames 是符号名的 RVA 数组）
    const auto* name_rvas = static_cast<DWORD*>(
        ImageRvaToVa(nt_headers, loaded_img.MappedAddress, export_dir->AddressOfNames, nullptr)
    );
    for (DWORD i = 0; i < export_dir->NumberOfNames; ++i) {
        auto sym_name = static_cast<const char*>(
            ImageRvaToVa(nt_headers, loaded_img.MappedAddress, name_rvas[i], nullptr)
        );
        if (sym_name && *sym_name != '\0') {
            symbols.emplace_back(sym_name);
        }
    }

    UnMapAndLoad(&loaded_img);
    return symbols;
}
#endif


std::vector<std::string> get_dylib_symbols(const std::string& lib_path);

// -------------------------- Linux 平台：保留原 ELF 解析逻辑 --------------------------
#if defined(__linux__)
// Linux：ELF 符号解析回调（复用你之前修复后的逻辑）
static int elf_symbol_callback(struct dl_phdr_info* info, size_t size, void* data) {
    std::vector<std::string>* symbols = reinterpret_cast<std::vector<std::string>*>(data);
    if (!symbols || info->dlpi_name == nullptr) return 1;

    // 只处理目标动态库（避免遍历所有系统库）
    if (std::string(info->dlpi_name) != lib_path) return 0;

    void* handle = dlopen(info->dlpi_name, RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) return 0;

    // ELF 类型适配（32/64位）
#if defined(__x86_64__) || defined(_LP64)
    using Elf_Phdr = Elf64_Phdr;
    using Elf_Dyn = Elf64_Dyn;
    using Elf_Sym = Elf64_Sym;
    using Elf_Word = Elf64_Word;
    #define ELF_ST_BIND ELF64_ST_BIND
    #define ELF_ST_TYPE ELF64_ST_TYPE
#else
    using Elf_Phdr = Elf32_Phdr;
    using Elf_Dyn = Elf32_Dyn;
    using Elf_Sym = Elf32_Sym;
    using Elf_Word = Elf32_Word;
    #define ELF_ST_BIND ELF32_ST_BIND
    #define ELF_ST_TYPE ELF32_ST_TYPE
#endif

    // 遍历 ELF 动态段（PT_DYNAMIC）
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const Elf_Phdr* phdr = &info->dlpi_phdr[i];
        if (phdr->p_type != PT_DYNAMIC) continue;

        Elf_Dyn* dyn = reinterpret_cast<Elf_Dyn*>(info->dlpi_addr + phdr->p_vaddr);
        Elf_Sym* symtab = nullptr;
        const char* strtab = nullptr;
        Elf_Word symcount = 0;

        // 提取符号表/字符串表地址
        for (; dyn->d_tag != DT_NULL; ++dyn) {
            switch (dyn->d_tag) {
                case DT_SYMTAB: 
#if defined(__x86_64__) || defined(_LP64)
                    symtab = reinterpret_cast<Elf_Sym*>(dyn->d_un.d_ptr);
#else
                    symtab = reinterpret_cast<Elf_Sym*>(static_cast<uintptr_t>(dyn->d_un.d_val));
#endif
                    break;
                case DT_STRTAB: 
#if defined(__x86_64__) || defined(_LP64)
                    strtab = reinterpret_cast<const char*>(dyn->d_un.d_ptr);
#else
                    strtab = reinterpret_cast<const char*>(static_cast<uintptr_t>(dyn->d_un.d_val));
#endif
                    break;
                case DT_SYMENT: 
                    symcount = phdr->p_memsz / sizeof(Elf_Sym);
                    break;
            }
        }

        if (!symtab || !strtab || symcount == 0) continue;

        // 筛选全局非调试符号
        for (Elf_Word j = 0; j < symcount; ++j) {
            const Elf_Sym* sym = &symtab[j];
            if (ELF_ST_BIND(sym->st_info) != STB_GLOBAL || 
                ELF_ST_TYPE(sym->st_info) == STT_DEBUG || 
                sym->st_name == 0) {
                continue;
            }
            const char* sym_name = strtab + sym->st_name;
            if (strstr(sym_name, "_init") || strstr(sym_name, "_fini")) continue;
            symbols->emplace_back(sym_name);
        }
    }

    #undef ELF_ST_BIND
    #undef ELF_ST_TYPE
    dlclose(handle);
    return 0;
}

// Linux：对外实现统一接口（调用 ELF 解析）
std::vector<std::string> get_dylib_symbols(const std::string& lib_path) {
    std::vector<std::string> symbols;
    // 遍历进程中已加载的 ELF 动态库，触发回调
    dl_iterate_phdr(elf_symbol_callback, &symbols);
    return symbols;
}

// -------------------------- macOS 平台：新增 Mach-O 解析逻辑 --------------------------
#elif defined(__APPLE__)
// macOS：全局变量（传递目标库路径和符号结果，适配 _dyld_iterate_imagees 回调）
static std::string s_target_lib_path;
static std::vector<std::string>* s_macho_symbols = nullptr;

// macOS：Mach-O 镜像遍历回调（替换 Linux 的 elf_symbol_callback）
static bool macho_image_callback(const struct mach_header* mh, uintptr_t vmaddr_slide, 
                                 const char* image_path, void* data) {
    // 1. 只处理目标动态库（跳过系统库和不匹配的库）
    if (!image_path || std::string(image_path) != s_target_lib_path) {
        return false; // 返回 false 继续遍历其他镜像
    }

    // 2. 打开目标动态库（获取可读写权限，用于解析符号表）
    void* handle = dlopen(image_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        std::cerr << "macOS dlopen failed: " << dlerror() << std::endl;
        return false;
    }

    // 3. 解析 Mach-O 加载命令，找到符号表（LC_SYMTAB）和字符串表
    const struct mach_header_64* mh64 = reinterpret_cast<const struct mach_header_64*>(mh);
    const struct load_command* lc = reinterpret_cast<const struct load_command*>(
        reinterpret_cast<const uint8_t*>(mh64) + sizeof(struct mach_header_64)
    );
    struct symtab_command* symtab_cmd = nullptr;

    // 遍历加载命令，找到符号表命令（LC_SYMTAB）
    for (uint32_t i = 0; i < mh64->ncmds; ++i) {
        if (lc->cmd == LC_SYMTAB) {
            symtab_cmd = reinterpret_cast<struct symtab_command*>(const_cast<struct load_command*>(lc));
            break;
        }
        // 跳到下一个加载命令
        lc = reinterpret_cast<const struct load_command*>(
            reinterpret_cast<const uint8_t*>(lc) + lc->cmdsize
        );
    }

    if (!symtab_cmd) {
        dlclose(handle);
        return false; // 无符号表，跳过
    }

    // 4. 提取符号表（nlist_t）和字符串表地址（需加上 ASLR 偏移 vmaddr_slide）
    const struct nlist_64* symtab = reinterpret_cast<const struct nlist_64*>(
        vmaddr_slide + symtab_cmd->symoff
    );
    const char* strtab = reinterpret_cast<const char*>(
        vmaddr_slide + symtab_cmd->stroff
    );
    const uint32_t symcount = symtab_cmd->nsyms;

    // 5. 筛选 Mach-O 符号（逻辑与 Linux 对齐：全局符号、非调试、非内部符号）
    for (uint32_t j = 0; j < symcount; ++j) {
        const struct nlist_64* sym = &symtab[j];
        // 条件：① 有符号名 ② 全局符号（N_EXT=1 + N_TYPE=N_UNDF/N_SECT/N_DATA 等）③ 非调试符号
        if (sym->n_un.n_strx == 0 || !(sym->n_type & N_EXT) || (sym->n_type & N_STAB)) {
            continue;
        }
        const char* sym_name = strtab + sym->n_un.n_strx;
        // 排除 Mach-O 内部符号（如 __mh_、_dyld_ 开头）
        if (strstr(sym_name, "__mh_") || strstr(sym_name, "_dyld_")) {
            continue;
        }
        s_macho_symbols->emplace_back(sym_name);
    }

    dlclose(handle);
    return true; // 找到目标库，返回 true 停止遍历
}

// macOS：对外实现统一接口（调用 Mach-O 解析）
std::vector<std::string> get_dylib_symbols(const std::string& lib_path) {
    std::vector<std::string> symbols;
    // 初始化全局变量（传递参数给回调）
    s_target_lib_path = lib_path;
    s_macho_symbols = &symbols;

    // 遍历 macOS 进程中已加载的 Mach-O 镜像，触发回调
    _dyld_iterate_imagees(macho_image_callback, nullptr);

    // 清空全局变量（避免污染）
    s_target_lib_path.clear();
    s_macho_symbols = nullptr;
    return symbols;
}
#endif

// -------------------------- 原代码中调用符号解析的地方：替换为统一接口 --------------------------
// （在 load_cppmodule 函数中，将原来的 get_elf_dylib_symbols 改为 get_dylib_symbols）
#endif

inline std::unordered_map<std::string, Value> load_cppmodule(const std::string& path) {
    std::unordered_map<std::string, Value> result;
    DYLIB_HANDLE handle = nullptr;
    std::vector<std::string> all_symbols;

    // 加载动态库
    handle = DYLIB_LOAD(path);
    if (!handle) {
        std::cerr << "Load module failed: " << DYLIB_ERROR() << std::endl;
        return result;
    }

    // 获取动态库的所有导出符号名
#ifdef _WIN32
    all_symbols = get_win_dylib_symbols(handle);
#else
    all_symbols = get_dylib_symbols(path); // 替换原来的 get_elf_dylib_symbols
#endif
    if (all_symbols.empty()) {
        std::cerr << "No exported symbols found in module" << std::endl;
        DYLIB_UNLOAD(handle);
        return result;
    }

    for (const auto& sym_name : all_symbols) {
        if (sym_name.starts_with("lamina_protect_")) continue;
        // 尝试解析为 Value 类型变量
        void* sym_addr = DYLIB_GETSYM(handle, sym_name.c_str());
        if (!sym_addr) continue;

        const auto* var_ptr = static_cast<Value*>(sym_addr);
        // Value 的 type 必须是合法值
        bool is_valid_value_var = (!var_ptr->is_lmCppFunction());
        if (is_valid_value_var) {
            result[sym_name] = *var_ptr;
            std::cout << "Found Value var: " << sym_name << std::endl;
            continue;
        }

        // 尝试解析为 "ModuleFunc 类型函数"（符号地址是函数指针）
        auto func_ptr = reinterpret_cast<ModuleFunc>(sym_addr);
        if (func_ptr != nullptr) {
            result[sym_name] = Value(std::make_shared<LmCppFunction>(func_ptr));
            std::cout << "Found ModuleFunc: " << sym_name << std::endl;
            continue;
        }


        std::cerr << "Skip unknown type symbol: " << sym_name << std::endl;
    }

    // 注意：动态库句柄需保留（后续调用函数依赖），建议在 VM 生命周期结束时统一卸载
    // DYLIB_UNLOAD(handle); 

    return result;
}

