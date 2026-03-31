# WestlakeV3 — 每个交付件的参照物映射

> 执行每个步骤前，先阅读对应参照物。不要凭空写代码。

---

## 参照物缩写索引

```
[H##]  = 19.4.Adapter-History/CASES.md 的历史案例 (共66个)
[B##]  = 19.3B.AI-Coding/ 的AI编程模式
[C##]  = 19.3C.MultiAgent-Automation/ 的多Agent模式
[D##]  = 19.3D.Decision-Reasoning/ 的决策推理模式
[E##]  = 19.3E.Analysis-Understanding/ 的分析理解模式
[F##]  = 19.3F.RAG-Knowledge/ 的检索知识模式
[W##]  = Wine 分析 (16.81-Oracale/L4-Adapter-History/)
[Y-xx] = Yue 实现 (16.13-Yue/)
```

### 文件路径速查

| 缩写 | 完整路径 |
|------|---------|
| CASES | `/mnt/e/10.Project/19.Document/19.4.Adapter-History/CASES.md` |
| PC-DEV | `/mnt/e/10.Project/19.Document/19.4.Adapter-History/PC-DEV-STRATEGY.md` |
| DUAL-LAB | `/mnt/e/10.Project/19.Document/19.4.Adapter-History/DUAL-DEVICE-LAB.md` |
| WINE-SUM | `/mnt/e/10.Project/16-WestLake/16.81-Oracale/L4-Adapter-History/ORACLE_SUMMARY.md` |
| WINE-ANA | `/mnt/e/10.Project/16-WestLake/16.81-Oracale/L4-Adapter-History/WINE_ANALYSIS.md` |
| STR-USE | `/mnt/e/10.Project/16-WestLake/16.81-Oracale/L4-Adapter-History/STRING_BRIDGE_USAGE.md` |
| str.h | `/mnt/e/10.Project/16-WestLake/16.81-Oracale/L4-Adapter-History/string_bridge.h` |
| str.c | `/mnt/e/10.Project/16-WestLake/16.81-Oracale/L4-Adapter-History/string_bridge.c` |
| Y-bridge | `/mnt/e/10.Project/16-WestLake/16.13-Yue/westlake/shim/bridge/` |
| Y-test | `/mnt/e/10.Project/16-WestLake/16.13-Yue/westlake/test-apps/` |
| Y-shim | `/mnt/e/10.Project/16-WestLake/16.13-Yue/westlake/shim/java/android/` |
| Y-art | `/mnt/e/10.Project/16-WestLake/16.13-Yue/art-universal/` |
| Y-db | `/mnt/e/10.Project/16-WestLake/16.13-Yue/westlake/database/api_compat.db` |
| Y-skills | `/mnt/e/10.Project/16-WestLake/16.13-Yue/A2OH-skills/` |
| Y-stub | `/mnt/e/10.Project/16-WestLake/16.13-Yue/westlake/scripts/aosp-stub-gen.py` |
| Y-orch | `/mnt/e/10.Project/16-WestLake/16.13-Yue/a2oh-orchestrator/ARCHITECTURE.md` |
| HB-build | `/mnt/e/10.Project/16-WestLake/16.12-HanBing/2026.03.25/build and deployment design.html` |
| AI-B | `/mnt/e/10.Project/19.Document/19.3B.AI-Coding/README.md` |
| AI-C | `/mnt/e/10.Project/19.Document/19.3C.MultiAgent-Automation/README.md` |
| AI-D | `/mnt/e/10.Project/19.Document/19.3D.Decision-Reasoning/README.md` |
| AI-E | `/mnt/e/10.Project/19.Document/19.3E.Analysis-Understanding/README.md` |

---

## Phase 0: 基础设施 (Step 1-30)

