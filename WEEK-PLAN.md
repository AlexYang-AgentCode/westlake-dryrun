# 西湖项目 — 一周落地计划 V2（2026-03-31 ~ 04-06）

> **核心策略**: Yue 先行验证 → HanBing 上真机
> - Yue 的 Mock 环境挡住 Java 层 bug → PC 上 30 秒出结果
> - 只有 Yue 验证不了的（Binder IPC / .z.so / appspawn-x）才上真机
> - 目标: 改 10 次代码，只上 3 次真机

---

## 三个交付物

1. **系统级 Hello World** — Android APK 在 OH 真机上跑起来
2. **软件工厂演示** — AI 自动生成 Bridge → 测试 → 修复循环
3. **少烧ROM 技术 ABC** — 实际工作数据（hot-push 秒数、Mock 通过率）

---

## 两条线并行

```
时间线:
        Day1          Day2          Day3      Day4       Day5       Day6      Day7
        ┃             ┃             ┃         ┃          ┃          ┃         ┃
Line A  ██ Yue PC验证  ██ HanBing    ██ 排障   ██         ██         ██        ██
(真机)  设备恢复       编译+部署     +跑通     技术ABC    (空闲)     全流程    汇报
        ┃             ┃             ┃         实战       ┃          演示      ┃
Line B  ██ Yue Mock   ██ 适配Mock   ██ Bridge  ██        ██ 软件    ██        ██
(PC)    跑通HelloWorld 给HanBing用  代码生成   自愈循环   工厂骨架   串联      缓冲
```

---

## 步骤总览：40步，7天

| Phase | Steps | 依赖 | 能立即开始？ |
|-------|-------|------|-------------|
| **Phase 0: PC 验证 (Yue)** | 1-8 | 只需 PC + JDK | **立即** |
| **Phase 1: 真机准备** | 9-14 | 需要设备+服务器 | **立即（与Phase 0并行）** |
| **Phase 2: 真机上机** | 15-22 | 依赖 Phase 1 | 等 Phase 1 |
| **Phase 3: 技术ABC实战** | 23-28 | 依赖 Phase 2 | 等 Phase 2 |
| **Phase 4: 软件工厂** | 29-36 | Phase 0 即可开始 | **部分立即** |
| **Phase 5: 汇报** | 37-40 | 全部完成 | 等前面 |

---

## Phase 0: PC 验证 — Yue 环境跑通 Hello World [立即开始]

> **目的**: 在 PC 上证明 Android 框架逻辑正确，为 HanBing 真机验证提供基线。
> **需要**: 只需 PC + JDK，不需要任何设备。

### Step 1: 确认 JDK 可用 ★立即
- [ ] `javac -version` / `java -version`
- [ ] 如果没有: `sudo apt install default-jdk`

### Step 2: 在 PC 上跑 Yue HelloWorld ★立即
- [ ] `cd 16.13-Yue/westlake/`
- [ ] 编译: `javac -sourcepath test-apps/mock:shim/java:test-apps/hello-world/src -d /tmp/hw HelloWorldRunner.java`
- [ ] 运行: `java -cp /tmp/hw HelloWorldRunner`
- [ ] 预期: 看到生命周期日志 + 测试通过
- **指导文件**: `16.13-Yue/westlake/test-apps/hello-world/src/HelloWorldRunner.java`

### Step 3: 跑 Calculator + MockDonalds ★立即
- [ ] 同样方式编译运行 CalculatorRunner 和 MockDonaldsRunner
- [ ] 记录: 多少测试通过/失败
- [ ] 这 3 个 demo 是后续真机验证的基线
- **指导文件**: `16.13-Yue/westlake/test-apps/07-calculator/`, `test-apps/04-mockdonalds/`

### Step 4: 提取 DrawRecord 基线 ★立即
- [ ] HelloWorld 的 Mock OHBridge 会记录所有 canvas 操作到 DrawRecord
- [ ] 导出为 JSON: `{op: "drawText", args: [x,y,fontSize], text: "Hello World!", color: 0xFF000000}`
- [ ] 这就是「行为录制」的第一个 ground truth — 不需要 Android 真机录制
- **指导文件**: `16.13-Yue/westlake/test-apps/mock/com/ohos/shim/bridge/OHBridge.java` (getDrawLog方法)

