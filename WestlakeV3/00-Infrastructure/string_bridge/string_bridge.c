/*
 * ============================================================================
 * 西湖 String Bridge: JNI字符串转换 - 实现文件
 * ============================================================================
 * 
 * 基于Wine源代码的优化设计
 * 实现文件位置: dlls/ntdll/rtlstr.c, dlls/kernelbase/file.c
 * 
 * ============================================================================
 */

#include "string_bridge.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/* 性能统计 (调试) */
static WLBridge_Stats g_stats = {0};

static pthread_mutex_t g_stats_lock = PTHREAD_MUTEX_INITIALIZER;


/* ========================================================================
 * 低级转换: UTF-8 ↔ UTF-16
 * ======================================================================== */

/**
 * UTF-8到UTF-16转换核心
 * 
 * 参考Wine: RtlMultiByteToUnicodeN()
 * 源代码: dlls/ntdll/rtlstr.c
 */
JNIEXPORT jint JNICALL
WLBridge_Utf8ToUtf16N(jchar *dest_buffer, jint dest_len,
                      const char *src_buffer, jint src_len)
{
    const unsigned char *src_ptr = (const unsigned char *)src_buffer;
    const unsigned char *src_end = src_ptr + src_len;
    jchar *dest_ptr = dest_buffer;
    jchar *dest_end = dest_buffer + (dest_len / sizeof(jchar));
    
    assert(dest_buffer != NULL);
    assert(src_buffer != NULL);
    
    while (src_ptr < src_end && dest_ptr < dest_end) {
        unsigned char c = *src_ptr;
        uint32_t codepoint = 0;
        int bytes_consumed = 0;
        
        /* UTF-8解码 */
        if ((c & 0x80) == 0) {
            /* ASCII: 0xxxxxxx */
            codepoint = c;
            bytes_consumed = 1;
        } else if ((c & 0xE0) == 0xC0) {
            /* 2字节: 110xxxxx 10xxxxxx */
            if (src_ptr + 1 >= src_end) break;
            codepoint = ((c & 0x1F) << 6) | (src_ptr[1] & 0x3F);
            bytes_consumed = 2;
        } else if ((c & 0xF0) == 0xE0) {
            /* 3字节: 1110xxxx 10xxxxxx 10xxxxxx */
            if (src_ptr + 2 >= src_end) break;
            codepoint = ((c & 0x0F) << 12) | ((src_ptr[1] & 0x3F) << 6) | (src_ptr[2] & 0x3F);
            bytes_consumed = 3;
        } else if ((c & 0xF8) == 0xF0) {
            /* 4字节: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx (代理对) */
            if (src_ptr + 3 >= src_end) break;
            codepoint = ((c & 0x07) << 18) | ((src_ptr[1] & 0x3F) << 12) 
                      | ((src_ptr[2] & 0x3F) << 6) | (src_ptr[3] & 0x3F);
            bytes_consumed = 4;
        } else {
            /* 非法序列, 跳过 */
            src_ptr++;
            continue;
        }
        
        /* UTF-16编码 */
        if (codepoint <= 0xFFFF) {
            /* BMP字符: 直接映射 */
            if (dest_ptr + 1 > dest_end) break;
            *dest_ptr++ = (jchar)codepoint;
        } else if (codepoint <= 0x10FFFF) {
            /* 非BMP字符: 使用代理对 */
            if (dest_ptr + 2 > dest_end) break;
            codepoint -= 0x10000;
            *dest_ptr++ = (jchar)(0xD800 + (codepoint >> 10));
            *dest_ptr++ = (jchar)(0xDC00 + (codepoint & 0x3FF));
        } else {
            /* 超出范围 */
            src_ptr++;
            continue;
        }
        
        src_ptr += bytes_consumed;
    }
    
    return (jint)((char *)dest_ptr - (char *)dest_buffer);
}


/**
 * UTF-16到UTF-8转换核心
 * 
 * 参考Wine: RtlUnicodeToMultiByteN()
 */