### 线A: 设备采集+Mock (Step 1-10)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 1-2 设备连接 | [H26 Anbox] binder 设备访问模型 | 理解 Android 在 Linux 上的设备层 |
| | [H27 Waydroid] namespace 隔离 | 理解轻量级隔离 vs 完整虚拟化 |
| 3-4 系统快照 | PC-DEV §Stage1 | Mock 层需要的 20+ 快照项清单 |
| | DUAL-LAB §采集协议 | 双机行为录制方法论 |
| 5-6 API行为录制 | [H35 Wine] wine-tests | API 行为回归测试的黄金标准 |
| | WINE-SUM:350-373 | Wine 的三级测试层次 |
| | [E08 截图对比] AI-E:111-122 | 截图 SSIM 基线采集方法 |
| 7-8 Mock引擎 | [H13 Emscripten] MEMFS | 虚拟 OS API 层设计 (最佳模板) |
| | PC-DEV §Stage1 mock层设计 | Mock OH API 的具体策略 |
| | Y-test/mock/ | **直接复用**: Yue 已有的 Mock OHBridge 实现 |
| 9 内核对比 | [H40 WSL] translation vs VM | 理解 Linux 6.6 共享的技术含义 |

### 线B: 编译管线+工具链 (Step 11-20)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 11 Plugin骨架 | [H12 Babel] 插件架构 | 15 个 Plugin 独立编译/测试的模板 |
| | WINE-ANA:324-334 | Wine DLL 模块化架构 |
| 12-13 build.sh | HB-build 全文 | 系统级编译设计（参考但不抄，因为是人力方案） |
| | Y-art/Makefile.ohos-arm64 | **直接复用**: ART 交叉编译 Makefile |
| 14 string_bridge | str.h 全文 (核心: L84-124结构体, L244-285获取函数) | 理解 RAII 栈缓冲模式 |
| | str.c 全文 (核心: L33-98 UTF-8→UTF-16) | 理解编码转换+代理对处理 |
| | WINE-SUM:119-135 | Wine 30 年为什么不需要缓存 |
| | STR-USE:187-198 | 性能统计代码 |
| 15 已有产物 | HB-build §产物清单 | 知道哪些 .so/.jar 已经存在 |
| 16 verify.py | [D07 Confidence] AI-D:109-120 | 置信度门控逻辑 |
| | [D05 LLM-as-Judge] AI-D:61-73 | AI 审查替代人审 |
| 17 Ralph Loop | [B11 Ralph Loop] AI-B:167-179 | 自愈编译循环模式 |
| | [C08 CI/CD Self-Healing] AI-C | 持续集成自愈 |
| 18 confidence_gate | [D07] + 17.3.20-confidence-auto-promote | 分数 >0.85 自动通过 |
| 20 端到端测试 | [H63 AWS Q Transform] | 工业级 AI 迁移的 6 步工作流 |

### 线C: GitHub 自动化 (Step 21-25)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 21-25 GitHub | [C08 CI/CD Self-Healing] AI-C | Issue→AI→PR→CI 工作流设计 |
| | [C09 Event-Driven] AI-C | GitHub webhook 事件驱动模式 |
| | reference_claude_code_ci_repos.md | 25 个 Claude Code CI 项目参考 |

---

## Demo 1: HelloWorld (Step 31-130)

### D1.1 Logging (Step 31-45) — 最简单，验证全流程

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 32 Mock实现 | Y-test/mock/OHBridge.java | **直接复用**: Yue 的 Mock println 实现 |
| 33 Bridge实现 | str.h L244-285 | RAII 字符串获取的标准用法 |
| | WINE-INSIGHTS.md §原则1 | A→W 转发: JNI→string_bridge→OH_LOG |
| | WINE-INSIGHTS.md §原则2 | 栈缓冲: 1KB覆盖 >90% |
| 34 Ralph Loop | [B11] ralph 4-stage: Spec→Plan→Code→QA | 编译失败自愈 |
| 35 单元测试 | [B06 Test Generation] AI-B | AI 生成测试用例 |
| | WINE-SUM:350-373 wine-tests | 边界: 空串/emoji/CJK/null |
| 36 行为对比 | DUAL-LAB §对比协议 | D600 Android 基线 vs Bridge 输出 |
| 37 LLM-as-Judge | [D05] AI-D:61-73 | AI 代码审查 prompt 设计 |

