# WestlakeV3 并行调度 + 编译抵消策略

## 核心矛盾

```
编译一次 = 1 小时
7天 × 18小时 = 126 小时
如果串行: 126 ÷ 1 = 最多 126 次编译
实际需要: ~50 次编译 (42个交付件 + 补编)
串行编译占用: 50 小时 = 40% 时间被编译吃掉!
```

**解法: 5 台服务器流水线 + hdc push 免 ROM**

---

## 5 台服务器 + PC + 2 开发板 = 8 个执行单元

| 单元 | 角色 | 能力 | 同时任务数 |
|------|------|------|-----------|
| **GZ02** | OH 主编译 | 16核/60G, Docker, OH+AOSP源码 | 2 (Docker内+宿主机) |
| **GZ05** | ART编译 + JVM测试 | 16核/60G, JDK11 | 2 (编译+测试并行) |
| **GZ01** | 轻量验证 + 监控 | 2核/4G, NFS客户端 | 1 |
| **GZ03** | 轻量验证 + NFS | 2核/4G, NFS客户端 | 1 |
| **HK01** | GitHub + 国际下载 | 2核/8G, 香港 | 1 |
| **本地PC** | AI 推理 + 设备 | Claude Code, hdc/adb | 3 (多Agent) |
| **D200** | 鸿蒙测试 | OH V7, hdc | 1 |
| **D600** | Android 基线 | AOSP V15, adb | 1 |

**理论最大并行度: 11 路**

---

## 编译抵消三板斧

### 板斧 1: 流水线 — 编译时开发下一个

```
时间线  ──────────────────────────────────────────────→
GZ02    [编译Logging.so 1h] [编译Activity.so 1h] [编译Window.so 1h]
本地PC   [开发Activity代码]  [开发Window代码]     [开发Rendering代码]
GZ05            [JVM测试Logging]  [JVM测试Activity]  [JVM测试Window]
```

**编译 A 的 1 小时内，本地 PC 已经写完 B 的代码**，编译 A 完成后立刻开始编译 B。
等效编译等待时间: **接近 0**。

### 板斧 2: hdc push 免 ROM — 增量测试

```
传统: 改 1 个 .so → 重编 ROM (1h) → 刷ROM (10min) → 测试
V3:   改 1 个 .so → 编译单个模块 (5min) → hdc push (10sec) → 测试

加速比: 70分钟 → 5分钟 = 14倍
```

**哪些可以 hdc push**:
| 文件 | 推送位置 | 需要重启? |
|------|---------|----------|
| liboh_adapter_bridge.so | /system/lib64/ | 否(kill进程即可) |
| framework.jar | /system/android/framework/ | 否(kill进程) |
| libbionic_compat.so | /system/android/lib64/ | 否 |
| Layer 2 任意 .so | /system/android/lib64/ | 否 |
| appspawn-x | /system/bin/ | 是(需重启init) |
| OH 补丁 .z.so | /system/lib64/platformsdk/ | **是(需重启)** |

**只有 OH 补丁和 appspawn-x 需要重启**。Bridge .so 改了直接 push + kill 进程即可。

### 板斧 3: 预测性编译 — 同时编两个版本

```
GZ02 Docker:  [编译版本A: 保守方案]
GZ05:         [编译版本B: 激进方案]   ← 同时进行
              哪个先通过测试用哪个
```

---

## 7 天 × 8 单元 并行调度表

### D1 (18h): Phase 0 — 三线并行 + 预编译

```
时间    GZ02(Docker)         GZ05              本地PC           GZ01        HK01        D200    D600
──────────────────────────────────────────────────────────────────────────────────────────────
0-2h    验证已有产物         ART .so验证        连接D200+D600     NFS挂载     GitHub仓库   快照采集  快照采集
        readelf/nm全部       dex2oat验证        API行为录制                   ci.yml
                                                                            auto-fix.yml
2-4h    string_bridge编译    已有2476测试        mock-hdc编写     同步快照     release.yml  继续录制  继续录制
        config.sh/build.sh   基线运行           mock-adb编写     到NFS       Issue模板
                                                verify.py
4-8h    [预编译OH补丁 1h]    [预编译ART 1h]     ralph-loop.sh    API行为     下载依赖     收起     收起
        ability_runtime      如缺失则编译       confidence_gate   回放引擎   (npm/pip)
        → libabilityms.z.so                     deploy.sh
                                                API行为对比
8-12h   [预编译其余补丁]     [编译dalvikvm]     Plugin骨架15个    监控编译    —           —       —
        appms/BMS/scene      boot image         Agent协作规则     状态
                             framework验证      3 Demo验收标准
12-18h  [预编译appspawn-x]   [预编Hello/Calc/   Phase 0 Gate     测试NFS    —           —       —
        全栈链接验证          McD .dex]          Demo 3 Bug脚本    读写速度
```

