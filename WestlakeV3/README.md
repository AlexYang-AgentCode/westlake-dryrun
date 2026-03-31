# WestlakeV3 — 179 API 全覆盖 Agent 飞速迭代版

> **版本**: V3.0 (2026-03-29)
> **焦点约束**: A最小化烧ROM / B最小化物理设备 / D Agent可确认验证
> **设备**: D200 (OH V7.0) + D600 (AOSP V15)

## 15 个边界 → 179 个 JNI 方法

| 目录 | 边界 | API数 | 优先级 | 复杂度 |
|------|------|-------|--------|--------|
| `01-Logging/` | 日志 | 4 | P1 | 极低 |
| `02-Activity/` | Activity/Ability管理 | 3 | P0 | 高 |
| `03-Window/` | 窗口/Surface | 6 | P0 | 高 |
| `04-Rendering/` | 渲染/Canvas | 47 | P0 | 高 |
| `05-Input/` | 输入事件 | 3 | P1 | 中 |
| `06-Preferences/` | SharedPreferences | 15 | P2 | 低 |
| `07-Database/` | SQLite/RdbStore | 20 | P2 | 中 |
| `08-Network/` | HTTP 网络 | 3 | P2 | 低 |
| `09-Notification/` | 通知 | 5 | P3 | 中 |
| `10-Media/` | 音频/MediaPlayer | 19 | P3 | 高 |
| `11-DeviceInfo/` | 设备信息/电话 | 10 | P3 | 低 |
| `12-Sensor/` | 传感器/位置 | 4 | P3 | 中 |
| `13-Clipboard/` | 剪贴板/振动 | 5 | P3 | 低 |
| `14-ArkUI/` | ArkUI 节点 | 14 | P0 | 高 |
| `15-Package/` | 包管理 | 0 (via AMS) | P0 | 中 |
| *其他* | 权限/定时器 | 6 | P2 | 低 |
| **合计** | | **179** | | |

## 关键技术基础 (来自 Oracle H35 Wine 分析)

- **string_bridge** (809行 C代码): 所有 15 个 Plugin 共用的 JNI 字符串转换 RAII 框架
  - 路径: `00-Infrastructure/string_bridge/`
  - 栈缓冲 1KB 覆盖 >90% 转换，零堆分配
  - 线程安全 (thread_local)
  - Wine 30年验证的 A→W 转发模式
- **Wine 五大设计原则**: 见 `00-Infrastructure/WINE-INSIGHTS.md`
- **Linux 6.6 内核共享**: OH V7.0 和 AOSP V15 共用内核，syscall ABI 一致
- **终态**: GitHub 开源 → Issue 自动修 → 自动发版，不需要大团队

## 相关文件

- `MASTER-PLAN.md` — 200步×179 API 完整计划
- `00-Infrastructure/WINE-INSIGHTS.md` — Wine 设计洞察融入说明
- `00-Infrastructure/string_bridge/` — JNI 字符串转换基础库
- `../../../16.21-Diff/refreshed-plan-v2.md` — 详细 200 步计划 (含代码示例)

## 每个 Plugin 目录结构

```
XX-Name/
  api-spec.yaml        # 179 API 中属于此 plugin 的完整规格
  src/                  # Bridge 实现 (C++ for JNI)
  mock/                 # Mock 实现 (用于主机测试)
  test/                 # 单元测试 + API 行为对比测试
  baseline/             # D200+D600 采集的 API 行为基线
  BUILD.gn              # OH 编译配置
  Makefile              # 主机 x86 编译配置
  PLAN.md               # 此 plugin 的具体执行步骤
  STATUS.md             # 当前进度
```

## 执行顺序

```
Phase 0: 00-Infrastructure (Mock + 快照 + 工具链)
Phase 1: 01-Logging (最简单，4 API，验证工具链)
Phase 2: 02-Activity + 03-Window + 15-Package (Hello World 必需)
Phase 3: 04-Rendering + 14-ArkUI (显示必需)
Phase 4: 05-Input (交互必需)
Phase 5: 06-Preferences + 07-Database + 08-Network (应用功能)
Phase 6: 09~13 (系统能力)
Phase 7: 90-Integration + 91-DeviceVerify (确认式上真机)
```
