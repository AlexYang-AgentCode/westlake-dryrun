# WestlakeV3 — AI 自治适配引擎

> **一句话**: 在鸿蒙上跑未修改的 Android APK，全程 AI 自治，不需要大团队。
> **终态**: GitHub 开源 → 社区提 Issue → AI 自动修复 → 自动发版。

---

## 核心理念

```
人管意图 → AI Agent 协作 → 自行迭代 → 可运行 APK → GitHub 自治
```

### 不是什么
- 不是传统大团队项目（不需要 20 个工程师）
- 不是靠人力海量烧 ROM 调试
- 不是做完 3 个 Demo 就结束

### 是什么
- 1 个人管意图 + N 个 AI Agent 全自动执行
- 前 3 个 Demo（280 步）验证闭环 → 后 179 个 API 全自动铺开
- 终态：GitHub 开源项目，Issue 驱动自动进化

---

## 人提供的客观约束

| 约束 | 值 |
|------|-----|
| D200 开发板 | RK3568, OH V7.0, API 25, hdc |
| D600 开发板 | UIS7885, AOSP V15, adb |
| 共享内核 | **Linux 6.6** (OH V7 = AOSP V15，syscall ABI 一致) |
| 编译服务器 | GZ02 (16核/60G, Docker), GZ05 (16核/60G) |
| 轻量服务器 | GZ01, GZ03 (2核/4G), HK01 (国际) |
| 现有资产 | 系统级方案产物 (.so/.jar/补丁) + 应用级验证框架 (2476测试/Mock/VM) |
| JNI 字符串 | string_bridge (Wine H35 模式, 809行 RAII C 代码) |

---

## 终态愿景: GitHub 自治

```
                    ┌─────────────────────────────────────┐
                    │  GitHub: westlake-adapter            │
                    │                                     │
                    │  Issues ──→ AI Bot ──→ PR ──→ CI    │
                    │    ↑                         │      │
                    │    │         自动发版 ←───────┘      │
                    │    │                                │
                    │  社区用户提 Bug/Feature Request      │
                    └─────────────────────────────────────┘
```

**工作流**:
1. 用户提 Issue: "API X 在 APK Y 上不工作"
2. AI Bot 自动认领 → 分析 → 编写修复 → 生成测试
3. CI 跑通 → 自动 PR → LLM-as-Judge 审查
4. 合并 → 自动打 Tag → 自动 Release (.so + .jar 产物)
5. 用户拉取新版本 → 问题解决

**技术栈**: GitHub Actions + Claude Code Action + 自动测试 + 自动发版

---

## 三阶段路线

| 阶段 | 目标 | 步骤 | AI 自治度 |
|------|------|------|----------|
| **阶段 1** | 3 个 Demo 验证闭环 | 1-280 | 95% (人管意图) |
| **阶段 2** | 179 API 全覆盖 | 281-900 | 99% (人只看报告) |
| **阶段 3** | GitHub 开源自治 | 901+ | **100%** (Issue→修→发版) |

本计划详细覆盖**阶段 1**，阶段 2-3 粗略。

### 执行约束: 1人7天18小时

```
总资源: 1人 × 7天 × 18小时 = 126 小时
目标:   320 步 (Phase 0 → Demo 3)
速度:   ~2.5 步/小时 = ~45 步/天

人做什么:   意图确认 + 物理操作 (插USB/看报告/批准Gate)
AI做什么:   其余全部 (编码/编译/测试/审查/修复)
```

### 7 天时间表

| 日 | 小时 | Phase | 步骤 | 产出 |
|----|------|-------|------|------|
| **D1** | 18h | Phase 0 | 1-30 | 快照+Mock+编译管线+GitHub |
| **D2** | 18h | Demo 1 前半 | 31-75 | Logging+Activity+Window Bridge |
| **D3** | 18h | Demo 1 后半 | 76-130 | Rendering+全栈编译+Mock全通 |
| **D4** | 18h | Demo 2 | 131-220 | Calculator 可交互 |
| **D5** | 18h | Demo 3 前半 | 221-260 | MockDonalds+自愈管线搭建 |
| **D6** | 18h | Demo 3 后半 | 261-300 | 自愈演示+应用层hotfix+系统层 |
| **D7** | 18h | 真机+收尾 | 301-320 | 真机验证3Demo+领导材料 |

**关键加速**: AI 并行。人批准 Gate 后 AI 同时跑 3-5 路任务。
**人的时间分配**: 70% 看 AI 输出+批准 / 20% 物理操作 / 10% 意图调整。

---

## 3 个给领导的 Demo — 讲 3 个递进的故事