### D1.2 Activity (Step 46-80) — 最复杂

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 47-50 生命周期 | Y-shim/android/app/Activity.java | **直接复用**: Yue 的 MiniActivityManager 状态机 |
| | Y-skills/A2OH-LIFECYCLE.md (822行) | Activity→UIAbility 生命周期映射 |
| | [H34 BlackBerry 10] | 唯一手机上跑 Android 兼容层的先例 |
| | [H04 J2ObjC] JRE Emulation | Java→Native 桥接最相似问题 |
| 51-53 Bridge | str.h RAII模式 | Intent→Want: string_bridge 处理 Action/Data 字符串 |
| | WINE-INSIGHTS.md §原则3 | 两步转换: 先算 Want 字段数→预分配→填充→StartAbility |
| 54-55 AOSP补丁 | HB-build §AOSP补丁章节 | 了解 ActivityThread/ActivityManager 补丁位置 |
| | Y-stub (aosp-stub-gen.py) | **直接复用**: 自动生成缺失依赖的 stub |
| 56-58 OH补丁 | HB-build §OH补丁章节 | 了解 ability_runtime/appms 补丁 |
| | WINE-INSIGHTS.md §原则4 | nm 符号只增不减验证脚本 |
| 59-62 验证 | [E06 兼容性分析] AI-E | 行为差异分析方法 |

### D1.3 Window/Surface (Step 81-100)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 82-86 Surface | [H27 Waydroid] Virgl 渲染管线 | 图形 Surface 桥接设计 |
| | [H37 DXVK] 状态跟踪 | 渲染 API 状态管理 |
| | Y-bridge/OHBridge.java 的 surface 方法 | **参考**: 169个native方法中的Surface部分 |
| 91-95 SSIM | [E08 截图对比] AI-E:111-122 | SSIM >0.85 作为视觉验证阈值 |
| | [H36 Proton] ProtonDB | 视觉兼容性报告模式 |

### D1.4 Rendering (Step 101-118) — 47 API 的核心 17 个

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 101 Swarm并行 | [C05 Specialist Team] AI-C | 3路Agent分工: Canvas/Pen+Brush/Font |
| | [C01 Supervisor] AI-C | Oracle 监督多路 Agent |
| 102-113 绘制 | [H48 Android View↔Compose] | View Bridge 实现模式 |
| | [H21 React Native] JSI优化 | Bridge 性能优化 (批量调用/缓存) |
| | Y-skills/A2OH-JAVA-TO-ARKTS.md | Java→ArkTS 类型映射 (Canvas 参数) |
| | Y-db (api_compat.db) | **查询**: Canvas API 的 OH 映射和难度评分 |
| 114-115 组合渲染 | [H55 Rosetta 2] | 零感知延迟标准 |
| | [E08 SSIM] | 像素级对比: 背景+矩形+居中文字 |

### D1.5 全栈编译 (Step 81-118)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 81-85 产物验证 | WINE-INSIGHTS.md §原则4 | nm/readelf 符号验证标准 |
| 86-92 ART编译 | Y-art/Makefile.ohos-arm64 | **直接复用**: ART ARM64 交叉编译 |
| | Y-art/auto-build.sh | **直接复用**: 依赖检查+自动编译 |
| 93-100 补丁验证 | HB-build §补丁列表 | 4 个 OH 补丁 .z.so 的完整清单 |
| | WINE-INSIGHTS.md §原则4 | 只增不减 nm 验证脚本 |
| 101-110 组装 | [H12 Babel] 插件组合 | 15 Plugin 产物组装模式 |
| 111-118 Mock环境 | [H13 Emscripten] MEMFS | 模拟 /system/ 目录 |
| | PC-DEV §Stage2 | QEMU OH 环境搭建 |

### D1.6 HelloWorld 集成 (Step 119-130)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 120 dalvikvm 运行 | Y-art (ART已预编译) | **直接复用**: dalvikvm + boot image |
| 122 SSIM >0.85 | [E08] + [H36 ProtonDB] | 视觉对比标准 |
| 124 回归 | Y-test/02-headless-cli/HeadlessTest.java | **参考**: 2476 测试回归子集 |
| 125 Confidence | [D07] | 全项 >0.85 才 PASS |

