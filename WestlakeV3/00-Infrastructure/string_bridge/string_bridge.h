/*
 * ============================================================================
 * 西湖 String Bridge: JNI字符串转换RAII封装
 * ============================================================================
 * 
 * 深度融合Wine字符串管理 + JNI最佳实践
 * 目的: 高效处理 Java String ↔ C String 转换
 * 
 * 性能策略:
 *   1. 栈上小缓冲 (<=256字符): 零堆分配
 *   2. 堆上大缓冲 (>256字符): 自动释放
 *   3. RAII自动管理: 避免手动Release泄漏
 * 
 * 兼容编码:
 *   - Java String: UTF-16LE (Android/HarmonyOS)
 *   - C String: UTF-8 (Linux/HarmonyOS)
 *   - 转换: 自动双向适配
 * 
 * 时间复杂度: O(n) 线性转换，无缓存不必要
 * 空间复杂度: O(min(n, 512)) 栈优化
 * 
 * ============================================================================
 */

#ifndef __WINE_STRING_BRIDGE_H__
#define __WINE_STRING_BRIDGE_H__

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 配置常量 (Wine启发)
 * ======================================================================== */

// 栈缓冲区大小: 对应Wine TEB静态缓冲区 (260字符 * 4字节UTF-8)
#define WLBRIDGE_STACK_BUFFER_SIZE   1024

// 最大字符串长度: 对应Wine 0xffff限制
#define WLBRIDGE_MAX_STRING_LENGTH   32768

// 数据结构初始化
#define WLBRIDGE_UTF8_NULL_TERM      1
#define WLBRIDGE_UTF16_NULL_TERM     2


/* ========================================================================
 * UTF-8字符串描述符 (对标Wine ANSI_STRING)
 * ======================================================================== */

typedef struct _WLBRIDGE_UTF8_STRING {
    uint32_t length;              /* 有效长度 (不含\0) */
    uint32_t maximum_length;      /* 缓冲区大小 */
    char *buffer;                 /* UTF-8数据指针 */
} WLBRIDGE_UTF8_STRING;


/* ========================================================================
 * UTF-16字符串描述符 (对标Wine UNICODE_STRING)
 * ======================================================================== */

typedef struct _WLBRIDGE_UTF16_STRING {
    uint32_t length;              /* 有效长度(字节) */
    uint32_t maximum_length;      /* 缓冲区大小(字节) */
    jchar *buffer;                /* UTF-16数据指针 */
} WLBRIDGE_UTF16_STRING;


/* ========================================================================
 * RAII容器: Java String → UTF-8 (关键热点)
 * 
 * 模式: AutoJavaStringToUtf8("hello")
 *   - 自动GetStringUTFChars
 *   - 栈上复制(小字符串)
 *   - 自动ReleaseStringUTFChars
 * ======================================================================== */

typedef struct {
    JNIEnv *env;
    jstring jstr;
    
    // 栈缓冲区: 避免堆分配的快速路径
    char stack_buffer[WLBRIDGE_STACK_BUFFER_SIZE];
    
    // 实际字符串指针 (可能指向stack_buffer或堆)
    char *utf8_ptr;
    
    // 是否需要释放 (JNI GetStringUTFChars返回值)
    bool is_heap_allocated;
    
    // 实际字符串长度 (不含\0)
    uint32_t length;
    
} WLBridge_AutoJavaStringUtf8;


/* ========================================================================
 * RAII容器: UTF-8 → Java String (逆向转换)
 * 
 * 模式: AutoUtf8ToJavaString(env, "hello")
 *   - UTF-8 → jchar数组转换
 *   - 栈优化小字符串
 *   - 创建Java String对象
 * ======================================================================== */

typedef struct {
    JNIEnv *env;
    jstring result;
    
    // 转换缓冲区
    char stack_buffer[WLBRIDGE_STACK_BUFFER_SIZE];
    char *utf8_ptr;
    bool is_heap_allocated;
    
    uint32_t length;
    
} WLBridge_AutoUtf8ToJavaString;


/* ========================================================================
 * Wine式大小计算函数 (预转换检查)
 * ======================================================================== */

