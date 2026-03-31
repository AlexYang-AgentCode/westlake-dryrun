# Step 2 — OH 原生 Hello World 代码

**日期**: 2026-03-30
**状态**: PASS

## 目的

编写一个最简 OpenHarmony 应用（ArkTS + ArkUI），验证开发工具链可用。项目作为后续 Adapter 的"宿主壳"参考。

## 输出

项目目录: `hello-world-oh/`

```
hello-world-oh/
├── AppScope/app.json5            # bundleName: com.example.helloworld
├── entry/
│   ├── src/main/ets/
│   │   ├── entryability/EntryAbility.ets   # UIAbility 生命周期
│   │   └── pages/Index.ets                 # UI: "Hello World!" + 副标题
│   └── src/main/module.json5
├── build-profile.json5           # compileSdkVersion: 22, runtimeOS: OpenHarmony
└── hvigorfile.ts
```

核心 UI (`Index.ets`):
- `Text('Hello World!')` — 36sp Bold 黑色
- `Text('Android running on OpenHarmony')` — 16sp 灰色
- 白色背景，垂直水平居中

## 遇到的问题

1. **项目骨架最初配的 API 24，但本地 SDK 只有 API 22**
   - 解决: 降级 compileSdkVersion/compatibleSdkVersion 到 22，Hello World 不依赖高版本 API
2. **entry/hvigorfile.ts 用了 `harTasks`（库模块）而非 `hapTasks`（应用模块）**
   - 解决: 改为 `import { hapTasks } from '@ohos/hvigor-ohos-plugin'`
3. **缺少 media 资源目录**（app_icon.png, startIcon.png, layered_image.json）
   - 解决: Python 生成 100x100 蓝色 PNG + layered_image.json