---

## Demo 2: Calculator (Step 131-220)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 131 经验复用 | [D04 Self-Reflection] AI-D | AI 自我回顾 Demo 1 加速 Demo 2 |
| 132-140 Input | [H48 Compose Interop] | 触摸事件双向传递 |
| | Y-bridge/OHBridge.java Input方法 | **参考**: Yue 的 Input 桥接 |
| | Y-skills/A2OH-LIFECYCLE.md §事件分发 | 触摸事件 Android→OH 映射 |
| 141-145 扩展渲染 | [H21 RN] 批量Bridge | 21 个按钮 = 批量绘制优化 |
| 151-160 Calculator逻辑 | Y-test/04-mockdonalds/ | **参考**: 完整 E2E 测试 app 的模式 |
| 161-175 测试矩阵 | [B06 Test Generation] | AI 自动生成 41 用例 |
| | [H35 Wine] wine-tests | 边界条件: 除零/溢出/连续运算 |
| 176-185 AI Vision | [E08 截图对比] | 每次点击截图→Claude Vision→确认 |
| 191-220 自愈buffer | [B11 Ralph Loop] | 30步完全自治，人不干预 |

---

## Demo 3: 自愈全流程 (Step 221-320)

### D3.1 环境搭建 (Step 221-235)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 221-222 MockDonalds | Y-test/04-mockdonalds/ | **直接复用**: Yue 已有完整 MockDonalds 源码 |
| | Y-shim/android/content/SharedPreferences.java | **参考**: Preferences shim |
| 223-225 Bug复现 | [H06 Mono] | 行为一致性问题的历史教训 |
| | [H35 Wine] AppDB | 兼容性问题的分类体系 |
| 226-230 GitHub自动化 | [C08 CI/CD Self-Healing] AI-C | Issue→AI→PR 工作流 |
| | [C09 Event-Driven] | webhook 触发模式 |
| | reference_claude_code_ci_repos.md | 25 个实际 CI 项目 |
| 231-235 测试基础设施 | [B06 Test Generation] | Preferences 行为对比测试 |
| | Y-skills/A2OH-DATA-LAYER.md | SharedPreferences→OH Preferences 映射 |

### D3.2 Issue→应用层Hotfix (Step 236-255)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 236 Bug发现 | [H35 Wine] AppDB Platinum/Gold/Silver | 兼容性评级体系 |
| 237 Issue提交 | [C09 Event-Driven] | Issue 格式→自动解析 |
| 238 AI认领 | [C08] + [B10 Debug Assistant] AI-B | AI 自动定位 Bug 的模式 |
| 239-243 Hotfix | [H63 AWS Q Transform] | 工业级: Analyze→Transform→Test→Repair |
| | [B03 Code Transformation] AI-B | Android→OH 代码转换模式 |
| | [B11 Ralph Loop] | 编译+测试自愈循环 |
| 244 发布 | [C08] release 自动化 | Tag→Release→产物 |
| 245-248 验证 | PARALLEL-SCHEDULE §hdc push | 免ROM推送验证 |

### D3.3 系统层根本修复 (Step 256-280)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 256-258 Bridge | str.h RAII + WINE §原则1-3 | Preferences Bridge 标准模式 |
| | Y-skills/A2OH-DATA-LAYER.md | SharedPreferences 15 API 完整映射 |
| | Y-bridge/OHBridge.java Preferences方法 | **参考**: 已有的 Preferences native 方法 |
| 260 测试 | [H35 wine-tests] | 15 API × 3 用例 边界条件 |
| 263 行为差异 | [E06 Compatibility Analysis] AI-E | apply()异步 vs flush()同步 差异分析 |
| | [D06 API Mapping Decision] AI-D | 映射决策: 加异步包装? |
| 266-272 全量回归 | [H35 Wine] 回归套件 | 每次新API不能破坏已有API |
| 273-278 ROM产物 | [C08] release 全流程 | 自动打Tag→Release→CHANGELOG |

### Phase I: 真机验证 (Step 281-320)