### Step 5: 验证 HanBing Java 适配器的纯 Java 逻辑 ★立即
- [ ] `LifecycleAdapter.java` — 6态↔4态映射表，纯 Java，PC 上写单元测试
- [ ] `IntentWantConverter.java` — Intent→Want 转换，纯 Java，PC 上测
- [ ] `LifecycleAdapter` 输入 OH_STATE_INACTIVE → 输出 ANDROID_STATE_CREATED？
- [ ] `IntentWantConverter` 输入 Intent(action="android.intent.action.MAIN") → 输出什么 Want？
- **指导文件**: `16.12-HanBing/AdapterCode/framework/activity/java/LifecycleAdapter.java`, `IntentWantConverter.java`

### Step 6: 建立 Mock 切换机制 ★立即
- [ ] 写一个 `MockBridgeAdapter.java` 包装 Yue 的 OHBridge
- [ ] 让 HanBing 的 `ActivityManagerAdapter` 在 mock 模式下调用它而不是 JNI
- [ ] 环境变量: `BRIDGE_MODE=mock` → PC 测试; `BRIDGE_MODE=real` → 真机
- [ ] 这样 HanBing 530 个方法中的纯 Java 部分都能 PC 上测
- **指导文件**: `16.87-TestLAB/ACTION-PLAN-SelfEvolve.md` (M4节)

### Step 7: 跑 TestLAB demo-method-B (Host Mock) ★立即
- [ ] `bash 16.87-TestLAB/demo-method-B.sh`
- [ ] 验证 PC Mock 测试流水线可用
- **指导文件**: `16.87-TestLAB/demo-method-B.sh`

### Step 8: Phase 0 检查点
- [ ] 3 个 demo 在 PC 上全部通过
- [ ] HanBing 的 LifecycleAdapter + IntentWantConverter PC 上测通
- [ ] DrawRecord 基线数据已保存
- [ ] Mock 切换机制可用
- **产出**: 一份「PC 验证报告」— 证明 Java 层逻辑正确

---

## Phase 1: 真机准备 [立即开始，与 Phase 0 并行]

> **目的**: 设备就绪 + 编译产物完整。与 Phase 0 同时进行。

### Step 9: 设备恢复 ★立即
- [ ] 检查 DAYU600: 能否开机、hdc 能否连接
- [ ] 如果不稳定: 按 RECOVERY-TODO.md 恢复
- [ ] 最坏: HiTool 重刷 (唯一一次)
- **指导文件**: `16.87-TestLAB/L4-device-push/RECOVERY-TODO.md`

### Step 10: hdc 通路验证 ★立即（设备开了就做）
- [ ] `hdc list targets` → 看到设备 ID
- [ ] `hdc shell param get const.ohos.apiversion` → API 版本
- [ ] `hdc shell mount | grep system` → /system 挂载方式
- **指导文件**: `16.12-HanBing/AdapterCode/build/deploy.sh` (check_device)

### Step 11: 编译服务器连通性 ★立即
- [ ] SSH GZ05 — OH 编译用
- [ ] SSH GZ02 — AOSP 编译用
- [ ] 确认源码树完整

### Step 12: 盘点编译产物
- [ ] `ls -la 16.12-HanBing/AdapterCode/out/` → 哪些 .so/.jar 已有
- [ ] `readelf -d` 检查动态依赖
- [ ] 列出缺失产物，触发编译
- **指导文件**: `16.87-TestLAB/L2-symbols/test_symbols.sh`

### Step 13: 触发缺失产物编译
- [ ] 如果 .z.so 缺: GZ05 上 `build/oh_build.sh`
- [ ] 如果 native .so 缺: `build/cross_compile_100pct.sh`
- [ ] 如果 framework.jar 缺: GZ02 上编译
- [ ] 编译是后台跑的，不阻塞其他工作