| Demo | 给领导看什么 | 步骤 | 冲击力 |
|------|-------------|------|--------|
| **HelloWorld** | "APK在鸿蒙上跑起来了" | 31-130 | 技术可行性 |
| **Calculator** | "能交互，21按钮可点" | 131-220 | 产品可用性 |
| **自愈全流程** | "发现不兼容→自动修→出补丁→出ROM" | 221-320 | **体系能力** |

### Demo 3 的设计哲学

Demo 1-2 展示的是"能跑"。Demo 3 展示的是**永动机在工作**。

**场景**: 一个真实 APK 在鸿蒙上运行，发现某个 API 不兼容（比如 SharedPreferences 的某个边界行为差异）。然后：

```
分钟 0    用户发现: "APP 存的数据重启后丢了"
          ↓ 提交 GitHub Issue
分钟 3    AI Bot 自动认领 → 分析 Issue → 定位到 Preferences.flush() 行为差异
          ↓
分钟 10   AI 生成应用层临时补丁 (Mock Preferences 内存实现)
          ↓ 自动 PR → CI 通过 → 合并
分钟 15   应用层 hotfix 发布 → 用户可立即使用 (hdc push .so)
          ↓
第 2 周   AI 完成系统层 Bridge 实现 (OH_Preferences → 真实文件持久化)
          ↓ 编译 → 全量回归 → 通过
          ↓ 新 ROM 产物发布
第 2 周+  用户刷新 ROM → 问题从根本解决
```

**这个 Demo 展示的不是一个 APP，是一个自治的研发体系。**

### 给领导的核心对比

| | 传统方案 | WestlakeV3 |
|---|---------|-----------|
| 发现 Bug | 人看→人分析→排期 | **AI 3分钟自动定位** |
| 临时方案 | 无，等排期 | **15分钟应用层 hotfix** |
| 根本方案 | 大团队开发数月 | **AI 2周自动出 ROM** |
| 团队 | 10-20 人 | **1人+AI** |
| 后续扩展 | 需持续招人 | **Issue 驱动自动进化** |
| 终态 | 人走项目停 | **GitHub 自治，永续运转** |

### Demo 需要的 API 子集

**Demo 1: HelloWorld (~25 API)**

| 组 | 数量 | 用途 |
|----|------|------|
| Logging | 2 | 调试 |
| Activity | 2 | 页面启动/关闭 |
| Window/Surface | 3 | 创建画面 |
| Canvas 绘制 | 12 | 绘制 UI |
| Font | 3 | 文字排版 |
| Bitmap | 1 | 画布载体 |
| ArkUI 初始化 | 1 | 初始化 |
| Preferences | 1 | 可选 |

**Demo 2: Calculator (+8 API)**

| 组 | 数量 | 用途 |
|----|------|------|
| Input 事件 | 2 | 按钮点击 |
| Canvas 扩展 | 4 | 按钮绘制 |
| Font 扩展 | 1 | 文字居中 |
| 资源释放 | 1 | 内存管理 |

**Demo 3: MockDonalds (+12 API)**

| 组 | 数量 | 用途 |
|----|------|------|
| Activity 多页面 | 2 | 5 页面导航 |
| Preferences | 3 | 购物车状态 |
| Canvas 扩展 | 3 | 商品图片 |
| ArkUI 扩展 | 2 | 列表动态添加 |
| Input 扩展 | 2 | 列表滚动 |

**3 Demo 合计: ~45 API (179 的 25%，覆盖最核心链路)**

---

## 从 3 个 Demo 倒推的依赖链

```
Demo 3 (自愈全流程, Step 221-320)
  需要: SharedPreferences 15 API (Bug 场景)
  需要: GitHub Actions + Claude Code Action (CI/CD)
  需要: 应用层 Mock ↔ 系统层 Bridge 双层切换能力
  需要: Demo 1+2 已通过 (证明全栈可用)
    ↑
Demo 2 (Calculator, Step 131-220)
  需要: Input 2 API (触摸)
  需要: Canvas 扩展 4 API (按钮)
  需要: Demo 1 已通过
    ↑
Demo 1 (HelloWorld, Step 31-130)
  需要: Logging 4 + Activity 3 + Window 6 + Canvas 12 + Font 3 + Bitmap 1 + ArkUI 1 = 30 API
  需要: 全栈编译通过 (bionic→Layer2→ART→framework.jar→bridge.so)
  需要: OH 补丁编译通过 (ability_runtime + appms + BMS + scene)
  需要: AOSP 补丁 (ActivityThread + ActivityManager)
  需要: appspawn-x 编译通过
    ↑
Phase 0 (基础设施, Step 1-30)
  需要: 设备快照 + Mock + 工具链 + string_bridge + verify + ralph-loop
```

**倒推得出: Phase 0 必须同时搞定 3 件事**
1. Mock 基础设施 (让 AI 不碰设备能工作)
2. 全栈编译管线 (让 .so/.jar 能产出)
3. GitHub 自动化骨架 (Demo 3 要用)