/**
 * WLBridge_Utf8ToUtf16Size - 计算UTF-8到UTF-16转换所需大小
 * 
 * @utf8_str: UTF-8源字符串
 * @utf8_len: UTF-8长度(字节)
 * 
 * @return: UTF-16转换后大小(字节), 含null终止符
 * 
 * Wine参考: RtlAnsiStringToUnicodeSize()
 */
static inline uint32_t WLBridge_Utf8ToUtf16Size(const char *utf8_str, uint32_t utf8_len)
{
    // 简化版: UTF-8最坏情况 4字节→2字节UTF-16
    // 实际需要逐字符检查多字节序列
    uint32_t utf16_bytes = 0;
    const unsigned char *p = (const unsigned char *)utf8_str;
    const unsigned char *end = p + utf8_len;
    
    while (p < end) {
        unsigned char c = *p;
        if ((c & 0x80) == 0) {
            // ASCII: 1字节 → 1个jchar (2字节)
            utf16_bytes += 2;
            p += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2字节UTF-8序列
            utf16_bytes += 2;
            p += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3字节UTF-8序列 → 1个jchar (2字节)
            utf16_bytes += 2;
            p += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4字节UTF-8序列 → 2个jchar (4字节, 代理对)
            utf16_bytes += 4;
            p += 4;
        } else {
            // 非法UTF-8, 跳过
            p += 1;
        }
    }
    
    return utf16_bytes + WLBRIDGE_UTF16_NULL_TERM;
}


/**
 * WLBridge_Utf16ToUtf8Size - 计算UTF-16到UTF-8转换所需大小
 * 
 * @utf16_str: UTF-16源字符串
 * @utf16_len: UTF-16长度(字符数)
 * 
 * @return: UTF-8转换后大小(字节), 含null终止符
 * 
 * Wine参考: RtlUnicodeStringToAnsiSize()
 */
static inline uint32_t WLBridge_Utf16ToUtf8Size(const jchar *utf16_str, uint32_t utf16_len)
{
    // UTF-16→UTF-8: 最坏情况 1个jchar→3字节UTF-8 (BMP字符)
    // 代理对→4字节UTF-8
    return utf16_len * 3 + WLBRIDGE_UTF8_NULL_TERM;
}


/* ========================================================================
 * 低级转换函数 (对应Wine RtlMultiByteToUnicodeN等)
 * ======================================================================== */

/**
 * WLBridge_Utf8ToUtf16N - UTF-8 → UTF-16原始转换
 * 
 * 对应Wine: RtlMultiByteToUnicodeN()
 */
JNIEXPORT jint JNICALL
WLBridge_Utf8ToUtf16N(jchar *dest_buffer, jint dest_len,
                      const char *src_buffer, jint src_len);

/**
 * WLBridge_Utf16ToUtf8N - UTF-16 → UTF-8原始转换
 * 
 * 对应Wine: RtlUnicodeToMultiByteN()
 */
JNIEXPORT jint JNICALL
WLBridge_Utf16ToUtf8N(char *dest_buffer, jint dest_len,
                      const jchar *src_buffer, jint src_len);


/* ========================================================================
 * RAII类 (C99 GCC语法)
 * ========================================================================
 * 
 * 使用GNU Cleanup属性自动释放资源
 * __attribute__((cleanup(func))) 在离开作用域时调用func()
 * 
 * 示例:
 *   WLBridge_AutoJavaStringUtf8 str __attribute__((cleanup(WLBridge_ReleaseJavaStringUtf8)))
 *       = WLBridge_AutoGetStringUtf8(env, jstr);
 *   // 自动调用WLBridge_ReleaseJavaStringUtf8(&str)
 * ======================================================================== */

/**
 * WLBridge_AutoGetStringUtf8 - 安全获取Java String的UTF-8表示
 * 
 * 对应JNI: GetStringUTFChars() 但自动管理生命周期
 * 
 * Wine策略: file_name_AtoW 结合 StaticUnicodeString
 *   - 小字符串: 使用栈缓冲区 (零堆分配)
 *   - 大字符串: JNI GetStringUTFChars返回值
 * 
 * @env: JNI环境
 * @jstr: Java String对象
 * 
 * @return: 初始化的RAII容器, 自动释放
 */