### Step 14: Phase 1 检查点
- [ ] 设备在线 + hdc 可用
- [ ] 编译产物全部就绪（或编译任务在跑）
- [ ] 至少一台编译服务器可用

---

## Phase 2: 真机上机 — Hello World [依赖 Phase 0 + Phase 1]

> **目的**: 把 PC 上验证通过的代码推到真机。
> **关键**: Phase 0 已经过滤掉 Java 层 bug，这里只验证 Binder/Surface/进程模型。

### Step 15: 备份 + 部署 OH 系统补丁
- [ ] 备份原始 .z.so: `hdc shell cp /system/lib64/platformsdk/libabilityms.z.so /data/backup/`
- [ ] `deploy.sh --target=oh-services` 推送 4 个 .z.so
- [ ] `deploy.sh --target=android-rt` 推送 framework.jar + 12 个 native .so
- [ ] `deploy.sh --target=adapter` 推送 appspawn-x + bridge
- [ ] **reboot 一次** (push 了系统服务，需要重启加载)
- **指导文件**: `16.12-HanBing/AdapterCode/build/deploy.sh`

### Step 16: 验证 appspawn-x 启动
- [ ] `hdc shell ps -ef | grep appspawn` → 进程存在
- [ ] `hdc shell hilog | grep -i adapter` → 看启动日志
- [ ] 如果不在 → 检查 appspawn_x.cfg 和权限
- **这是 Yue 帮不了的第一个验证点 [6]**

### Step 17: 安装 + 启动 HelloWorld APK
- [ ] `hdc file send HelloWorld.apk /data/local/tmp/`
- [ ] `hdc shell bm install -p /data/local/tmp/HelloWorld.apk`
- [ ] `hdc shell aa start -a MainActivity -b com.example.helloworld`
- [ ] 看 hilog: 有没有 onCreate / Binder 调用日志
- **这是 Yue 帮不了的第二个验证点 [7][8]**
- **指导文件**: `16.87-TestLAB/L6-e2e/test_hello_apk.sh`

### Step 18: 排障 — 分层定位
- [ ] **如果 crash 在 Java 层** → 回 PC 用 Yue Mock 复现修复 (不上真机)
- [ ] **如果 crash 在 JNI 层** → 看 native backtrace → hot-push 修复的 .so
- [ ] **如果 crash 在 OH 系统服务** → hilog 看 abilityms 日志 → 重编 .z.so → push + kill
- [ ] 每轮修复: 先问「这个 bug Yue 能验证吗？」能 → PC 修; 不能 → 真机修
- **关键原则**: Java 层 bug 回 PC, 系统层 bug 才上真机

### Step 19: 验证渲染通路
- [ ] 屏幕有输出？(哪怕白屏也算进展)
- [ ] `hdc shell snapshot_display -f /data/local/tmp/hello.png` 截图
- [ ] 和 Step 4 的 DrawRecord 基线对比: 文字位置、颜色是否一致
- **这是 Yue 帮不了的第三个验证点 [9]**

### Step 20: SecondActivity 跳转
- [ ] 点击按钮 → startActivity(SecondActivity)
- [ ] hilog 看 Intent→Want 转换日志
- [ ] 对比 Step 5 的 PC 测试结果: Want 参数是否一致
- **Intent→Want 的逻辑已在 PC 验证过，这里只验 Binder 通路**

### Step 21: 截图 + 视频
- [ ] 保存截图为 `results/hello_oh.png`
- [ ] 录屏/拍视频: 启动 → UI → 按钮 → Activity跳转
- [ ] 和 Step 4 DrawRecord 做像素级对比 (如有 ImageMagick)

### Step 22: Phase 2 检查点 — 里程碑 1
- [ ] **Hello World APK 在 OH 真机上可见运行**
- [ ] 截图已保存
- [ ] 记录: 从 Phase 1 到跑通，真机上改了几次？(vs PC 上改了几次？)
- [ ] 已知问题列表

---

## Phase 3: 技术 ABC 实战 [依赖 Phase 2]

> **目的**: 用 Hello World 作为载体，演示三种少烧ROM技术的真实效果。