| Step | 参照物 | 为什么看 |
|------|--------|---------|
| 282-286 推送 | PARALLEL-SCHEDULE §hdc push表 | 哪些文件push到哪个路径 |
| 291-310 3Demo验证 | [E08 截图对比] | AI Vision 截图确认 |
| | [D07 Confidence] | 全项 >0.85 才最终 PASS |
| 306-310 远程修复 | [B11 Ralph Loop] | 真机问题→远程自愈 |

---

## 核心参照物分级

### 必读 (执行前必须看完)

| 文档 | 适用范围 | 预计阅读时间 |
|------|---------|-------------|
| str.h + str.c (809行) | **全部 179 API** | 30 min |
| WINE-INSIGHTS.md (139行) | **全部 15 Plugin** | 15 min |
| WINE-SUM §119-135 (无需缓存) | 架构决策 | 5 min |
| Y-bridge/OHBridge.java (169方法) | Bridge 实现模板 | 20 min |
| Y-test/mock/OHBridge.java | Mock 实现模板 | 10 min |
| Y-skills/A2OH-LIFECYCLE.md | Activity Bridge | 20 min |
| Y-skills/A2OH-DATA-LAYER.md | Preferences/DB Bridge | 15 min |

### 重点参考 (遇到对应问题时深读)

| 文档 | 适用范围 |
|------|---------|
| [H35 Wine] CASES.md | API 行为测试方法论 |
| [H21 React Native] CASES.md | Bridge 性能优化 |
| [H34 BlackBerry 10] CASES.md | 手机上的 Android 兼容层 (唯一先例) |
| [H63 AWS Q Transform] CASES.md | 工业级 AI 迁移工作流 |
| [H26 Anbox] + [H27 Waydroid] CASES.md | Android on Linux 架构 |
| Y-art/Makefile.ohos-arm64 | ART 交叉编译 |
| Y-stub (aosp-stub-gen.py) | 自动生成 AOSP 缺失 stub |
| Y-db (api_compat.db, 57289 API) | API 映射优先级排序 |
| HB-build 全文 | 系统级编译架构 (人力方案，参考不抄) |
| PC-DEV-STRATEGY.md | Mock 开发 3 阶段策略 |

### 按需查阅 (AI 模式库)

| 模式 | 何时查 |
|------|--------|
| [B11 Ralph Loop] AI-B:167-179 | 任何编译失败时 |
| [D05 LLM-as-Judge] AI-D:61-73 | 代码审查环节 |
| [D07 Confidence] AI-D:109-120 | Gate 判定环节 |
| [E08 Screenshot] AI-E:111-122 | 渲染验证环节 |
| [C01 Supervisor] AI-C:10-22 | 多 Agent 协调 |
| [C05 Specialist Team] AI-C | Swarm 分工 |
| [C08 CI/CD Self-Healing] AI-C | GitHub 自动化 |
| [B06 Test Generation] AI-B | 测试用例生成 |
| [F03 Code RAG] AI-F | 查找 OH/AOSP 实现 |

---

## Yue 直接可复用清单 (复制级别)

| 组件 | 源路径 | 复用方式 | 优先级 |
|------|--------|---------|--------|
| OHBridge JNI (169方法) | Y-bridge/ | 扩展，加系统级方法 | P0 |
| ART Runtime (ARM64) | Y-art/ | 直接使用预编译产物 | P0 |
| AOSP Framework (2056文件) | Y-shim/java/android/ | 整体复制不修改 | P0 |
| HeadlessTest (2476测试) | Y-test/02-headless-cli/ | 适配+扩展 | P1 |
| Mock OHBridge | Y-test/mock/ | 直接复用 | P1 |
| aosp-stub-gen.py | Y-stub | 直接使用 | P1 |
| MockDonalds 源码 | Y-test/04-mockdonalds/ | 作为 Demo 3 基础 | P1 |
| API DB (57289 API) | Y-db | 查询参考 | P2 |
| A2OH Skills (11文档) | Y-skills/ | 映射指南 | P2 |
| Orchestrator 架构 | Y-orch | 参考多worker模式 | P3 |