---

## Phase 0: 基础设施 — 三线并行 (Step 1-30)

> 让 AI 能自行工作。人最后一次碰硬件。
> **三线并行**: 设备采集 ∥ 编译管线验证 ∥ GitHub 自动化搭建

### 线 A: 设备采集 + Mock (Step 1-10)

| Step | 任务 | 执行者 |
|------|------|--------|
| 1 | D200 连接 + Linux 6.6 内核验证 | 人插USB, AI执行 |
| 2 | D600 连接 + Linux 6.6 内核验证 | 同上 |
| 3 | D200 全量系统快照 (20+ 项) | AI |
| 4 | D600 全量系统快照 (20+ 项) | AI |
| 5 | 双机 API 行为录制 (45 API: HelloWorld+Calc+Prefs) | AI |
| 6 | 双机行为自动对比 → diff 报告 | AI |
| 7 | mock-hdc + mock-adb (基于快照回放) | AI |
| 8 | API 行为回放引擎 | AI |
| 9 | 内核特性对比 (Linux 6.6 共享优势) | AI |
| 10 | **收起物理设备** | 人拔USB |

### 线 B: 编译管线 + 工具链 (Step 11-20) [与线A并行]

| Step | 任务 |
|------|------|
| 11 | adapter 项目目录 + Plugin 骨架 (15组) |
| 12 | build/config.sh (AOSP_ROOT, OH_ROOT, OH_CLANG 路径) |
| 13 | build.sh 入口 (--target=logging / --target=all) |
| 14 | string_bridge 编译验证 (x86 + aarch64) |
| 15 | 收集已有产物: .so + .jar + 补丁 → adapter/out/ |
| 16 | verify.py 统一验证 (符号/截图/行为/日志/AI审查) |
| 17 | ralph-loop.sh 自愈引擎 |
| 18 | confidence_gate.py 置信度门控 |
| 19 | deploy.sh (默认mock, --real时真机) |
| 20 | 编译管线端到端测试: 空.c → 编译 → .so → mock推送 |

### 线 C: GitHub 自动化骨架 (Step 21-25) [与线A/B并行]

> **这是 Demo 3 的前提**: Issue→AI修→PR→CI→发版

| Step | 任务 |
|------|------|
| 21 | 创建 GitHub 仓库 westlake-adapter (或本地 Git) |
| 22 | `.github/workflows/ci.yml` — PR 触发: 编译+Mock测试+SSIM |
| 23 | `.github/workflows/auto-fix.yml` — Issue 触发: AI分析→修复→PR |
| 24 | `.github/workflows/release.yml` — Tag 触发: 打包→Release |
| 25 | Issue 模板 + PR 模板 + CODEOWNERS |

### Gate (Step 26-30)

| Step | 任务 |
|------|------|
| 26 | Phase 0 Gate: Mock可用? 编译通? GitHub Actions通? |
| 27 | 3 Demo 验收标准定义 (倒推确认) |
| 28 | AI Agent 协作规则文档 |
| 29 | Demo 3 场景脚本编写 (SharedPreferences Bug 复现步骤) |
| 30 | 记录完成 → 进入 Demo 1 |

---

## Demo 1: HelloWorld (Step 31-130)

> 验证全链路: VM → Bridge → OH → 屏幕显示
> **100 步做 3 件事**: Bridge 开发 (30 API) + 全栈编译 + Mock 全通过

### 倒推: HelloWorld 需要什么全部编好

```
屏幕显示 "Hello from Android on OH!"
  ↑ appspawn-x fork进程 → ART VM → framework.jar → Activity Bridge → Canvas Bridge → 显示
  ↑ 需要已编译: appspawn-x + libart.so + framework.jar + liboh_adapter_bridge.so
  ↑ 需要已编译: libbionic_compat.so + Layer2 (10个.so) + OH补丁 (4个.z.so)
  ↑ 需要 30 个 API Bridge 实现: Logging(4) + Activity(3) + Window(6) + Canvas(12) + Font(3) + ArkUI(1) + Bitmap(1)
```

**Demo 1 = API Bridge 开发 + 全栈编译 + Mock 集成验证**
两条线并行:
- **Bridge 线**: 30 API Mock→实现→测试 (Step 31-80)
- **编译线**: bionic→Layer2→ART→framework→补丁→appspawn (Step 81-118, 复用已有产物)
- **集成**: 合并两线 → Mock 全栈运行 (Step 119-130)

### D1.1 Logging — 工具链验证 (Step 31-45)

4 个最简单的 API，验证编译→测试→集成管道。

