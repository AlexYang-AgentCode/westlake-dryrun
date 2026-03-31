# Wine (H35) 设计洞察融入 WestlakeV3

> 来源: Oracle (16.81) 2026-03-29 深度分析
> 原始文件: `/mnt/e/10.Project/16-WestLake/16.81-Oracale/L4-Adapter-History/`

## 1. Wine→西湖 核心映射

| Wine 概念 | 西湖等价物 | 应用位置 |
|-----------|-----------|---------|
| ANSI_STRING | UTF-8 byte buffer (JNI GetStringUTFChars) | 全部 179 API 的字符串参数 |
| UNICODE_STRING | Java String (JNI jstring) | 全部 179 API |
| FILE_name_AtoW | `WLBridge_AutoGetStringUtf8()` | JNI 入口处字符串转换 |
| RtlAnsiStringToUnicodeString | `WLBridge_Utf8ToUtf16N()` | 底层编码转换 |
| NtCurrentTeb()->StaticUnicodeString | `stack_buffer[1024]` (栈缓冲) | 热路径零堆分配 |
| CreateFileA→W 转发 | Android API A → OH API W 转发 | 所有 Bridge 函数 |

## 2. 五大设计原则 (从 Wine 30 年经验提炼)

### 原则 1: A→W 转发模式 (DRY)
**Wine 做法**: CreateFileA() 只做字符串转换，然后调用 CreateFileW()
**西湖应用**: 每个 Bridge 函数只做参数映射，然后调用 OH 原生 API
```
Android API (Java) → JNI Bridge (C++) → OH Native API (C)
     "A版本"            "转换层"           "W版本"
```
**影响**: 15 个 Plugin 的所有 Bridge 函数都遵循此模式

### 原则 2: 栈缓冲优先 (>90% 命中)
**Wine 做法**: TEB 静态缓冲覆盖 >95% 的字符串操作，仅大字符串用堆
**西湖应用**: 每个 JNI Bridge 函数使用 1KB 栈缓冲:
```c
// 所有 179 API 的 JNI 入口标准模式
JNIEXPORT void JNICALL nativeLogInfo(JNIEnv *env, jobject, jstring tag, jstring msg) {
    WLBridge_AutoJavaStringUtf8 tag_utf8;
    WLBridge_AutoGetStringUtf8(env, tag, &tag_utf8);  // 栈缓冲，零堆分配

    WLBridge_AutoJavaStringUtf8 msg_utf8;
    WLBridge_AutoGetStringUtf8(env, msg, &msg_utf8);

    OH_LOG_INFO(LOG_APP, "%{public}s", tag_utf8.str);  // 转发到 OH API
    // RAII 自动释放，不需要手动 ReleaseStringUTFChars
}
```
**影响**: 性能目标 <300ns/次 (小字符串)

### 原则 3: 两步转换 (预测+执行)
**Wine 做法**: 先 RtlAnsiStringToUnicodeSize() 算大小，再分配+转换
**西湖应用**: 对于需要类型转换的 API 参数:
1. 先计算 OH API 需要的参数格式
2. 分配精确大小的缓冲
3. 执行转换
4. 调用 OH API
```
Intent → Want 转换也遵循此模式:
  1. 计算 Want 字段数量
  2. 预分配 WantParams
  3. 填充字段
  4. 调用 StartAbility(want)
```

### 原则 4: 只增不减的符号表 (兼容性)
**Wine 做法**: 新版 Wine 永远不删除旧 API，只增加新的
**西湖应用**: OH 补丁的 .z.so 必须 nm 符号只增不减
```bash
# 验证脚本 (Step 93, 97)
nm -D patched.so | sort > new.txt
nm -D original.so | sort > old.txt
comm -23 old.txt new.txt  # 如果有输出 → 删除了符号 → 不允许!
```

### 原则 5: wine-tests 回归套件
**Wine 做法**: 每个 API 都有 wine-tests 中的行为测试
**西湖应用**: 每个 API 都有 3 级测试:
1. 单元测试 (Mock 环境)
2. API 行为对比 (vs D600 录制基线)
3. 截图 SSIM (渲染类 API)

## 3. string_bridge 融入 WestlakeV3

Oracle 已交付的 `string_bridge.h` + `string_bridge.c` (共 809 行) 直接作为 **所有 15 个 Plugin 的公共基础设施**:

```
WestlakeV3/
  00-Infrastructure/
    string_bridge/
      string_bridge.h  ← 来自 Oracle L4 分析
      string_bridge.c  ← 来自 Oracle L4 分析
```

每个 Plugin 的 JNI Bridge 代码 `#include "string_bridge.h"`:
- `01-Logging/src/oh_log_bridge.c` → #include
- `02-Activity/src/oh_ability_manager_client.c` → #include
- `03-Window/src/oh_surface_bridge.c` → #include
- ... (全部 15 个)

### 性能预期 (基于 Wine 对标)

| 操作 | Wine (TEB) | 西湖 (stack_buffer) | 目标 |
|------|-----------|--------------------|----|
| 小字符串 (<1KB) | <200ns | **<300ns** | ✅ |
| 栈缓冲命中率 | >95% | **>90%** | ✅ |
| 堆分配频率 | <5% | **<10%** | ✅ |
| 线程安全 | TEB (零竞争) | thread_local (零竞争) | ✅ |

## 4. Wine AppDB 模式 → 西湖 APK 兼容性数据库

Wine 用 AppDB 追踪每个 Windows 应用的兼容性等级:
- Platinum: 完美运行
- Gold: 可用，小问题
- Silver: 可用，较多问题
- Bronze: 基本可用
- Garbage: 不可用

**西湖等价物**: 在 Phase J (Step 199-200) 建立 APK 兼容性数据库:
```yaml
# apk-compat-db.yaml
apps:
  - name: "Hello World"
    package: "com.test.hello"
    rating: platinum
    apis_used: [logInfo, startAbility, surfaceCreate, canvasDrawText]
    tested_on: "D200 OH V7.0"

  - name: "Calculator"
    package: "com.test.calc"
    rating: gold
    apis_used: [logInfo, 21 button click events, canvasDraw*]
    issues: ["button animation slightly slower"]
```

## 5. 从 Wine 学到的反模式

| 反模式 | Wine 中的教训 | 西湖如何避免 |
|--------|-------------|-------------|
| 完美兼容追求 | Wine 30 年仍有不兼容 | 接受 80/20，聚焦 Top 100 APK |
| 忽视行为差异 | API 签名一样但行为不同 | Phase 0 双机行为录制对比 |
| 手动测试 | 早期 Wine 靠人工测 | 全 Agent 自动化 |
| 整体编译 | Wine 早期改一处全编 | Plugin 架构 (Babel 模式) |
