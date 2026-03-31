# Step 3 — OH 编译 → 签名 → 安装 → 截图

**日期**: 2026-03-31
**状态**: PASS

## 目的

打通 OH 应用从源码到设备运行的全链路：命令行编译 HAP → 自签名 → 安装到 D200 → 截图验证 UI 显示。

## 输出

| 产物 | 路径 | 大小 |
|------|------|------|
| 未签名 HAP | `entry/build/default/outputs/default/entry-default-unsigned.hap` | 76 KB |
| 签名 HAP | `entry/build/default/outputs/default/entry-default-signed.hap` | ~80 KB |
| 截图 | `hello_world_d200_3.jpeg` | 25 KB (720x1280) |
| 编译脚本 | `build.bat` | — |
| 签名材料 | `signing2/` (keystore.p12, *.cer, profile.p7b) | — |

### 编译链路

```
build.bat → hvigor 6.22.3 assembleHap → entry-default-unsigned.hap
  需要: JAVA_HOME (DevEco JBR 21) + Node 18 + OH SDK (API 22)
```

### 签名链路

```
hap-sign-tool.jar:
  generate-keypair (app-key, ECC P-256)
  generate-ca → root.cer
  generate-ca → sub.cer (issuer=root)
  generate-app-cert → app.cer
  generate-keypair (profile-key)
  generate-profile-cert → profile.cer
  sign-profile (SDK模板 + 自定义UDID/bundleName) → profile.p7b
  sign-app (cert-chain + profile) → signed.hap
```

### 设备信任注入

D200 (root) 修改两个系统文件后重启 foundation:
- `/system/etc/security/trusted_root_ca.json` — 添加自签 RootCA
- `/system/etc/security/trusted_apps_sources.json` — 添加 "WestLake debug apps" 信任源

## 遇到的问题

共 **6 个阻塞问题**，全部解决:

### 1. hvigor 找不到 hvigor-config.json5
- 现象: `hvigor-config.json5 does not exist`
- 解决: 创建 `hvigor/hvigor-config.json5`

### 2. build-profile.json5 schema 不兼容 hvigor 6.x
- 现象: `compileSdkVersion` 放在 `app` 层级报 schema 错误
- 解决: 移到 `products[0]` 下面（hvigor 6.x 格式变化）

### 3. SDK 目录结构不匹配
- 现象: `Unable to find toolchains:22, ArkTS:22...`
- 原因: hvigor 期望 `sdk/22/ets/` 但实际是 `sdk/openharmony/ets/`
- 解决: Windows `mklink /J` 创建 junction `sdk/22/` → `sdk/openharmony/`
- 坑: WSL symlink 对 Windows node.exe 不可见，必须用 Windows junction

### 4. Java 不在 PATH（spawn java ENOENT）
- 现象: ArkTS 编译成功但 PackageHap 失败
- 原因: WSL 环境变量对 Windows node.exe 不可见
- 解决: 改用 `build.bat` 在 Windows 侧设 JAVA_HOME + PATH

### 5. sign-app "Illegal base64 character 20" 错误
- 现象: 所有自定义 profile template 签名后，sign-app 都报 base64 错误
- 根因: Python json.dump 写入的 cert 在 profile 中格式与 SDK 原生模板有细微差异
- 解决: 使用 SDK 原生模板 `UnsgnedDebugProfileTemplate.json` 作为基础，仅修改 `bundle-name` + `device-ids`
- 额外发现: `appCertFile` 必须是证书链（app.cer + sub.cer + root.cer 拼接），单证书报 "must be cert chain"

### 6. 设备不信任自签名证书
- 现象: 依次遇到 `fail to verify pkcs7 file` → `not trusted app source` → `device is unauthorized`
- 解决: 三步注入
  1. trusted_root_ca.json 添加 RootCA
  2. trusted_apps_sources.json 添加信任源（匹配 app/profile cert subject）
  3. profile template 填入 D200 的 UDID（通过 `bm get --udid` 获取）