**D1 结束时已完成**:
- ✅ 快照 + Mock 就绪
- ✅ 工具链就绪 (verify/ralph/confidence/deploy)
- ✅ GitHub 就绪 (ci/auto-fix/release)
- ✅ 预编译: OH 补丁 4 个 + appspawn-x + ART + 3 个 .dex (利用编译空闲时间!)
- ✅ 设备已收起

### D2 (18h): Demo 1 前半 — Logging + Activity (Bridge∥编译)

```
时间    GZ02(Docker)              GZ05                本地PC(AI)           GZ01         GZ03
──────────────────────────────────────────────────────────────────────────────────────────
0-3h    [编译Logging Bridge 5min] JVM: Logging单元    Logging Mock+Bridge  监控          —
        → 等Activity Bridge完成   测试(16用例)        (string_bridge RAII)
                                                      Activity Mock:
                                                      状态机+Intent→Want
3-6h    [编译Activity Bridge 1h]  JVM: Activity       Activity Bridge:     nm验证       API行为
        oh_ability_manager       Mock 50用例测试       startAbility 实现    Logging      对比
                                                      AOSP补丁编写         符号
6-10h   编译中...继续...          JVM: terminateSelf   terminateSelf        LLM-Judge    nm验证
        [同时编AOSP补丁.jar]      checkPermission      checkPermission      Logging      Activity
                                  测试                  Bridge编写
10-14h  [编译完成!]               Activity联合测试     Window Mock开始      LLM-Judge    行为对比
        收集产物→NFS              多Activity栈测试     surfaceCreate等      Activity     Activity
                                  2476回归(子集)
14-18h  [编译Window Bridge 1h]    [继续JVM测试]        Window Bridge 完成   AI回顾       —
        surfaceCreate等                                Rendering Mock 开始  lessons
```

**关键: GZ02 编译的 1 小时内, 本地 PC 已经写完下一个 Bridge 代码, GZ05 在跑测试**

### D3 (18h): Demo 1 后半 — Rendering Swarm + 全栈集成

```
时间    GZ02(Docker)              GZ05                本地PC(3路AI)         GZ01        GZ03
──────────────────────────────────────────────────────────────────────────────────────────
0-6h    [编译Rendering全部 1h]    JVM: Window测试     Swarm 3路并行:       nm验证      SSIM
        47个draw函数链入.so       PNG渲染→SSIM        路1: Canvas 7 API    Window      基线
                                                      路2: Pen+Brush 5
                                                      路3: Font+Bitmap 5
6-10h   [编完→push到NFS]         dalvikvm全栈运行     组合绘制测试         符号全      行为
        [开始编Input Bridge]      HelloWorld!          HelloWorld PNG       量验证      对比
                                  生命周期日志
10-14h  [编译Input 30min]         2476完整回归         Confidence评估       LLM-Judge   —
        → 产物就绪                                     AI回顾Demo1
                                                       ★ Demo 1 PASS
14-18h  [编译Calculator扩展]      Calculator逻辑       Calculator Mock      —           —
        DrawCircle/RoundRect      表达式解析器         21按钮布局
        等5个新API                JVM测试
```

**注意: D3 后半已经开始 Demo 2 的工作 (流水线)**

### D4 (18h): Demo 2 — Calculator

