# Step 6-7 — Adapter 架构设计 + API 映射表

**日期**: 2026-03-31
**状态**: PASS

## 目的

为 Hello World APK 设计最简 Adapter 架构：让**未修改的** Android Hello World APK 在 OH 上显示 "Hello World!"。定义 Android API → OH API 的映射规范。

## 架构决策

### 选型：应用级桥接（实用主义路线）

经对比 HanBing（系统级/10 JNI）和 Yue（应用级/26 API），**Hello World 采用应用级桥接**：

| 考量 | 系统级 (HanBing) | 应用级 (Yue) | 选择 |
|------|-----------------|-------------|------|
| 编译依赖 | 需整套 AOSP + OH 源码 | 仅需 OH SDK | **应用级** |
| 开发周期 | 需烧 ROM | hdc install 即可 | **应用级** |
| Hello World 够用？ | 过度设计 | 刚好够 | **应用级** |
| 后续可升级？ | — | 可逐步替换为系统级 | 是 |

**理由**：Hello World 只需 Activity 生命周期 + TextView 显示。应用级桥接 1 天可跑通，系统级需要 1 周编译环境。先证明概念，再升级。

### 架构图

```
┌────────────────────────────────────────────────┐
│  Android Hello World APK (classes.dex)         │
│  ┌──────────────────────────────────────────┐  │
│  │ MainActivity extends Activity            │  │
│  │   onCreate() → setContentView(R.layout)  │  │
│  │   R.layout.activity_main → LinearLayout  │  │
│  │     TextView "Hello World!"              │  │
│  │     TextView "Android running on ..."    │  │
│  └──────────────────────────────────────────┘  │
└───────────────────────┬────────────────────────┘
                        │ DEX 字节码解析
                        ▼
┌────────────────────────────────────────────────┐
│  Adapter Layer (OH 原生 ArkTS)                  │
│  ┌──────────────────────────────────────────┐  │
│  │ DexParser: 解析 APK → 提取布局和代码意图   │  │
│  │ ActivityBridge: 生命周期映射              │  │
│  │ ViewBridge: Android View → ArkUI 组件     │  │
│  │ ResourceBridge: R.layout → ArkUI Builder  │  │
│  └──────────────────────────────────────────┘  │
└───────────────────────┬────────────────────────┘
                        │
                        ▼
┌────────────────────────────────────────────────┐
│  OH Runtime (ArkTS + ArkUI)                     │
│  UIAbility + @Component → 屏幕渲染               │
└────────────────────────────────────────────────┘
```

### Hello World 最简路径

**不需要运行 DEX 字节码**。Hello World APK 的行为完全可预测：
1. 启动 → `MainActivity.onCreate()` → `setContentView(R.layout.activity_main)`
2. 布局 = LinearLayout(vertical, center) + 2 个 TextView

**策略**：静态解析 APK 的 AndroidManifest.xml + resources.arsc + layout XML，在 OH 侧生成等价 ArkUI 页面。

### 三层架构

| 层 | 职责 | Hello World 实现 |
|----|------|-----------------|
| **APK Parser** | 解析 APK zip → 提取 manifest、layout XML、字符串 | 读 AndroidManifest.xml 得到 launcher Activity；读 layout XML 得到 View 树 |
| **View Bridge** | Android View 树 → ArkUI 组件树 | LinearLayout→Column, TextView→Text, 属性映射（fontSize/textColor/gravity） |
| **Activity Bridge** | Activity 生命周期 → UIAbility 生命周期 | onCreate→onWindowStageCreate, onResume→onForeground, finish→terminateSelf |

## API 映射表

### Activity 生命周期映射

| Android API | OH API | 映射方式 |
|-------------|--------|---------|
| `Activity.onCreate(Bundle)` | `UIAbility.onCreate(Want, LaunchParam)` + `onWindowStageCreate(WindowStage)` | 拆分：非 UI 初始化→onCreate，UI 初始化→onWindowStageCreate |
| `Activity.onResume()` | `UIAbility.onForeground()` | 直接映射 |
| `Activity.onPause()` | `UIAbility.onBackground()` | 直接映射 |
| `Activity.onDestroy()` | `UIAbility.onDestroy()` | 直接映射 |
| `Activity.setContentView(int layoutId)` | `WindowStage.loadContent(pageName)` | 布局 ID → 页面名；XML 内容→ ArkUI @Builder |
| `Activity.finish()` | `UIAbilityContext.terminateSelf()` | 直接映射 |

### View/Layout 映射

| Android View | ArkUI 组件 | 属性映射 |
|-------------|-----------|---------|
| `LinearLayout(vertical)` | `Column()` | gravity→justifyContent/alignItems |
| `LinearLayout(horizontal)` | `Row()` | gravity→justifyContent/alignItems |
| `RelativeLayout` | `RelativeContainer()` | 规则→锚点约束 |
| `FrameLayout` | `Stack()` | — |
| `TextView` | `Text(content)` | textSize→fontSize, textColor→fontColor, textStyle→fontWeight |
| `Button` | `Button(label)` | onClick→onClick |
| `ImageView` | `Image(src)` | src→src, scaleType→objectFit |
| `EditText` | `TextInput()` | hint→placeholder |

### 属性映射（Hello World 用到的）

| Android 属性 | ArkUI 属性 | 转换 |
|-------------|-----------|------|
| `android:text="Hello World!"` | `.Text("Hello World!")` | 直接 |
| `android:textSize="36sp"` | `.fontSize(36)` | 去掉 sp 单位（ArkUI 默认 fp≈sp） |
| `android:textStyle="bold"` | `.fontWeight(FontWeight.Bold)` | 枚举映射 |
| `android:textColor="#000000"` | `.fontColor('#000000')` | 直接 |
| `android:layout_width="match_parent"` | `.width('100%')` | match_parent→'100%' |
| `android:layout_height="wrap_content"` | 默认行为 | wrap_content 是 ArkUI 默认 |
| `android:gravity="center"` | `.justifyContent(FlexAlign.Center)` + `.alignItems(HorizontalAlign.Center)` | 拆分为主轴+交叉轴 |
| `android:layout_marginTop="12dp"` | `.margin({ top: 12 })` | dp≈vp（ArkUI 默认单位） |
| `android:background="#FFFFFF"` | `.backgroundColor('#FFFFFF')` | 直接 |
| `android:orientation="vertical"` | `Column()` (而非 `Row()`) | 布局容器选择 |

## 实现计划

Phase 4 将实现以下三个模块（Step 8-10）：

### Step 8: Activity Bridge
- 创建 `AdapterAbility extends UIAbility`
- 接收 APK 路径参数
- 解析 AndroidManifest.xml 获取 launcher Activity 名称
- 在 `onWindowStageCreate` 中加载对应的桥接页面

### Step 9: View Bridge
- 解析 layout XML → 生成 ArkUI 组件树
- 实现 LinearLayout→Column/Row、TextView→Text 映射
- 处理属性转换（fontSize, textColor, margin 等）

### Step 10: 端到端集成
- 读取 `hello-world.apk` → 解析 → 桥接 → 显示
- 验证："Hello World!" 文字在 D200 屏幕上正确显示

## 遇到的问题

无阻塞问题。架构设计基于对 HanBing/Yue 两套方案的深度调研，关键决策：
1. **选应用级而非系统级**：Hello World 场景不需要 ART 运行时
2. **选静态解析而非动态执行**：APK 行为可预测，避免 DEX 解释器复杂度
3. **属性映射表先覆盖 Hello World 用到的 10 个属性**，后续按需扩展