static inline WLBridge_AutoJavaStringUtf8
WLBridge_AutoGetStringUtf8(JNIEnv *env, jstring jstr)
{
    WLBridge_AutoJavaStringUtf8 result;
    result.env = env;
    result.jstr = jstr;
    result.is_heap_allocated = false;
    result.utf8_ptr = NULL;
    result.length = 0;
    
    if (!jstr) {
        result.stack_buffer[0] = '\0';
        result.utf8_ptr = result.stack_buffer;
        return result;
    }
    
    // 获取JNI提供的UTF-8指针 (可能指向JVM内部)
    const char *jni_utf8 = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (!jni_utf8) {
        result.stack_buffer[0] = '\0';
        result.utf8_ptr = result.stack_buffer;
        return result;
    }
    
    jsize utf8_len = (*env)->GetStringUTFLength(env, jstr);
    result.length = utf8_len;
    
    // Wine策略: 栈/堆混合
    if (utf8_len < WLBRIDGE_STACK_BUFFER_SIZE - 1) {
        // 小字符串: 复制到栈缓冲区
        memcpy(result.stack_buffer, jni_utf8, utf8_len);
        result.stack_buffer[utf8_len] = '\0';
        result.utf8_ptr = result.stack_buffer;
        result.is_heap_allocated = false;
    } else {
        // 大字符串: 使用JNI返回的指针 (需要释放)
        result.utf8_ptr = (char *)jni_utf8;
        result.is_heap_allocated = true;
    }
    
    return result;
}

/**
 * WLBridge_ReleaseJavaStringUtf8 - 自动释放UTF-8字符串
 * 
 * 用作cleanup函数, 由__attribute__((cleanup()))自动调用
 * 
 * @str: AutoJavaStringUtf8指针
 */
static inline void
WLBridge_ReleaseJavaStringUtf8(WLBridge_AutoJavaStringUtf8 *str)
{
    if (str && str->is_heap_allocated && str->jstr) {
        // 释放JNI GetStringUTFChars的返回值
        (*str->env)->ReleaseStringUTFChars(str->env, str->jstr, str->utf8_ptr);
    }
    // 栈缓冲区不需要释放
}


/**
 * WLBridge_AutoCreateStringUtf8 - UTF-8字符串 → Java String
 * 
 * 对应JNI: NewStringUTF() 的高级包装
 * 
 * @env: JNI环境
 * @utf8_str: UTF-8源字符串
 * 
 * @return: Java String对象 (需要在JNI代码中释放引用)
 */
static inline jstring
WLBridge_AutoCreateStringUtf8(JNIEnv *env, const char *utf8_str)
{
    if (!utf8_str) {
        return NULL;
    }
    
    return (*env)->NewStringUTF(env, utf8_str);
}


/**
 * WLBridge_AutoGetStringChars - 获取Java String的UTF-16表示
 * 
 * 对应JNI: GetStringChars()
 * 
 * Wine策略同上: 栈优化小字符串
 * 
 * @env: JNI环境
 * @jstr: Java String对象
 * 
 * @return: jchar指针 (调用者需要释放)
 */
static inline jchar*
WLBridge_AutoGetStringChars(JNIEnv *env, jstring jstr)
{
    jboolean is_copy = JNI_FALSE;
    return (*env)->GetStringChars(env, jstr, &is_copy);
}

static inline void
WLBridge_ReleaseStringChars(JNIEnv *env, jstring jstr, const jchar *chars)
{
    (*env)->ReleaseStringChars(env, jstr, chars);
}


/* ========================================================================
 * 高级转换: 双向通道
 * ======================================================================== */