```
时间    GZ02(Docker)              GZ05                本地PC(AI)           GZ01        GZ03
──────────────────────────────────────────────────────────────────────────────────────────
0-4h    [D3已编完Input]           Calculator全栈       21按钮点击测试       nm验证      行为对比
        收集产物                  JVM运行              每次截图→AI Vision
4-10h   [编译Preferences Mock     41用例测试矩阵      算术边界测试         LLM-Judge   SSIM
        应用层版本 30min]         回归测试             5÷0, 999999²        Input       Calculator
        ← D5 Demo3 预编译!
10-14h  [编译MockDonalds          Calculator SSIM      ★ Demo 2 PASS       —           —
        相关Bridge 30min]        对比D600
14-18h  自愈buffer时间            自愈buffer            AI自主修复任何       监控        —
        如需重编→立即              回归重测             发现的问题
```

**关键: D4 下午已经在预编译 D5 需要的东西**

### D5 (18h): Demo 3 前半 — MockDonalds + 自愈管线

```
时间    GZ02(Docker)              GZ05                本地PC(AI)           HK01           D200
──────────────────────────────────────────────────────────────────────────────────────────
0-4h    [D4已预编Prefs Mock]      MockDonalds          构建5页面APK        完善            —
        Preferences Bridge骨架    JVM运行              SharedPrefs Bug     auto-fix.yml
                                  验证购物车功能        编写Issue模板       Claude Action
4-8h    [编译系统层Prefs 1h]      Bug复现:             GitHub Actions      测试:Issue      —
        OH_Preferences_*          加购→关→重开          本地调试            →workflow
                                  →数据丢了!
8-12h   编译中...                 Preferences行为       准备演示脚本        —              连接
                                  对比测试                                                准备
12-15h  [编译完成,产物就绪]       —                    🎬 演示开始          —              运行McD
                                                       提Issue                            发现Bug
15-18h  —                         —                    🎬 AI自动修          —              验证
                                                       →hotfix发布                        hotfix
```

### D6 (18h): Demo 3 后半 — 系统层 + 回归

```
时间    GZ02(Docker)              GZ05                本地PC(AI)           GZ01        GZ03
──────────────────────────────────────────────────────────────────────────────────────────
0-6h    [系统层Prefs已编好]       全量回归:            系统层Bridge集成     SSIM        行为对比
        集成到主bridge.so         Demo1+Demo2+McD      LLM-Judge审查       全量        全量
        [全量编译 1h]             2476测试
6-12h   [全量编译中...]           回归继续...           Confidence评估      —           —
                                  SSIM对比              如有差异→修复
12-16h  [编译完成]                Gate通过              v0.4.0 Release      —           —
        全量产物→NFS                                    Issue自动关闭
16-18h  [预编译D7部署包]          —                    ★ Demo 3 PASS       —           —
                                                       领导材料
```

### D7 (18h): 真机验证 — 确认不是调试

```
时间    GZ02              GZ05              本地PC(AI)           D200            D600
──────────────────────────────────────────────────────────────────────────────────
0-2h    —                 —                 重连D200              连接            连接
                                            推送全部产物
2-3h    —                 —                 重启(ROM#1)           重启...         —
                                            健康检查
3-6h    —                 —                 安装3个Demo           HelloWorld      基线采集
                                            AI截图验证            Calculator
                                                                 MockDonalds
6-10h   [如需修→编译]    [如需→测试]       如有问题→Ralph Loop   验证中          SSIM基线
10-14h  [如需→ROM#2]     —                 D200 vs D600 SSIM     最终截图        最终截图
14-18h  —                —                 最终报告               —              —
                                            ★ 3 Demo 真机 PASS
                                            部署脚本+领导材料
```

---

## 编译时间抵消统计

| 编译任务 | 时长 | 在哪台 | 并行做什么(其他机器) |
|---------|------|--------|---------------------|
| OH 补丁 libabilityms.z.so | 1h | GZ02 | 本地:写Activity Bridge / GZ05:JVM测试 |
| OH 补丁 libappms.z.so | 1h | GZ02 | 本地:写Window Bridge / GZ05:测试Activity |
| OH 补丁 BMS+Scene | 1h | GZ02 | 本地:写Rendering / GZ05:测试Window |
| ART 交叉编译 (如需) | 1h | GZ05 | GZ02:编OH补丁 / 本地:写Bridge |
| Logging Bridge .so | 5min | GZ02 | — (太快不需要抵消) |
| Activity Bridge .so | 1h | GZ02 | 本地:写Window / GZ05:测Activity |
| Window Bridge .so | 1h | GZ02 | 本地:写Rendering / GZ05:测Window |
| Rendering .so (47 API) | 1h | GZ02 | 本地:测试+SSIM / GZ05:dalvikvm全栈 |
| Input Bridge .so | 30min | GZ02 | 本地:Calculator逻辑 |
| Preferences .so | 1h | GZ02 | 本地:MockDonalds / HK01:GitHub |
| 全量编译 (最终) | 1h | GZ02 | GZ05:回归 / 本地:报告 |
| D7 修复编译 (如需) | 1h | GZ02 | 本地:其他Demo测试 |
| **总编译: ~11h** | | | **等效等待: ~2h** (只有前2次无法完全抵消) |