### Step 23: 技术A — hot-push 计时
- [ ] 修改一处日志字符串 → 重编 .so → `hdc file send` → `kill` 进程
- [ ] **计时**: 从保存代码到 hilog 看到新日志 = ?秒
- [ ] 目标: < 30秒
- [ ] 连续做 3 次，取平均值
- **指导文件**: `16.87-TestLAB/demo-method-A.sh`, `ACTION-PLAN-SelfEvolve.md` (M1)

### Step 24: 技术A — 系统服务 hot-push
- [ ] 修改 libabilityms.z.so 中的一处日志
- [ ] push → `hdc shell kill $(pidof ability_manager_service)`
- [ ] 确认服务自动重启 + 新日志出现
- [ ] **计时**: ?秒 (不需要 reboot)

### Step 25: 技术B — Host Mock 演示
- [ ] 在 HanBing 的 LifecycleAdapter 中故意写一个 bug (映射错误)
- [ ] PC 上用 Yue Mock 跑测试 → 立即发现
- [ ] 修复 → PC 重跑 → 通过
- [ ] **从未上真机就修好了**
- [ ] 对比: 如果没有 Mock，这个 bug 需要 push 真机 → 看 hilog → 猜原因
- **指导文件**: `16.87-TestLAB/demo-method-B.sh`

### Step 26: 技术C — Mock hdc 演示
- [ ] `bash 16.87-TestLAB/demo-method-C.sh`
- [ ] 演示: 用预录快照开发部署脚本，不需要真机连着
- **指导文件**: `16.87-TestLAB/demo-method-C.sh`

### Step 27: 截图自动对比
- [ ] OH 截图 vs Step 4 DrawRecord 基线 → RMSE
- [ ] `compare -metric RMSE hello_android.png hello_oh.png diff.png`
- [ ] 记录 RMSE 值
- **指导文件**: `ACTION-PLAN-SelfEvolve.md` (M6)

### Step 28: Phase 3 检查点 — 里程碑 2
- [ ] hot-push 有 3 个计时数据点
- [ ] Host Mock 有一个「PC 上发现 bug → 修复 → 从未上真机」的案例
- [ ] Mock hdc 有运行日志
- [ ] RMSE 有一个真实数据点

---

## Phase 4: 软件工厂 [Phase 0 完成即可开始部分步骤]

> **目的**: 展示 AI 驱动的 Bridge 生成 → 测试 → 自愈循环。

### Step 29: Bridge 代码生成模板 ★Phase 0 后即可开始
- [ ] 从 Yue `ohbridge_native.c` 提取 JNI registration 模式
- [ ] 写 `codegen-bridge.sh`: 输入 api-spec.yaml → 输出 bridge.c 骨架
- [ ] 用 01-Logging (4 API) 验证生成器
- **指导文件**: `16.11-Main/WestlakeV3/01-Logging/api-spec.yaml`, `ohbridge_native.c`

### Step 30: 置信度门控脚本
- [ ] 写 `scripts/confidence-gate.sh`
- [ ] 输入: 编译结果 + 符号检查 + Mock测试结果 + 截图RMSE
- [ ] 输出: 0-1 分数; >0.85 自动通过, <0.5 拒绝
- **指导文件**: `ACTION-PLAN-SelfEvolve.md` (M7)

### Step 31: Ralph Loop 自愈 (最小版)
- [ ] 写 `scripts/ralph-loop.sh`
- [ ] 流程: 编译 → Mock测试 → 失败 → 读错误 → 建议修复 → 重编 → 重测
- [ ] 最多 5 轮
- [ ] 用 L3 host-mock 验证，不需要真机
- **指导文件**: `ACTION-PLAN-SelfEvolve.md` (M8)

### Step 32: 用 Logging Bridge 走完整循环
- [ ] 自动生成 → Yue Mock 测试 → 编译 → 符号检查 → 置信度评分
- [ ] 这是「软件工厂」的最小完整演示
- [ ] 录制终端全过程

