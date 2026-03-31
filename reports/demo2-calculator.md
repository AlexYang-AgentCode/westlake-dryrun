# Demo 2 — Calculator

**日期**: 2026-03-31
**状态**: PASS

## 目的

在 Demo 1 (静态 Hello World) 基础上，验证 Adapter 能处理：
- 多层嵌套布局 (LinearLayout 嵌套)
- Button 点击交互
- 动态 UI 更新 (@State)
- 四则运算逻辑

## 输出

| 产物 | 路径 | 说明 |
|------|------|------|
| Android APK | `calculator-android/build/apk/calculator.apk` | 13 KB, 16 按钮计算器 |
| D600 截图 | `calculator_d600.png` | Android 基准 |
| ArkTS 适配页 | `hello-world-adapted/.../AdaptedPage.ets` | 157 行，含完整计算逻辑 |
| D200 截图 | `calculator_d200.jpeg` | Adapter 版 |

### 新增 View/属性映射

| Android | ArkUI | 首次在 Demo 2 使用 |
|---------|-------|--------------------|
| LinearLayout(horizontal) | Row() | ✅ |
| Button | Button() | ✅ |
| layout_weight=1 | layoutWeight(1) | ✅ |
| padding | padding() | ✅ |
| layout_marginEnd | margin({right}) | ✅ |
| layout_marginBottom | margin({bottom}) | ✅ |
| gravity=end | textAlign(End) | ✅ |
| background (Button) | backgroundColor() | ✅ |
| onClick | onClick() + @State | ✅ |

### 视觉对比

| 维度 | D600 Android | D200 Adapter |
|------|-------------|-------------|
| 显示屏 | #333 背景, 白字, 右对齐 | 一致 |
| 运算符键 | 橙色 #FF9800 | 一致 |
| C 键 | 红色 #F44336 | 一致 |
| = 键 | 绿色 #4CAF50 | 一致 |
| 数字键 | 灰色 (Android 默认) | 蓝色 (ArkUI 默认) |
| 布局 | 4 列等宽, 4 行 | 一致 |
| 交互 | 可点击计算 | 可点击计算 |

数字键颜色差异是因为 Android 和 ArkUI 的默认 Button 背景色不同（Android 灰, ArkUI 蓝）。功能完全一致。

## 遇到的问题

### 1. javac: findViewById 返回 View 需要强转
- API 23 的 `findViewById` 没有泛型，需要 `(TextView) findViewById()`

### 2. hvigor daemon symlink 权限错误 (EPERM)
- 现象: hvigor 在 `C:\Users\.hvigor\project_caches\` 创建 symlink 失败
- 根因: Windows 非管理员用户无 symlink 权限, C 盘目录权限受限
- 解决: 设 `HVIGOR_USER_HOME=E:\..\.hvigor-home`，把缓存移到 E 盘

### 3. Calculator 逻辑需要手动注入
- 现象: 纯静态布局解析无法生成计算逻辑
- 解决: 布局从 APK 解析，逻辑从 Java 源码手动移植为 ArkTS
- 后续方向: DEX 字节码分析 → 自动生成 @State + onClick 逻辑
