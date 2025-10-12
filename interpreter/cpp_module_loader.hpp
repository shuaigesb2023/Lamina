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
#include <dlfcn.h>
#include <link.h>  
#include <elf.h>

#ifndef STT_DEBUG
#define STT_DEBUG 0x11
#endif

#define DYLIB_HANDLE void*
#define DYLIB_LOAD(path) dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD)  // 加载后不重复加载
#define DYLIB_GETSYM(handle, name) dlsym(handle, name)
#define DYLIB_UNLOAD(handle) dlclose(handle)
#define DYLIB_ERROR() dlerror()
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

#ifndef _WIN32
// Linux/macOS：ELF符号解析回调（核心修复区）
static int elf_symbol_callback(struct dl_phdr_info* info, size_t size, void* data) {
    std::vector<std::string>* symbols = reinterpret_cast<std::vector<std::string>*>(data);
    if (!symbols) return 1;

    void* handle = dlopen(info->dlpi_name, RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) return 0;

    // 修复2：统一定义ELF类型 + 适配32/64位宏
#if defined(__x86_64__) || defined(_LP64)
    using Elf_Phdr = Elf64_Phdr;
    using Elf_Dyn = Elf64_Dyn;
    using Elf_Sym = Elf64_Sym;
    using Elf_Word = Elf64_Word;
    #define ELF_ST_BIND ELF64_ST_BIND  // 64位用ELF64宏
    #define ELF_ST_TYPE ELF64_ST_TYPE
#else
    using Elf_Phdr = Elf32_Phdr;
    using Elf_Dyn = Elf32_Dyn;
    using Elf_Sym = Elf32_Sym;
    using Elf_Word = Elf32_Word;
    #define ELF_ST_BIND ELF32_ST_BIND  // 32位用ELF32宏
    #define ELF_ST_TYPE ELF32_ST_TYPE
#endif

    // 遍历ELF段（外层循环变量i保留）
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const Elf_Phdr* phdr = &info->dlpi_phdr[i];
        if (phdr->p_type != PT_DYNAMIC) continue;

        // 修复3：32/64位动态段地址解析（区分d_val/d_ptr）
        Elf_Dyn* dyn = reinterpret_cast<Elf_Dyn*>(info->dlpi_addr + phdr->p_vaddr);
        Elf_Sym* symtab = nullptr;
        const char* strtab = nullptr;
        Elf_Word symcount = 0;

        for (; dyn->d_tag != DT_NULL; ++dyn) {
            switch (dyn->d_tag) {
                case DT_SYMTAB: 
                    // 64位用d_ptr，32位用d_val（转成uintptr_t再强转指针）
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

        // 修复4：内层循环变量改为j，避免与外层i重复
        for (Elf_Word j = 0; j < symcount; ++j) {
            const Elf_Sym* sym = &symtab[j];
            // 用适配后的ELF_ST_BIND/ELF_ST_TYPE宏，不再报错
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

    // 修复5：删除宏定义（避免污染其他代码）
#undef ELF_ST_BIND
#undef ELF_ST_TYPE

    dlclose(handle);
    return 0;
}

// Linux/macOS：触发符号遍历（逻辑不变）
std::vector<std::string> get_elf_dylib_symbols(const std::string& so_path) {
    std::vector<std::string> symbols;
    dl_iterate_phdr(elf_symbol_callback, &symbols);
    return symbols;
}
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
    all_symbols = get_elf_dylib_symbols(path);
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

