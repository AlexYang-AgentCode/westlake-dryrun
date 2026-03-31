# Step 1 — 环境确认

**日期**: 2026-03-30
**状态**: PASS

## 目的

确认双设备物理连接可用，明确硬件型号、OS 版本、API 级别、工具链路径。为后续所有步骤建立基线环境。

## 输出

| 项目 | D200 (鸿蒙侧) | D600 (Android 侧) |
|------|---------------|-------------------|
| 硬件 | DAYU200 / RK3568 | DAYU600 / UIS7885 |
| 系统 | OpenHarmony 7.0.0.18 | AOSP V16 (API 36) |
| API | 24 | 36 |
| 工具 | hdc.exe (DevEco SDK) | adb.exe (platform-tools) |
| Device ID | dd011a4144363141301012500404ac00 | N100CU025C18D000501 |
| hdc/adb 路径 | `D:\Program Files\Huawei\DevEco Studio\sdk\default\openharmony\toolchains\hdc.exe` | `E:\platform-tools-win\platform-tools\adb.exe` |

## 遇到的问题

无。两台设备即插即用。