| Step | 任务 | 说明 |
|------|------|------|
| 31 | AI 分工决策 | 首次多 Agent 协作 |
| 32 | Mock 实现 (printf) | 主机可运行 |
| 33 | Bridge 实现 (OH HiLog) | 使用 string_bridge RAII |
| 34 | Ralph Loop 编译 | 自愈: 失败→分析→修→重编 |
| 35 | 单元测试 (4 API × 4 用例) | 空串/正常/中文/emoji |
| 36 | API 行为对比 vs D600 基线 | JSON diff |
| 37 | LLM-as-Judge 审查 | AI 替代人审 |
| 38 | 集成到 .so | 链接 |
| 39 | Mock 推送验证 | mock-hdc |
| 40-42 | 并发/压测/内存检查 | 边界条件 |
| 43 | AI 回顾首次协作 | 自我改进 |
| 44 | 标记 4/25 API | 进度 |
| 45 | buffer | |

### D1.2 Activity — 核心复杂组 (Step 46-80)

3 API 但工作量最大: OH 补丁 + AOSP 补丁 + 状态机。

| Step | 任务 |
|------|------|
| 46 | AI 分工 (Bridge ∥ Mock 并行) |
| **startAbility (Step 47-62)** | |
| 47-50 | Mock: 生命周期状态机 + Intent→Want 转换 + 50 用例 |
| 51-53 | Bridge: oh_ability_manager_client.cpp + Ralph Loop |
| 54-55 | AOSP ActivityThread + ActivityManager 补丁 |
| 56-58 | OH ability_runtime + appms 补丁 + Docker 编译 |
| 59-62 | 验证: 行为对比 + nm 符号检查 + LLM-as-Judge |
| **terminateSelf (Step 63-65)** | |
| 63-65 | Mock + Bridge + 生命周期回调验证 |
| **checkPermission (Step 66-68)** | |
| 66-68 | Mock(GRANTED) + Bridge(AccessTokenKit) + 测试 |
| **集成 (Step 69-80)** | |
| 69-72 | 联合测试: start→运行→terminate, 多 Activity 栈 |
| 73-75 | 全对比 + LLM-as-Judge + 集成 |
| 76-78 | AI 回顾: 并行效率? Ralph Loop 表现? |
| 79-80 | 标记 7/25 API + buffer |

### D1.3 Window/Surface (Step 81-100)

6 API: surfaceCreate/Destroy/Resize/GetCanvas/Flush + arkuiInit

| Step | 任务 |
|------|------|
| 81 | AI 分工 |
| 82-86 | Mock (内存Canvas) + Bridge (XComponent+OH_NativeWindow) + 编译 |
| 87-90 | 其余 3 API + OH 补丁 (如需) |
| 91-95 | 集成测试: 全链路渲染 + PNG + SSIM 对比 |
| 96-98 | 行为对比 + LLM-as-Judge |
| 99 | 标记 13/25 API |
| 100 | buffer |

### D1.4 Rendering — 17 Canvas/Pen/Brush/Font API (Step 101-118)

Swarm 并行: 3 路 Agent 同时开发。

| Step | 任务 |
|------|------|
| 101 | AI 分工 → Swarm 3 路并行 |
| 102-107 | Canvas: create/drawColor/drawRect/drawText/save/restore/translate |
| 108-110 | Pen: create/setColor/setWidth + Brush: create/setColor |
| 111-113 | Font: create/setSize/measureText |
| 114 | 组合绘制: 背景+矩形+居中文字 = HelloWorld 画面 |
| 115 | PNG 渲染 → SSIM > 0.90 |
| 116-118 | 行为对比 + LLM-as-Judge + 集成 |

### D1.5 全栈编译线 (Step 81-118) [与 Bridge 线并行]

> 大量复用已有产物。已编译的直接用，未编译的 Ralph Loop 自愈编译。

#### Step 81-85: 验证已有产物完整性 🤖
- 81: libbionic_compat.so — readelf 验证 AArch64 + 符号完整
- 82: Layer 2 (10个.so) — 逐个 nm 验证
- 83: framework.jar — jar tf 验证类列表
- 84: OH 补丁 .z.so (4个) — readelf + nm (只增不减)
- 85: liboh_adapter_bridge.so — readelf 依赖链

#### Step 86-92: 补编缺失产物 🤖🔄 (Ralph Loop)
- 86: 如 ART .so 缺失/版本不对 → Docker 交叉编译
- 87: 如 dex2oat 缺失 → 编译
- 88: 如 libandroid_runtime.so 缺失 → 编译
- 89: 将 Demo 1 的 30 个 Bridge 函数链入 liboh_adapter_bridge.so
- 90: appspawn-x 编译 (如缺失)
- 91: appspawn_x.cfg 生成
- 92: 全栈链接验证 (ldd 模拟)