**编译利用率: 11h编译 / 2h等待 = 82% 的编译时间被并行工作抵消**

---

## "意图驱动 → 多Agent协作 → 自我进化" 在服务器上的体现

### 意图驱动

```
人(本地PC): "现在做 Activity Bridge"
  ↓ 意图分发
GZ02: 收到 → 准备编译环境 → 等待代码
GZ05: 收到 → 准备JVM测试环境 → 加载Mock
GZ01: 收到 → 准备nm/readelf验证脚本
```

人只说"做什么"，不说"怎么编译、在哪台机器、什么参数"。

### 多Agent协作

```
Agent-Bridge(本地): 写 oh_ability_manager.cpp
  → git push
Agent-Compile(GZ02): 检测到新push → 自动拉取 → Docker编译 → 产物→NFS
Agent-Test(GZ05): 检测到NFS新产物 → 拉取 → JVM测试 → 结果→NFS
Agent-Verify(GZ01): 检测到产物 → readelf/nm → 报告→NFS
Agent-Judge(GZ03): 检测到代码变更 → LLM审查 → 结果→NFS
Agent-CI(HK01): 检测到push → GitHub Actions → CI结果
```

**5台服务器 = 5个专职Agent**，通过 NFS/Git 自动协调，无需人工调度。

### 自我进化

```
Ralph Loop 在 GZ02 发现编译错误:
  → 分析错误 → 自动修改代码 → 重新编译 → 测试
  → 如果 GZ02 繁忙 → 自动切到 GZ05 编译
  → 记录 lessons → 下次同类错误直接跳过

Confidence Gate 发现低置信:
  → 自动增加测试用例 → 重跑 → 提升置信度
  → 如果仍低 → 自动降级到应用层Mock(不走系统层)
```

---

## hdc push 免 ROM 迭代循环

```
正常开发循环 (5分钟/轮):
  本地PC: 改Bridge代码 → git push
  GZ02: 自动编译 .so (5min增量)
  GZ02→NFS→本地PC: 产物同步
  本地PC: hdc file send bridge.so /system/lib64/
  本地PC: hdc shell "kill $(pidof appspawn-x)"  ← 不重启!进程自动重拉起
  本地PC: hdc shell "aa start ..." → 测试 → 截图 → AI验证

需要ROM的场景 (仅2次):
  1. 首次推送 OH 补丁 .z.so (替换系统服务,必须重启)
  2. 首次推送 appspawn-x.cfg (init配置,必须重启)
  之后: 改Bridge.so → hdc push → kill → 自动重拉 → 测试 (免ROM!)
```

---

## 关键路径分析

```
关键路径 (决定总工期的最长链):

D1: OH补丁预编译(4h) → 不在关键路径(与Phase0并行)
D2: Activity Bridge编写(3h) → 编译(1h) → 测试(2h) = 6h
D3: Rendering Swarm(3h) → 编译(1h) → 全栈集成(3h) = 7h ← 最长!
D4: Calculator(3h) → 编译(0.5h) → 测试(3h) = 6.5h
D5: MockDonalds(4h) → GitHub(3h) → 演示(3h) = 10h
D6: 系统层(3h) → 编译(1h) → 回归(3h) → Release(2h) = 9h
D7: 推送(1h) → 重启(0.5h) → 3Demo验证(6h) = 7.5h

总关键路径 = D1(4h预编) + D2(6h) + D3(7h) + D4(6.5h) + D5(10h) + D6(9h) + D7(7.5h)
           = 50h (远低于 126h 可用时间)
余量 = 126 - 50 = 76h (60% 余量，用于自愈/重试/意外)
```