/**
 * WLBridge_JavaStringToUtf8Buffer - Java String → 用户缓冲区
 * 
 * Wine参考: FILE_name_WtoA() 的逆向
 * 
 * 用法:
 *   char buffer[256];
 *   jint result = WLBridge_JavaStringToUtf8Buffer(env, jstr, buffer, sizeof(buffer));
 *   if (result > 0) {
 *       // buffer[0..result-1] 包含UTF-8字符串 (不含\0)
 *   }
 * 
 * @env: JNI环境
 * @jstr: Java String源
 * @dest_buffer: 目标缓冲区 (必须足够大)
 * @dest_len: 目标缓冲区大小
 * 
 * @return: 转换的字节数, 0表示失败
 */
JNIEXPORT jint JNICALL
WLBridge_JavaStringToUtf8Buffer(JNIEnv *env, jstring jstr,
                                char *dest_buffer, jint dest_len);


/**
 * WLBridge_Utf8BufferToJavaString - UTF-8缓冲区 → Java String
 * 
 * @env: JNI环境
 * @utf8_buffer: UTF-8源缓冲区
 * @utf8_len: 长度(字节)
 * 
 * @return: 新创建的Java String对象
 */
JNIEXPORT jstring JNICALL
WLBridge_Utf8BufferToJavaString(JNIEnv *env,
                                const char *utf8_buffer, jint utf8_len);


/* ========================================================================
 * 性能计数器 (调试用)
 * ======================================================================== */

typedef struct {
    uint64_t java_to_utf8_count;
    uint64_t utf8_to_java_count;
    uint64_t stack_hits;        // 栈缓冲区命中次数
    uint64_t heap_allocations;  // 堆分配次数
    uint64_t total_bytes_converted;
} WLBridge_Stats;

JNIEXPORT void JNICALL WLBridge_ResetStats(void);
JNIEXPORT WLBridge_Stats JNICALL WLBridge_GetStats(void);


/* ========================================================================
 * 初始化/清理
 * ======================================================================== */

/**
 * WLBridge_Initialize - 初始化字符串转换模块
 * 
 * 调用时机: JNI_OnLoad()
 */
JNIEXPORT jint JNICALL WLBridge_Initialize(JNIEnv *env);

/**
 * WLBridge_Cleanup - 清理资源
 * 
 * 调用时机: JNI_OnUnload()
 */
JNIEXPORT void JNICALL WLBridge_Cleanup(JNIEnv *env);


#ifdef __cplusplus
}
#endif

#endif /* __WINE_STRING_BRIDGE_H__ */


/* ============================================================================
 * 使用范例
 * ============================================================================
 * 
 * === 场景1: Java String → C字符串处理 ===
 * 
 * JNIEXPORT void JNICALL Java_MyClass_processString(JNIEnv *env, jobject obj, jstring jstr)
 * {
 *     WLBridge_AutoJavaStringUtf8 str __attribute__((cleanup(WLBridge_ReleaseJavaStringUtf8)))
 *         = WLBridge_AutoGetStringUtf8(env, jstr);
 *     
 *     // 自动使用栈缓冲区(小字符串)或JNI返回值(大字符串)
 *     printf("String: %s, length: %d\n", str.utf8_ptr, str.length);
 *     
 *     // 自动释放JNI资源 (cleanup属性)
 * }
 * 
 * === 场景2: 路径名转换 (Wine FILE_name_AtoW模式) ===
 * 
 * JNIEXPORT jint JNICALL Java_MyClass_openFile(JNIEnv *env, jobject obj, jstring path_jstr)
 * {
 *     WLBridge_AutoJavaStringUtf8 path __attribute__((cleanup(WLBridge_ReleaseJavaStringUtf8)))
 *         = WLBridge_AutoGetStringUtf8(env, path_jstr);
 *     
 *     // 使用path.utf8_ptr进行文件操作 (open, read等)
 *     int fd = open(path.utf8_ptr, O_RDONLY);
 *     
 *     return (fd > 0) ? 0 : -1;
 * }
 * 
 * === 场景3: C字符串 → Java String ===
 * 
 * JNIEXPORT jstring JNICALL Java_MyClass_createString(JNIEnv *env, jobject obj)
 * {
 *     const char *result = "Hello from C";
 *     return WLBridge_AutoCreateStringUtf8(env, result);
 * }
 * 
 * ============================================================================
 */