#### Step 93-100: AOSP/OH 补丁验证 🤖🔍
- 93: AOSP ActivityThread 补丁 — LLM-as-Judge 审查
- 94: AOSP ActivityManager 补丁 — LLM-as-Judge 审查
- 95: OH ability_runtime 补丁 — nm 只增不减
- 96: OH appms 补丁 — nm 只增不减
- 97: OH BMS 补丁 — nm 只增不减
- 98: OH SceneSessionManager 补丁 — nm 只增不减
- 99: 补丁冲突检查 (OH V7.0 API 25 vs 补丁目标版本)
- 100: 如需重编补丁 → Docker Ralph Loop

#### Step 101-110: 全栈产物组装 🤖
- 101: 创建完整 adapter/out/ 目录树
  ```
  out/
  ├── native/ (bionic + Layer2 + ART + android_runtime)
  ├── adapter/ (liboh_adapter_bridge.so + appspawn-x)
  ├── java/ (framework.jar)
  └── oh-services/ (4个补丁 .z.so)
  ```
- 102: 全量 readelf 依赖链验证 (无断链)
- 103: 全量 nm 符号覆盖验证 (30 个 Bridge JNI 函数存在)
- 104: HelloWorld.dex 编译
- 105: MockDonalds.dex 编译 (Demo 3 用，提前准备)
- 106: Calculator.dex 编译 (Demo 2 用)
- 107: 打包 deploy 包
- 108: Mock deploy 全流程测试 (mock-hdc)
- 109: deploy.sh --dry-run 验证
- 110: 编译线 Gate

#### Step 111-118: Mock 环境搭建 🤖
- 111: 模拟 /system/android/ 目录结构
- 112: 模拟 /system/lib64/platformsdk/ (补丁后)
- 113: 模拟 appspawn-x 启动 (x86 版本 --mock-vm)
- 114: 验证 BOOTCLASSPATH 设置正确
- 115: 验证 LD_LIBRARY_PATH 链完整
- 116: 主机 dalvikvm 能加载 framework.jar
- 117: 主机 dalvikvm 能执行简单 .dex
- 118: 编译+Mock 全部就绪 Gate

### D1.6 HelloWorld 全栈集成 (Step 119-130)

| Step | 任务 |
|------|------|
| 119 | 全栈 Mock 组装 |
| 120 | 主机 dalvikvm HelloWorld 运行 |
| 121 | Mock 渲染 → PNG |
| 122 | SSIM 对比 D600 基线 (> 0.85) |
| 123 | 生命周期日志验证 |
| 124 | 已有 2476 测试回归 (相关子集) |
| 125 | Confidence 评估 (全部 > 0.85) |
| 126 | AI 回顾 Demo 1 全过程 |
| 127 | **Demo 1 HelloWorld: VIRTUAL PASS ✅** |
| 128-130 | buffer |

---

## Demo 2: Calculator (Step 131-220)

> 验证: 输入事件 + 实时渲染 + AI 复用 Demo 1 经验加速

### D2.1 Input + 额外渲染 (Step 131-150)

| Step | 任务 |
|------|------|
| 131 | AI 分工 (参考 Demo 1 经验优化) |
| 132-138 | dispatchTouchEvent: Mock→Bridge→测试→21按钮命中 |
| 139-140 | dispatchKeyEvent |
| 141-145 | canvasDrawCircle/DrawRoundRect/DrawLine/ClipRect + fontGetMetrics |
| 146-150 | Calculator UI 布局 + 按钮→显示链路 + 算术测试 |

### D2.2 Calculator 全栈 (Step 151-190)

| Step | 任务 |
|------|------|
| 151-160 | 完整 Calculator 逻辑 (21按钮 + 表达式解析) |
| 161-175 | 测试矩阵 (41 用例: 21按钮 + 10算术 + 5边界 + 5序列) |
| 176-185 | 渲染验证: 每次点击截图 → AI Vision 确认 |
| 186-188 | Confidence + AI 回顾 (比 Demo 1 快了多少?) |
| 189 | **Demo 2 Calculator: VIRTUAL PASS ✅** |
| 190 | buffer |

### D2.3 自愈 buffer (Step 191-220)

30 步预留给 AI 自主处理意外。**人不干预。**

这是"自行迭代"能力的核心验证区域。
- 编译错误 → Ralph Loop
- 行为差异 → 分析+修复
- 渲染不一致 → 调整
- 任何新问题 → AI 自主解决

---

## Demo 3: 自愈全流程演示 (Step 221-320)

> **这不是展示一个 APP，是展示一个自治研发体系**
> 场景: APK 不兼容 → Issue → AI 自动修 → 应用层 hotfix → 系统层 ROM

### 场景设计

选一个**真实会遇到的不兼容问题**作为演示:

```
APK: MockDonalds 点餐应用 (5页面, 使用 SharedPreferences 存购物车)
不兼容: SharedPreferences.apply() 在 Android 异步写磁盘，
        但 Bridge 层当前只实现了内存 Mock → APP 重启后购物车清空
```

这个 Bug 足够真实（历史案例 H06 Mono 就遇到过行为一致性问题），
而且能完整展示"应用层快速补→系统层根本修"的双层响应机制。

### D3.1 搭建 Demo 环境 (Step 221-235)

#### Step 221-225: MockDonalds APK 准备 🤖
- 221: 构建 MockDonalds APK (5页面: 首页/菜单/详情/购物车/确认)
- 222: APK 使用 SharedPreferences 存购物车 (这是关键 API)
- 223: 在 Mock 环境运行 → 加入商品到购物车 → 购物车有数据
- 224: 模拟"重启 APP" → 购物车数据丢失 (Bug 复现!)
- 225: 截图记录: "购物车里有 3 个汉堡" → 重启 → "购物车空了"

#### Step 226-230: 搭建 GitHub Issue 自动化 🤖
- 226: 创建 `.github/workflows/auto-fix.yml` (Issue 触发 AI)
- 227: 创建 `.github/workflows/ci.yml` (PR 触发测试)
- 228: 配置 Claude Code Action 权限
- 229: 编写 Issue 模板 (API 不兼容报告格式)
- 230: 本地测试: 手动创建 Issue → 验证 workflow 触发

#### Step 231-235: 准备验证基础设施 🤖
- 231: 编写 Preferences 行为对比测试 (apply → 重启 → 数据在?)
- 232: 添加到 CI 测试矩阵
- 233: 准备应用层 Preferences Mock (ConcurrentHashMap + 文件序列化)
- 234: 准备系统层 Bridge 骨架 (OH_Preferences_* API)
- 235: Demo 环境 Gate

### D3.2 现场演示: Issue→应用层Hotfix (Step 236-255)

> **这部分是给领导看的"现场直播"**

#### Step 236: 🎬 演示开始 — 发现问题
在 D200 真机上运行 MockDonalds:
1. 打开 APP → 菜单 → 选汉堡 → 加入购物车 → 购物车显示 3 个汉堡
2. 关闭 APP → 重新打开
3. 购物车空了 ← **Bug!**
4. 截图证据

#### Step 237: 🎬 提交 Issue
```markdown
# Bug: SharedPreferences 数据重启后丢失

## 复现步骤
1. MockDonalds 添加商品到购物车
2. 关闭 APP
3. 重新打开 → 购物车为空

## 预期行为
购物车数据应该在 APP 重启后仍然存在

## API
SharedPreferences.apply() / SharedPreferences.getString()

## 截图
[购物车有数据] → [重启后为空]
```

#### Step 238: 🎬 AI Bot 自动认领 (分钟 0-3)
GitHub Actions 触发 → Claude Code Action:
1. 读取 Issue
2. 解析: 涉及 Preferences API (边界 7)
3. 查看 `plugins/06-preferences/api-spec.yaml`
4. 定位: `preferencesFlush` 当前是 no-op (Mock 未持久化)
5. 发 Comment: "已定位问题。Preferences.apply() 的 Mock 实现缺少文件持久化。开始修复。"

#### Step 239-243: 🎬 AI 生成应用层 Hotfix (分钟 3-15)
AI 自动:
- 239: 读取 Mock Preferences 代码 → 发现只是 ConcurrentHashMap
- 240: 添加文件序列化: HashMap → JSON → 写文件 → 重启时读回
- 241: 生成测试: 写入→"重启"(清内存)→读回→验证数据在
- 242: Ralph Loop 编译 + 测试通过
- 243: 提交 PR → CI 自动运行 → 全部通过

#### Step 244: 🎬 应用层 Hotfix 发布
PR 合并 → 自动打 Tag `v0.3.1-hotfix` → Release 产物:
```
liboh_adapter_bridge_preferences_hotfix.so (应用层快速修复)
```
用户 `hdc push` 即可立即使用，无需刷 ROM。

#### Step 245-248: 🎬 验证 Hotfix
- 245: hdc push hotfix .so 到 D200
- 246: 运行 MockDonalds → 加购物车 → 关闭 → 重开
- 247: **购物车数据还在!** → 截图证据
- 248: AI 在 Issue 评论: "应用层 hotfix 已发布。根本修复(系统层)将在 2 周内发布。"

#### Step 249-255: buffer + 回顾
- 249: 记录全流程时间线 (从 Issue 到 Hotfix: 15 分钟)
- 250-255: buffer

### D3.3 演示延续: 系统层根本修复 (Step 256-280)

> **这部分可以是后续汇报，或者现场加速演示**