JNIEXPORT jint JNICALL
WLBridge_Utf16ToUtf8N(char *dest_buffer, jint dest_len,
                      const jchar *src_buffer, jint src_len)
{
    const jchar *src_ptr = src_buffer;
    const jchar *src_end = src_ptr + (src_len / sizeof(jchar));
    unsigned char *dest_ptr = (unsigned char *)dest_buffer;
    unsigned char *dest_end = dest_ptr + dest_len;
    
    assert(dest_buffer != NULL);
    assert(src_buffer != NULL);
    
    while (src_ptr < src_end && dest_ptr < dest_end) {
        jchar c = *src_ptr;
        uint32_t codepoint = 0;
        int bytes_needed = 0;
        
        /* UTF-16解码 */
        if ((c & 0xD800) != 0xD800) {
            /* BMP字符 (非代理) */
            codepoint = c;
        } else if ((c & 0xDC00) == 0xD800) {
            /* 高代理, 必须跟随低代理 */
            if (src_ptr + 1 >= src_end) break;
            jchar low = src_ptr[1];
            if ((low & 0xDC00) != 0xDC00) break; /* 非法代理对 */
            codepoint = 0x10000 + (((c & 0x3FF) << 10) | (low & 0x3FF));
            src_ptr++; /* 跳过低代理 */
        } else {
            /* 非法代理, 跳过 */
            src_ptr++;
            continue;
        }
        
        /* UTF-8编码 */
        if (codepoint <= 0x7F) {
            bytes_needed = 1;
            if (dest_ptr + 1 > dest_end) break;
            dest_ptr[0] = (unsigned char)codepoint;
        } else if (codepoint <= 0x7FF) {
            bytes_needed = 2;
            if (dest_ptr + 2 > dest_end) break;
            dest_ptr[0] = (unsigned char)(0xC0 | (codepoint >> 6));
            dest_ptr[1] = (unsigned char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            bytes_needed = 3;
            if (dest_ptr + 3 > dest_end) break;
            dest_ptr[0] = (unsigned char)(0xE0 | (codepoint >> 12));
            dest_ptr[1] = (unsigned char)(0x80 | ((codepoint >> 6) & 0x3F));
            dest_ptr[2] = (unsigned char)(0x80 | (codepoint & 0x3F));
        } else {
            bytes_needed = 4;
            if (dest_ptr + 4 > dest_end) break;
            dest_ptr[0] = (unsigned char)(0xF0 | (codepoint >> 18));
            dest_ptr[1] = (unsigned char)(0x80 | ((codepoint >> 12) & 0x3F));
            dest_ptr[2] = (unsigned char)(0x80 | ((codepoint >> 6) & 0x3F));
            dest_ptr[3] = (unsigned char)(0x80 | (codepoint & 0x3F));
        }
        
        dest_ptr += bytes_needed;
        src_ptr++;
    }
    
    return (jint)(dest_ptr - (unsigned char *)dest_buffer);
}


/* ========================================================================
 * 高级转换接口
 * ======================================================================== */

/**
 * WLBridge_JavaStringToUtf8Buffer
 * 
 * 对应Wine: FILE_name_WtoA()
 * 源代码: dlls/kernel32/file.c 118-143
 */
JNIEXPORT jint JNICALL
WLBridge_JavaStringToUtf8Buffer(JNIEnv *env, jstring jstr,
                                char *dest_buffer, jint dest_len)
{
    if (!jstr || !dest_buffer || dest_len <= 0) {
        return 0;
    }
    
    /* 获取UTF-16 */
    jboolean is_copy = JNI_FALSE;
    const jchar *utf16_ptr = (*env)->GetStringChars(env, jstr, &is_copy);
    jsize utf16_len = (*env)->GetStringLength(env, jstr);
    
    if (!utf16_ptr) {
        return 0;
    }
    
    /* 转换 UTF-16 → UTF-8 */
    jint result = WLBridge_Utf16ToUtf8N(dest_buffer, dest_len - 1,
                                        utf16_ptr, utf16_len * sizeof(jchar));
    
    /* 空终止 */
    if (result < dest_len) {
        dest_buffer[result] = '\0';
    } else {
        dest_buffer[dest_len - 1] = '\0';
    }
    
    /* 统计 */
    pthread_mutex_lock(&g_stats_lock);
    g_stats.java_to_utf8_count++;
    g_stats.total_bytes_converted += result;
    pthread_mutex_unlock(&g_stats_lock);
    
    (*env)->ReleaseStringChars(env, jstr, utf16_ptr);
    
    return result;
}


/**
 * WLBridge_Utf8BufferToJavaString
 * 
 * 对应JNI: NewStringUTF()
 */
JNIEXPORT jstring JNICALL
WLBridge_Utf8BufferToJavaString(JNIEnv *env,
                                const char *utf8_buffer, jint utf8_len)
{
    if (!utf8_buffer || utf8_len <= 0) {
        return NULL;
    }
    
    /* 计算UTF-16大小 */
    uint32_t utf16_size = WLBridge_Utf8ToUtf16Size(utf8_buffer, utf8_len);
    
    if (utf16_size > WLBRIDGE_MAX_STRING_LENGTH) {
        return NULL;
    }
    
    /* 分配临时缓冲区 (栈优化) */
    char stack_buffer[WLBRIDGE_STACK_BUFFER_SIZE];
    jchar *utf16_buffer = NULL;
    bool is_heap = false;
    
    if (utf16_size <= sizeof(stack_buffer)) {
        utf16_buffer = (jchar *)stack_buffer;
    } else {
        utf16_buffer = (jchar *)malloc(utf16_size);
        if (!utf16_buffer) {
            return NULL;
        }
        is_heap = true;
    }
    
    /* 转换 UTF-8 → UTF-16 */
    jint converted = WLBridge_Utf8ToUtf16N(utf16_buffer, utf16_size,
                                           utf8_buffer, utf8_len);
    
    if (converted <= 0) {
        if (is_heap) free(utf16_buffer);
        return NULL;
    }
    
    /* 创建Java String */
    jstring result = (*env)->NewString(env, utf16_buffer, converted / sizeof(jchar));
    
    /* 清理 */
    if (is_heap) {
        free(utf16_buffer);
        pthread_mutex_lock(&g_stats_lock);
        g_stats.heap_allocations++;
        pthread_mutex_unlock(&g_stats_lock);
    } else {
        pthread_mutex_lock(&g_stats_lock);
        g_stats.stack_hits++;
        pthread_mutex_unlock(&g_stats_lock);
    }
    
    pthread_mutex_lock(&g_stats_lock);
    g_stats.utf8_to_java_count++;
    g_stats.total_bytes_converted += utf8_len;
    pthread_mutex_unlock(&g_stats_lock);
    
    return result;
}


/* ========================================================================
 * 统计接口
 * ======================================================================== */

JNIEXPORT void JNICALL WLBridge_ResetStats(void)
{
    pthread_mutex_lock(&g_stats_lock);
    memset(&g_stats, 0, sizeof(g_stats));
    pthread_mutex_unlock(&g_stats_lock);
}

JNIEXPORT WLBridge_Stats JNICALL WLBridge_GetStats(void)
{
    WLBridge_Stats result;
    pthread_mutex_lock(&g_stats_lock);
    result = g_stats;
    pthread_mutex_unlock(&g_stats_lock);
    return result;
}


/* ========================================================================
 * 初始化/清理
 * ======================================================================== */

JNIEXPORT jint JNICALL WLBridge_Initialize(JNIEnv *env)
{
    /* 预留初始化逻辑 (如果需要) */
    memset(&g_stats, 0, sizeof(g_stats));
    return 0;
}

JNIEXPORT void JNICALL WLBridge_Cleanup(JNIEnv *env)
{
    /* 输出最终统计 (调试) */
    WLBridge_Stats stats = WLBridge_GetStats();
    fprintf(stderr, "[WLBridge] Statistics:\n");
    fprintf(stderr, "  Java→UTF8: %lld\n", (long long)stats.java_to_utf8_count);
    fprintf(stderr, "  UTF8→Java: %lld\n", (long long)stats.utf8_to_java_count);
    fprintf(stderr, "  Stack hits: %lld\n", (long long)stats.stack_hits);
    fprintf(stderr, "  Heap allocations: %lld\n", (long long)stats.heap_allocations);
    fprintf(stderr, "  Total bytes: %lld\n", (long long)stats.total_bytes_converted);
}