### Step 33: 故意注入 bug → 自愈演示
- [ ] 在生成的 Bridge 中注入 bug (函数签名错误)
- [ ] Ralph Loop 自动检测 → 修复 → 重编 → 通过
- [ ] 录制全过程 (asciinema 或视频)

### Step 34: 兼容性数据库
- [ ] 创建 `results/compatibility-db.json`
- [ ] 格式: `{boundary, api, status, rmse, confidence, timestamp}`
- [ ] 录入 Logging + Activity 的数据
- **指导文件**: `ACTION-PLAN-SelfEvolve.md` (M10)

### Step 35: 端到端演示脚本
- [ ] 写 `demo-software-factory.sh` 串联:
  1. api-spec.yaml → 生成 Bridge
  2. Yue Mock 测试
  3. 编译 .so
  4. 符号检查
  5. hot-push (如果有设备)
  6. 截图对比
  7. 置信度评分
  8. PASS/FAIL
- [ ] 用 Logging Bridge 走完

### Step 36: Phase 4 检查点 — 里程碑 3
- [ ] 代码生成器可用
- [ ] 自愈循环有真实案例
- [ ] 端到端脚本可运行
- [ ] 兼容性数据库有数据

---

## Phase 5: 汇报 + 缓冲 [Day 7]

### Step 37: 一页纸总结
- [ ] 3 个交付物 × 关键数据
- [ ] 截图: Hello World 在 OH / 像素对比 / 自愈过程

### Step 38: 数据表
- [ ] 烧ROM次数: 0 (目标)
- [ ] reboot次数: 1 (首次部署)
- [ ] hot-push 平均耗时: ?秒
- [ ] PC Mock 拦截 bug 比例: ?%
- [ ] 自动通过率: ?%

### Step 39: 更新 STATUS.md
- [ ] `D:/ObsidianVault/10-Projects/16-WestLake/16.11-Main/STATUS.md`

### Step 40: 下周规划
- [ ] 179 API 全覆盖调度
- [ ] Calculator / MockDonalds demo 时间线

---

## ★ 立即能开始的事（按优先级排序）

### 现在就做 — PC 上，不需要任何设备或服务器

| 优先级 | Step | 做什么 | 耗时 | 产出 |
|--------|------|--------|------|------|
| **1** | Step 1 | `javac -version` 确认 JDK | 10秒 | 环境就绪 |
| **2** | Step 2 | Yue HelloWorld 在 PC 跑 | 5分钟 | 基线通过 |
| **3** | Step 3 | Calculator + MockDonalds 跑 | 10分钟 | 3个demo全绿 |
| **4** | Step 4 | 提取 DrawRecord 基线 | 15分钟 | ground truth JSON |
| **5** | Step 5 | HanBing LifecycleAdapter 单测 | 30分钟 | Java逻辑验证 |
| **6** | Step 7 | `bash demo-method-B.sh` | 2分钟 | Mock流水线验证 |

### 同时做 — 需要设备/服务器，可与上面并行

| 优先级 | Step | 做什么 | 耗时 | 产出 |
|--------|------|--------|------|------|
| **1** | Step 9 | DAYU600 开机 + hdc 检查 | 5分钟 | 设备状态 |
| **2** | Step 11 | SSH GZ05/GZ02 | 5分钟 | 编译服务器就绪 |
| **3** | Step 12 | 盘点 out/ 编译产物 | 10分钟 | 缺失清单 |
| **4** | Step 13 | 触发缺失编译 (后台) | 0分钟(后台) | 编译在跑 |

### 最快出效果的路径

```
现在                    +30分钟               +2小时              +1天
  │                       │                    │                   │
  ▼                       ▼                    ▼                   ▼
  Step 1-2               Step 3-4             Step 5-6            Step 15-17
  JDK + Yue HelloWorld   3个Demo全绿          HanBing Java        真机部署
  PC上跑通               DrawRecord基线        适配器PC验证        Hello World
  │                       │                    │                   │
  └── 证明: APK 逻辑对   └── 有 ground truth   └── Java层无bug     └── 系统级跑通
```

---

## Yue 对 HanBing 的价值速查表