#### Step 256-265: AI 开发系统层 Bridge 🤖
- 256: AI 读取 OH API 文档: `OH_Preferences_Open/Get/Put/Flush`
- 257: 编写 Bridge: JNI → OH_Preferences_* (真实文件系统)
- 258: 使用 string_bridge RAII (Wine 模式)
- 259: Ralph Loop 交叉编译 (aarch64)
- 260: 单元测试 (15 API × 3 用例 = 45 测试)
- 261: API 行为对比 vs D600 基线 (apply 后文件确实写入)
- 262: vs D200 基线 (OH_Preferences 的行为)
- 263: 差异分析: Android apply()异步 vs OH flush()同步 → Bridge 加异步包装
- 264: LLM-as-Judge 审查
- 265: 集成到主 .so

#### Step 266-272: 全量回归 🤖
- 266: Demo 1 HelloWorld 回归
- 267: Demo 2 Calculator 回归
- 268: MockDonalds 全流程 (含购物车持久化)
- 269: 已有 2476 测试回归
- 270: 全量 SSIM 对比
- 271: Confidence 全部 > 0.85
- 272: Gate 通过

#### Step 273-278: 新 ROM 产物发布 🤖
- 273: 将系统层 Bridge 编入 liboh_adapter_bridge.so (完整版)
- 274: 更新 OH 补丁 (如需)
- 275: Docker 编译全量产物
- 276: 自动打 Tag `v0.4.0` → Release:
  ```
  liboh_adapter_bridge.so (含 Preferences 系统层实现)
  framework.jar (更新)
  deploy.sh (一键部署)
  CHANGELOG.md (自动生成)
  ```
- 277: AI 在 Issue 评论:
  ```
  系统层修复已发布 v0.4.0。
  - 应用层 hotfix (v0.3.1): hdc push .so, 无需刷ROM
  - 系统层完整修复 (v0.4.0): 需刷ROM, SharedPreferences 完全兼容
  关闭 Issue。
  ```
- 278: Issue 自动关闭 ✅

#### Step 279-280: Demo 3 总结
- 279: **Demo 3 自愈全流程: PASS ✅**
  ```
  全流程时间线:
  分钟 0:   发现 Bug (购物车数据丢失)
  分钟 3:   AI 自动定位 (Preferences Mock 缺持久化)
  分钟 15:  应用层 Hotfix 发布 (用户立即可用)
  第 2 周:  系统层 ROM 发布 (根本解决)
  人工干预: 0 次 (仅提交了 1 个 Issue)
  ```
- 280: 生成领导汇报材料

---

## Phase I: 确认式真机验证 (Step 281-320)

> 代码已在 Mock 全部通过。上真机是"确认"不是"调试"。

| Step | 任务 | ROM? |
|------|------|------|
| 281 | 重新连接 D200 | |
| 282-286 | 推送全部产物 (.so + .jar + 补丁 + appspawn-x) | |
| 287 | 重启 | **ROM #1** |
| 288-290 | AI 健康检查 (ps/aa/bm/hidumper) | |
| 291-295 | HelloWorld: 安装→启动→AI截图→SSIM | |
| 296-300 | Calculator: 安装→点击1+2=→AI截图 | |
| 301-305 | MockDonalds: 安装→5页面导航→AI截图 | |
| 306-310 | 如有问题→远程 Ralph Loop | |
| 311 | 如需第二次重启 | **ROM #2 (仅在需要时)** |
| 312-318 | Phase I Gate + 最终报告 | |
| 319 | **3 Demo 真机验证完成 ✅** | |
| 320 | buffer | |

---

## 阶段 2: 179 API 全覆盖 (Step 321+) — 粗略

> **不需要大团队。Demo 1-3 验证的 AI 协作模式直接铺开。**

### 执行模式

```
对每个剩余 API (134 个):
  1. AI 读取 api-spec.yaml → 理解映射关系
  2. AI 生成 Mock 实现 (秒级)
  3. AI 生成 Bridge 实现 (使用 string_bridge)
  4. Ralph Loop 编译 (自愈)
  5. AI 生成测试 + 运行
  6. API 行为对比 vs 双机基线
  7. LLM-as-Judge 审查
  8. 集成
```

每个 API 约 4-8 步，134 API × 5步 ≈ **670 步**。
Swarm 5 路并行 → 实际时间 ÷5。

### 按优先级排列

| 批次 | API 组 | 数量 | 预计 |
|------|--------|------|------|
| Batch 1 | Preferences + Input | 18 | 完善 Demo 交互 |
| Batch 2 | Database + Network | 23 | 应用数据能力 |
| Batch 3 | Notification + Media | 24 | 系统能力 |
| Batch 4 | DeviceInfo + Sensor + Clipboard | 19 | 硬件抽象 |
| Batch 5 | ArkUI 扩展 + 其他 | 20 | 高级 UI |
| Batch 6 | 长尾 API + 边界条件 | 30 | 兼容性覆盖 |

