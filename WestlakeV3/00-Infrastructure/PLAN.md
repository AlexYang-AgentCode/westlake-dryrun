# Phase 0: 基础设施 (Step 1-20)

> OH V7.0 + AOSP V15 共用 Linux 6.6 内核，底层依赖最匹配
> D200 (RK3568, OH V7.0, API 25, hdc) + D600 (UIS7885, AOSP V15, adb)

## 关键发现: Linux 6.6 内核共享

OH V7.0 和 AOSP V15 都基于 Linux 6.6 内核。这意味着:
- syscall ABI 完全一致
- bionic ↔ musl 的内核接口层 (futex/epoll/ioctl) 兼容性最高
- /proc /sys 文件系统布局一致
- Binder 驱动版本可能一致 (需验证)

这是西湖方案最佳版本组合。

## Step 1-4: 双机连接与快照采集

### Step 1: D200 连接确认 + 内核版本验证
```bash
hdc shell "uname -r"  # 预期: 6.6.x
hdc shell "cat /proc/version"
hdc shell "param get const.ohos.fullname"  # OH V7.0
```

### Step 2: D600 连接确认 + 内核版本验证
```bash
adb shell "uname -r"  # 预期: 6.6.x
adb shell "cat /proc/version"
adb shell "getprop ro.build.version.release"  # 15
```

### Step 3: D200 全量快照
采集: version, lib64, platformsdk, ps, aa, bm, hidumper, hilog, uname, cpuinfo, meminfo, mount, /proc/version

### Step 4: D600 全量快照
采集: getprop, lib64, framework, ps, dumpsys activity/package/window, uname, cpuinfo, meminfo, mount

## Step 5-8: 双机 API 行为录制

对 179 个 API 分 15 组，在两台真机上各运行一次 API Probe:
- Step 5: D200 lifecycle + rendering 行为录制
- Step 6: D600 lifecycle + rendering 行为录制
- Step 7: D200 全量 API 行为录制 (15组)
- Step 8: D600 全量 API 行为录制 (15组)

## Step 9-12: Mock 基础设施

- Step 9: 双机行为自动对比 (生成 15 组 diff 报告)
- Step 10: mock-hdc (基于 D200 快照)
- Step 11: mock-adb (基于 D600 快照)
- Step 12: API 行为回放引擎

## Step 13-16: 验证工具链

- Step 13: hdc relay 隧道 (备用)
- Step 14: verify.py 统一验证框架
  - verify_symbols: nm/readelf
  - verify_screenshot: SSIM AI Vision
  - verify_api_behavior: JSON diff
  - verify_log: 模式匹配
  - llm_judge: LLM-as-Judge
- Step 15: ralph-loop.sh 自愈引擎
- Step 16: confidence_gate.py 置信度门控

## Step 17-20: Plugin 架构 + Gate

- Step 17: 创建 15 个 Plugin 目录
- Step 18: Plugin 独立编译测试框架
- Step 19: Phase 0 Gate
- Step 20: 收起物理设备