| Hello World 验证点 | PC 验 (Yue) | 真机验 | 备注 |
|---|:---:|:---:|---|
| [1] Activity 生命周期 6↔4 | ✅ | — | LifecycleAdapter 纯 Java |
| [2] Intent → Want 转换 | ✅ | — | IntentWantConverter 纯 Java |
| [3] View 树 measure/layout | ✅ | — | AOSP 代码，Yue 已验证 |
| [4] Canvas 绘制参数 | ✅ | — | DrawRecord 日志对比 |
| [5] APK 逻辑正确性 | ✅ | — | 3个demo 已跑通 |
| [6] appspawn-x 进程模型 | — | ✅ | 必须真机 |
| [7] Binder IPC 通路 | — | ✅ | 必须真机 |
| [8] OH .z.so 补丁 | — | ✅ | 必须真机 |
| [9] RSSurfaceNode 渲染 | — | ✅ | 必须真机 |
| [10] 安全沙箱 | — | ✅ | 必须真机 |

**规则**: 改了 [1-5] 相关代码 → PC 修。改了 [6-10] 相关代码 → 真机修。

---

## 排障决策树

```
代码改了 → 编译通过？
  │                │
  No → 读错误     Yes → 涉及 Binder/进程/Surface？
       PC修              │              │
                         No             Yes
                         │              │
                    Yue Mock测试    hot-push 真机
                         │              │
                    通过？          hilog 看日志
                    │    │              │
                   Yes   No         通过？
                    │    │          │    │
              hot-push  PC修       Yes   No
              真机验证  (不上机)    Done  分析crash
                                         │
                                    Java层？ → PC修
                                    JNI层？  → 改.so→hot-push
                                    系统层？ → 改.z.so→kill服务
```

---

## 风险与降级

| 风险 | 降级方案 |
|------|---------|
| DAYU600 变砖 | HiTool 重刷 (30分钟，唯一一次烧ROM) |
| HanBing 产物不完整 | Yue 的 ohbridge_native.c 交叉编译替代 |
| appspawn-x 起不来 | 降级用 Yue 的 dalvikvm 手动启动 |
| 编译服务器不可用 | 只推已有产物，编译问题后面解决 |
| OH .z.so 版本不兼容 | 先不推 .z.so，只推 Android Runtime + Bridge |

---

## 文件地图

### Phase 0 用到的文件 (PC验证)
| 文件 | 用途 |
|------|------|
| `16.13-Yue/westlake/test-apps/hello-world/src/HelloWorldRunner.java` | PC 运行入口 |
| `16.13-Yue/westlake/test-apps/mock/com/ohos/shim/bridge/OHBridge.java` | Mock 实现 |
| `16.13-Yue/westlake/shim/java/android/` | Android 框架 shim |
| `16.12-HanBing/AdapterCode/framework/activity/java/LifecycleAdapter.java` | 生命周期映射 |
| `16.12-HanBing/AdapterCode/framework/activity/java/IntentWantConverter.java` | Intent→Want |
| `16.87-TestLAB/demo-method-B.sh` | Host Mock 演示 |

### Phase 2 用到的文件 (真机)
| 文件 | 用途 |
|------|------|
| `16.12-HanBing/AdapterCode/build/deploy.sh` | 部署脚本 (400行) |
| `16.12-HanBing/AdapterCode/out/` | 编译产物 |
| `16.87-TestLAB/L6-e2e/test_hello_apk.sh` | E2E 测试 |
| `16.87-TestLAB/L4-device-push/RECOVERY-TODO.md` | 设备恢复 |

### Phase 4 用到的文件 (软件工厂)
| 文件 | 用途 |
|------|------|
| `16.11-Main/WestlakeV3/01-Logging/api-spec.yaml` | 代码生成输入 |
| `16.13-Yue/westlake/westlake-host/jni/ohbridge_native.c` | JNI 模板 |
| `16.87-TestLAB/ACTION-PLAN-SelfEvolve.md` | M1-M10 措施代码 |
| `16.87-TestLAB/Waydroid-DeepDive.md` | 架构参照 |