### 质量保证

每个 Batch 完成后:
1. 全量 Mock 回归 (所有已完成 API)
2. 截图 SSIM 对比 (所有 Demo)
3. Confidence 评估
4. 真机增量验证 (hdc push .so, 无需 ROM)

---

## 阶段 3: GitHub 自治 (Step 900+) — 愿景

### 开源仓库结构

```
github.com/westlake-adapter/westlake
├── .github/
│   ├── workflows/
│   │   ├── ci.yml              # 每次 PR: 编译+Mock测试+SSIM
│   │   ├── auto-fix.yml        # Issue触发: AI分析→修复→PR
│   │   └── release.yml         # Tag触发: 打包.so/.jar→Release
│   └── ISSUE_TEMPLATE/
│       ├── bug_report.md       # "API X 在 APK Y 上不工作"
│       └── feature_request.md  # "请支持 API Z"
├── plugins/
│   ├── 01-logging/
│   ├── 02-activity/
│   ├── ...
│   └── 15-package/
├── infrastructure/
│   ├── string_bridge/
│   ├── verify.py
│   ├── ralph-loop.sh
│   └── mock-hdc.sh
├── tests/
│   ├── unit/                   # 537+ 单元测试
│   ├── behavior/               # 179 × 2 行为对比
│   └── e2e/                    # 23 Demo E2E
└── releases/
    └── v1.0.0/
        ├── liboh_adapter_bridge.so
        ├── framework.jar
        └── deploy.sh
```

### 自动化流水线

```yaml
# .github/workflows/auto-fix.yml
name: Auto Fix Issue
on:
  issues:
    types: [opened]
jobs:
  auto-fix:
    runs-on: ubuntu-latest
    steps:
      - uses: anthropics/claude-code-action@v1
        with:
          prompt: |
            分析 Issue 描述，确定哪个 API 有问题。
            1. 读取对应 plugin 的 api-spec.yaml
            2. 分析 Bridge 代码，定位 bug
            3. 修复 + 生成测试
            4. 运行 Mock 测试 + SSIM 对比
            5. 提交 PR
```

### 版本发布策略

| 版本 | 内容 | 触发 |
|------|------|------|
| v0.1 | Demo 1 HelloWorld | 阶段 1 完成 |
| v0.2 | Demo 2 Calculator | 阶段 1 完成 |
| v0.3 | Demo 3 MockDonalds | 阶段 1 完成 |
| v1.0 | 179 API 全覆盖 | 阶段 2 完成 |
| v1.x | Issue 驱动迭代 | 社区驱动 |

### APK 兼容性数据库 (Wine AppDB 模式)

```yaml
# apk-compat-db.yaml — 社区维护
apps:
  - name: "HelloWorld"
    rating: platinum    # 完美
    apis: [logInfo, startAbility, ...]
  - name: "Calculator"
    rating: gold        # 可用，小问题
    issues: ["按钮动画略慢"]
  - name: "MockDonalds"
    rating: gold
    issues: ["图片加载首次略慢"]
  - name: "某第三方APP"
    rating: silver      # 可用，较多问题
    issues: [...]
```

---

## "人管意图，AI 自行迭代" 验证矩阵

| 验证点 | Phase 0 | Demo 1 | Demo 2 | Demo 3 | 阶段2 | 阶段3 |
|--------|---------|--------|--------|--------|-------|-------|
| AI 分工 | — | 首次 | 复用 | 成熟 | 自动 | 自动 |
| 并行开发 | — | 2路 | 验证 | 3路 | 5路 | N路 |
| Ralph Loop | 框架 | 实战 | 减少 | 接近零 | 稳定 | CI集成 |
| LLM-as-Judge | 工具 | 首次 | 批量 | 信任 | 常规 | PR自动 |
| AI Vision | — | 截图 | 按钮 | 5页面 | 23Demo | 社区APK |
| 人干预 | USB | **0** | **0** | **0** | **0** | **0** |
| Issue→修→发 | — | — | — | — | — | **全自动** |

---

## 关键数据

| 指标 | 值 |
|------|-----|
| 总 API | 179 |
| 阶段 1 步数 | ~320 (详尽) |
| 阶段 2 步数 | ~670 (AI 自动) |
| 阶段 3 | 持续 (Issue 驱动) |
| Plugin 数 | 15 |
| 内核匹配 | Linux 6.6 (OH V7 = AOSP V15) |
| ROM 刷写 | ≤2 |
| 需要的人 | **1** (管意图) |
| AI Agent | N 个 (自动协作) |
| 终态 | GitHub 开源自治 |
