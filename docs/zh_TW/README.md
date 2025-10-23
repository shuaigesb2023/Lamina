# <img src="/assets/logo-icon.svg" width="10%"> Lamina

<img src="/assets/logo.svg" width="100%">

<div align="right">
  <strong>繁体中文</strong> | <a href="../zh_CN/README.md">简体中文</a> | <a href="../en_US/README.md">English</a>
</div>
<br>

[![GitHub issues](https://img.shields.io/github/issues/lamina-dev/Lamina)](https://github.com/Lamina-dev/Lamina/issues)
[![GitHub stars](https://img.shields.io/github/stars/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/forks)
[![GitHub contributors](https://img.shields.io/github/contributors/lamina-dev/Lamina?style=flat)](https://github.com/Lamina-dev/Lamina/graphs/contributors)
![GitHub last commit](https://img.shields.io/github/last-commit/lamina-dev/Lamina?style=flat)
[![License](https://img.shields.io/badge/license-GNUv3-blue.svg)](/LICENSE)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Math](https://img.shields.io/badge/math-precise-green.svg)](#精確數學)
[![QQ](https://img.shields.io/badge/QQ-Group-red?logo=qq&logoColor=white)](https://qm.qq.com/q/QwPXCgsJea)

## 一門專注於精確數學計算的程式化程式語言

[快速開始](#快速開始) • [語法指南](#基本語法) • [數學特性](#精確數學) • [程式碼範例](#程式碼範例) • [編譯指南](Compile.md) • [貢獻指南](CONTRIBUTING.md) • [Wiki](https://github.com/lamina-dev/Lamina/wiki) • [動態函式庫外掛程式開發](PLUGIN_GUIDE.md)

---

## 目錄

- [概述](#概述)
- [精確數學](#精確數學)
- [快速開始](#快速開始)
- [基本語法](#基本語法)
- [資料類型](#資料類型)
- [運算子](#運算子)
- [內建函式](#內建函式)
- [程式碼範例](#程式碼範例)
- [設計原則](#設計原則)

---

## 概述

**Lamina** 是一門用於數學計算的程式化語言，專注於為數學問題提供高效的解決方案，並具備圖靈完備性。其目標是提供簡潔、直觀的語法，同時支援複雜的數學操作。

### 主要特性

- **精確數學**：支援有理數（分數）和無理數（√、π、e）的精確表示與計算。
- **精度保護**：避免浮點數精度遺失；重複迴圈計算仍保持精確。
- **數學友善**：原生支援向量、矩陣運算及數學函式。
- **遞迴支援**：可設定的遞迴深度限制。
- **大整數**：任意精度的整數計算。
- **互動式 REPL**：支援互動式程式設計和指令稿執行。
- **智慧終端**：自動偵測終端機色彩支援，避免亂碼輸出。
- **堆疊追蹤**：完整的錯誤堆疊追蹤，方便偵錯。

---

## 精確數學

Lamina 的核心優勢在於其精確數學能力，解決了傳統語言中常見的浮點數精度遺失問題。

### 有理數

- **自動分數表示**：除法運算產生精確的分數。
- **精確的多次計算**：避免累積誤差。
- **自動簡化**：分數會自動簡化為最簡形式。

```lamina
var fraction = 16 / 9;           // 結果是 16/9，而不是 1.777...
var result = fraction;

// 多次迴圈計算仍保持精確
var i = 1;
while (i <= 10) {
    result = result * 9 / 9;     // 始終保持為 16/9
    i = i + 1;
}
```

### 無理數

- **符號表示**：√2、π、e 保持符號形式。
- **自動簡化**：√8 → 2√2, √(a²) → a
- **精確計算**：符號計算避免近似誤差

```lamina
var root2 = sqrt(2);             // 精確的 √2
var root8 = sqrt(8);             // 自動簡化為 2√2
var pi_val = pi();               // 精確的 π
var result = root2 * root2;      // 精確結果：2
```

### 精度比較範例

```lamina
// 傳統浮點數（其他語言）
// 0.1 + 0.2 = 0.30000000000000004

// Lamina 精確計算
var result = 1/10 + 2/10;        // 結果：3/10（完全精確）
```

---

## 快速開始

### 編譯與執行

```bash
# 初始化子模組（首次建置或更新後需要）
git submodule update --init --recursive

# 編譯
# 請參閱編譯指南

# 互動模式
./lamina

# 執行指令稿
./lamina script.lm
```

**注意**：Lamina 使用 libuv 函式庫進行網路通訊，該函式庫以 Git 子模組形式包含。在首次建置或程式碼更新後，務必執行上述的子模組初始化指令。

### 簡單範例

```lamina
// 精確數學計算
print("Hello, Lamina!");
var fraction = 16 / 9;          // 精確分數，非浮點數
print("16/9 =", fraction);      // 輸出：16/9

var root = sqrt(2);             // 精確無理數
print("√2 =", root);            // 輸出：√2
print("√2 × √2 =", root * root);// 輸出：2（精確結果）

// 向量運算
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];
print("v1 + v2 =", v1 + v2);
print("v1 · v2 =", dot(v1, v2));

// 精確常數
var pi_val = pi();
print("π =", pi_val);           // 輸出：π（符號）
```

---

## 基本語法

### 陳述式終止符

**分號必要**：Lamina 中的所有陳述式都必須以分號 `;` 結尾，這有助於提高解析效率和程式碼的可讀性。

```lamina
var x = 10;           //  正確
print(x);             //  正確
bigint big = 100!;    //  正確

var y = 20            //  錯誤：缺少分號
print(y)              //  錯誤：缺少分號
```

### 模組系統

Lamina 支援模組化程式設計，使用 `include` 陳述式來匯入檔案。

#### Include 陳述式

**引號必要**：`include` 陳述式必須使用引號來包圍檔案名稱，以確保語法的一致性和清晰度。

```lamina
include "math_utils";     //  正確：使用引號
include "lib/vectors";    //  正確：支援相對路徑

include math_utils;       //  錯誤：必須使用引號
```

#### 模組搜尋路徑

Lamina 會依序在以下路徑中搜尋模組檔案：

1. 目前目錄
2. `./` 目錄
3. `./include/` 目錄

如果檔案名稱缺少 `.lm` 副檔名，將會自動新增。

#### 內建模組

提供了一些特殊的內建模組：

- `include "splash";` - 顯示 Lamina 啟動畫面
- `include "them";` - 顯示開發者資訊與致謝

### 關鍵字

以下為保留字，不允許作為變數或函式名稱：

```plaintext
var func if else while for return break continue
print true false null include define bigint input
```

### 註解

```lamina
// 單行註解
/* 區塊註解 */
```

### 變數宣告

```lamina
var x = 10;
var name = "Alice";
var arr = [1, 2, 3];
bigint large_num = 999999999999;  // 大整數必須顯式宣告
bigint factorial_result = 25!;    // 可由計算結果賦值
```

---

## 資料類型

### 基本類型

| 類型 | 描述 | 範例 |
|---|---|---|
| `int` | 整數 | `42`, `-10` |
| `float` | 浮點數 | `3.14`, `-0.5` |
| `rational`| 有理數（精確分數） | `16/9`, `1/3` |
| `irrational`| 無理數（精確符號） | `√2`, `π`, `e` |
| `bool` | 布林值 | `true`, `false` |
| `string` | 字串 | `"Hello"`, `"world"` |
| `null` | 空值 | `null` |
| `bigint` | 大整數（需顯式宣告） | `bigint x = 999999;`, `bigint y = 30!;` |

### 複合類型

```lamina
// 陣列
var arr = [1, 2, 3];
var matrix = [[1,2], [3,4]];     // 2x2 矩陣

// 函式
func add(a, b) {
    return a + b;
}
```

---

## 運算子

### 算術運算子

| 運算子 | 功能 | 範例 |
|---|---|---|
| `+` | 加法 | `2 + 3 → 5` |
| `-` | 減法 | `5 - 2 → 3` |
| `*` | 乘法 | `4 * 3 → 12` |
| `/` | 除法（精確） | `5 / 2 → 5/2` |
| `%` | 取模 | `7 % 3 → 1` |
| `^` | 冪 | `2^3 → 8` |
| `!` | 階乘 | `5! → 120` |

### 向量/矩陣運算

```lamina
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];

print(v1 + v2);                 // 向量加法: [5, 7, 9]
print(dot(v1, v2));             // 點積: 32
print(2 * v1);                  // 純量乘法: [2, 4, 6]
```

### 比較運算子

`==` `!=` `>` `<` `>=` `<=`

---

## 內建函式

### 數學函式

| 函式 | 描述 | 範例 |
|---|---|---|
| `sqrt(x)` | 平方根（精確） | `sqrt(2) → √2` |
| `pi()` | 圓周率常數 | `pi() → π` |
| `e()` | 歐拉常數 | `e() → e` |
| `abs(x)` | 絕對值 | `abs(-5) → 5` |
| `sin(x)` | 正弦函式 | `sin(0) → 0` |
| `cos(x)` | 餘弦函式 | `cos(0) → 1` |
| `log(x)` | 自然對數 | `log(e()) → 1` |

### 向量/矩陣函式

| 函式 | 描述 | 範例 |
|---|---|---|
| `dot(v1, v2)` | 向量點積 | `dot([1,2], [3,4]) → 11` |
| `cross(v1, v2)` | 三維向量外積 | `cross([1,0,0], [0,1,0]) → [0,0,1]` |
| `norm(v)` | 向量模長 | `norm([3,4]) → 5` |
| `det(m)` | 矩陣行列式 | `det([[1,2],[3,4]]) → -2` |

### 工具函式

| 函式 | 描述 | 範例 |
|---|---|---|
| `print(...)` | 列印輸出 | `print("x =", 42)` |
| `input(prompt)` | 取得輸入 | `input("Name: ")` |
| `size(x)` | 取得大小 | `size([1,2,3]) → 3` |
| `fraction(x)` | 小數轉分數 | `fraction(0.75) → 3/4` |
| `decimal(x)` | 分數轉小數 | `decimal(3/4) → 0.75` |
| `visit()` | 存取 | `visit(a, 3, 0)` |
| `visit_by_str()` | 透過字串存取 | `visit_by_str(b, "name")` |

### 字串處理函式

| 函式 | 描述 | 範例 |
|---|---|---|
| `string_concat(...)` | 串接字串 | `string_concat("abc", "abc", "abc") → "abcabcabc"` |
| `string_char_at(str, index)`| 取得索引處的字元碼 | `string_char_at("abc", 1) → 98` |
| `string_length(str)` | 字串長度 | `string_length("abc") → 3` |
| `string_find(str, start_index, sub_str)` | 尋找子字串 | `string_find("abcAAA123", 0, "AAA") → 3` |
| `string_sub_string(str, start_index, len)` | 子字串 | `string_sub_string("abcAAA123", 3, 3) → "AAA"` |
| `string_replace_by_index(str, start_index, sub_str)` | 透過索引取代 | `string_replace_by_index("abcAAA123", 3, "CCC") → "abcCCC123"` |

---

## 程式碼範例

### 精確數學計算

```lamina
// 精確分數計算
print("=== 精確分數計算 ===");
var fraction = 16 / 9;           // 精確分數 16/9
print("16/9 =", fraction);

// 多次迴圈，保持精確
var result = fraction;
var i = 1;
while (i <= 5) {
    result = result * 9 / 9;     // 應保持為 16/9
    print("迴圈", i, ":", result);
    i = i + 1;
}

// 精確無理數計算
print("\n=== 精確無理數計算 ===");
var root2 = sqrt(2);             // 精確的 √2
var root3 = sqrt(3);             // 精確的 √3
print("√2 =", root2);
print("√3 =", root3);
print("√2 + √3 =", root2 + root3);
print("√2 × √3 =", root2 * root3);
print("√2 × √2 =", root2 * root2);

// 精確常數
var pi_val = pi();
var e_val = e();
print("π =", pi_val);
print("e =", e_val);
print("π + e =", pi_val + e_val);
```

### 分數與小數轉換

```lamina
// 小數轉分數
print("=== 小數轉分數 ===");
print("0.5 =", fraction(0.5));      // 1/2
print("0.25 =", fraction(0.25));    // 1/4
print("0.75 =", fraction(0.75));    // 3/4
print("0.125 =", fraction(0.125));  // 1/8
print("0.625 =", fraction(0.625));  // 5/8

// 分數轉小數
print("\n=== 分數轉小數 ===");
print("1/2 =", decimal(1/2));       // 0.5
print("3/4 =", decimal(3/4));       // 0.75
print("1/3 =", decimal(1/3));       // 0.333333
print("2/3 =", decimal(2/3));       // 0.666667

// 精確計算的優勢
print("\n=== 精確 vs. 浮點數 ===");
var float_result = 0.1 + 0.2;
var exact_result = fraction(0.1) + fraction(0.2);
print("浮點數: 0.1 + 0.2 =", float_result);
print("精確: 1/10 + 1/5 =", exact_result);
print("作為小數:", decimal(exact_result));
```

### 解一元二次方程式

```lamina
func quadratic(a, b, c) {
    var discriminant = b^2 - 4*a*c;
    if (discriminant < 0) {
        print("不支援複數根");
        return null;
    } else {
        var root1 = (-b + sqrt(discriminant))/(2*a);
        var root2 = (-b - sqrt(discriminant))/(2*a);
        return [root1, root2];
    }
}

var roots = quadratic(1, -3, 2);
print("根:", roots);  // 輸出 [2, 1]
```

### 大整數階乘

```lamina
// 直接計算
bigint result = 25!;
print("25! =", result);

// 遞迴版本
func factorial_big(n) {
    if (n <= 1) return 1;
    return n * factorial_big(n - 1);
}

bigint fact10 = factorial_big(10);
print("10! =", fact10);
```

### 向量與矩陣運算

```lamina
// 向量運算
var v1 = [1, 2, 3];
var v2 = [4, 5, 6];
var v3 = [1, 0, 0];
var v4 = [0, 1, 0];

print("向量加法:", v1 + v2);
print("點積:", dot(v1, v2));
print("外積:", cross(v3, v4));
print("模長:", norm(v1));

// 矩陣運算
var matrix = [[1, 2], [3, 4]];
print("行列式:", det(matrix));
```

### 模組化程式設計

```lamina
// 引入數學工具模組
include "math_utils";

// 引入向量計算模組
include "vectors";

// 使用模組中定義的函式
var result = advanced_calculation(10, 20);
var vec = create_vector(1, 2, 3);

// 顯示啟動畫面
include "splash";
```

---

## 設計原則

1. **簡潔性**：語法簡潔，遵循數學慣例
2. **數學友善**：直接支援向量、矩陣及常見數學運算
3. **精確計算**：精確的有理數與無理數表示，避免精度遺失
4. **程式化**：以函式組織程式碼，無複雜的物件模型
5. **圖靈完備**：支援無限迴圈、遞迴與任意計算
6. **使用者可設定**：遞迴深度、大整數類型需顯式設定

---

## 未來擴充

- **複數支援**：實作複數類型與運算
- **符號微積分**：支援符號微分與積分
- **語法糖**：支援分數字面值（例如 `16/9`）和無理數字面值（例如 `√5`）
- **並行計算**：多執行緒以加速計算
- **進階數學函式庫**：統計、數值分析等

---

## 錯誤處理與偵錯

### 堆疊追蹤支援

Lamina 提供完整的錯誤堆疊追蹤：

```text
Traceback (most recent call last):
  File "script.lm", line 12, in level1
  File "script.lm", line 8, in level2
  File "script.lm", line 4, in level3
RuntimeError: Undefined variable 'unknown_variable'
```

### 智慧終端支援

- **自動色彩偵測**：根據終端機能力啟用/停用色彩
- **NO_COLOR 支援**：遵循 NO_COLOR 環境變數標準
- **重導向友善**：當輸出重導向時停用色彩，避免亂碼

---

## 運算子優先級

`! > ^ > * / % > + - > == != > && ||`