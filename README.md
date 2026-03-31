# WestLake Adapter

> Run unmodified Android APKs on OpenHarmony — powered by AI.

## What is this?

WestLake Adapter automatically converts Android APK layouts and logic into equivalent OpenHarmony (ArkTS/ArkUI) applications. No source code modification needed.

## How it works

```
Android APK (unmodified)
  ↓ aapt2 parse manifest + layout XML
  ↓ apk-to-arkts.py converts View tree → ArkUI components
  ↓ hvigor build → hap-sign-tool sign → hdc install
  ↓
OpenHarmony device displays same UI
```

## Verified Demos

| Demo | Android Views | ArkUI Components | Interactive |
|------|--------------|------------------|-------------|
| HelloWorld | 2 TextView | 2 Text | No |
| Calculator | 16 Button + 1 TextView | 16 Button + 1 Text | Yes (+-×/) |

## Supported Mappings

### Views
| Android | ArkUI |
|---------|-------|
| LinearLayout(vertical) | Column() |
| LinearLayout(horizontal) | Row() |
| TextView | Text() |
| Button | Button() |

### Attributes
| Android | ArkUI |
|---------|-------|
| textSize (sp) | fontSize (fp) |
| textColor | fontColor |
| textStyle=bold | fontWeight(Bold) |
| background | backgroundColor |
| gravity=center | justifyContent+alignItems |
| layout_weight | layoutWeight |
| padding | padding |
| layout_margin* | margin |

## Project Structure

```
westlake-adapter/
├── adapter/                  # Core: APK → ArkTS converter
│   └── apk-to-arkts.py     # Main adapter script
├── demos/                    # Verified demo APKs + adapted projects
│   ├── hello-world/
│   └── calculator/
├── signing/                  # OH signing materials (debug)
├── scripts/                  # Build, sign, deploy automation
└── .github/workflows/        # CI/CD pipelines
```

## Self-Healing Pipeline (Demo 3)

```
User reports bug → GitHub Issue
  ↓ AI Bot auto-claims (3 min)
  ↓ Analyzes → generates fix → auto PR
  ↓ CI runs tests → SSIM visual comparison
  ↓ Auto-merge → auto-release
  ↓ User pulls hotfix (15 min total)
```

## Requirements

- OpenHarmony device (tested: DAYU200 RK3568, OH 7.0 API 24)
- Android device for baseline (tested: DAYU600 UIS7885, AOSP V16)
- DevEco Studio (hvigor + hap-sign-tool + OH SDK)
- Android SDK build-tools (aapt2 for APK parsing)

## License

Apache 2.0
