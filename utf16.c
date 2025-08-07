#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include "builtins.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

#define EPSL_COMMON_PREFIX "json_"

struct ARRAY_W16 {
    uint64_t ref_counter;
    uint64_t capacity;
    uint64_t length;
    uint16_t *content;
};

struct ARRAY_Byte {
    uint64_t ref_counter;
    uint64_t capacity;
    uint64_t length;
    unsigned char *content;
};

typedef struct ARRAY_Byte NULLABLE_ARRAY_Byte;
#include <stdio.h>
NULLABLE_ARRAY_Byte *json_utf16_to_utf_8(struct ARRAY_W16 *utf16) {
#ifdef _WIN32
    if (utf16->length > INT_MAX) return NULL;

    int utf8_size = WideCharToMultiByte(
        CP_UTF8, // dest encoding
        MB_ERR_INVALID_CHARS, // flags
        utf16->content, // src str
        utf16->length, // src len
        NULL, // dest buffer (ignored due to next param)
        0, // dest buffer size (0 indicated do not write, just calc size)
        NULL, NULL // unused arguments
    );
    if (utf8_size == 0) return NULL;
    
    char *utf8_buffer = malloc(utf8_size);
    int status = WideCharToMultiByte(
        CP_UTF8, // dest encoding
        MB_ERR_INVALID_CHARS, // flags
        utf16->content, // src str
        utf16->length, // src len
        utf8_buffer, // dest buffer
        utf8_size, // dest buffer size
        NULL, NULL // unused arguments
    );
    if (status == 0) {
        free(utf8_buffer);
        return NULL;
    }

    int utf8_len = utf8_size - 1;
#else
#if SIZE_MAX < UINT64_MAX
    if (utf16->length >= SIZE_MAX / 3) return NULL;
#endif
    
    iconv_t conv_desc = iconv_open("UTF8", "UTF16");
    if (conv_desc == (iconv_t)-1) {
        epsl_panicf("Cannot convert UTF-16 to UTF-8");
    }

    // each UTF-16 pair may result in up to 3 bytes in UTF-8
    size_t utf8_size = utf16->length * 3;
    char *utf8_buffer = malloc(utf8_size);

    char *utf16_content = (char*)utf16->content;
    size_t utf16_byte_len = utf16->length * 2;
    char *utf8_buffer_ptr = utf8_buffer;
    size_t utf8_bytes_remaining = utf8_size;

    size_t status = iconv(conv_desc,
        &utf16_content, &utf16_byte_len,
        &utf8_buffer_ptr, &utf8_bytes_remaining);
    if (status == (size_t)-1) {
        free(utf8_buffer);
        return NULL;
    }

    iconv_close(conv_desc);
    uint64_t utf8_len = utf8_size - utf8_bytes_remaining;
#endif

    struct ARRAY_Byte *result = malloc(sizeof(*result));
    result->ref_counter = 0;
    result->capacity = utf8_size;
    result->length = utf8_len;
    result->content = (unsigned char*)utf8_buffer;
    return result;
}
