# Step 4-5 — Android 参考基线

**日期**: 2026-03-31
**状态**: PASS

## 目的

在 D600 上运行同样的 Hello World APK，截图建立 Android 端视觉基准，供后续 Adapter 版本做像素级对比。

## 输出

| 产物 | 路径 | 大小 |
|------|------|------|
| APK | `hello-world-android/build/apk/hello-world.apk` | 13 KB |
| 截图 | `hello_world_d600_3.png` | 38 KB (1080x1920) |
| Android SDK | `android-sdk/` (build-tools 35.0.0 + platform-23) | ~200 MB |
| 编译脚本 | `build-android.bat` | — |

### 编译链路（无 Android Studio / Gradle）

```
aapt2 compile → aapt2 link → javac (JBR 21) → d8 → zip → zipalign → apksigner
```

### 视觉对比基线

| 维度 | D200 (OH) | D600 (Android) |
|------|-----------|----------------|
| 标题 | Hello World! | Hello World! |
| 副标题 | Android running on OpenHarmony | Android running on Android |
| 字号 | 36sp Bold / 16sp | 36sp Bold / 16sp |
| 背景 | #FFFFFF | #FFFFFF |
| 分辨率 | 720x1280 | 1080x1920 |

## 遇到的问题

### 1. 本机无 Android SDK
- 解决: 下载 cmdline-tools，sdkmanager 安装 build-tools + platform

### 2. Windows javac 不认 Linux 路径
- 解决: 用 batch 文件（build-android.bat）跑 javac，其余步骤用 WSL Linux 工具

### 3. AOSP V16 要求 targetSdkVersion >= 24
- 现象: `INSTALL_FAILED_DEPRECATED_SDK_VERSION`
- 解决: AndroidManifest.xml 添加 `<uses-sdk android:targetSdkVersion="36" />`

### 4. resources.arsc 必须 uncompressed
- 现象: `Targeting R+ requires resources.arsc to be stored uncompressed`
- 解决: `zip -0` 单独存储 resources.arsc，其余文件正常压缩

### 5. D600 屏幕休眠
- 解决: `adb shell input keyevent KEYCODE_WAKEUP` + `wm dismiss-keyguard`
