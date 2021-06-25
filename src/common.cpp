#include "common.h"

#include <Windows.h>
#include <cstdarg>
#include <cstdio>

const WORD FG_LIGHT_GREY = 7;
const WORD FG_YELLOW = 14;
const WORD FG_MAGENTA = 13;
const WORD FG_CYAN = 11;

// extract filename from __FILE__
static const char *get_filename(const char *file_macro)
{
    size_t len = strlen(file_macro);
    for (const char *end = file_macro + len;
         end != file_macro;
         end--) 
    {
        if (end[0] == '\\')
            return end + 1;
    }
    return file_macro;
}

static void print_color_string(WORD color, const char *string)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Save current attributes
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(hConsole, &console_info);
    WORD saved_attributes = console_info.wAttributes;

    SetConsoleTextAttribute(hConsole, color);
    fprintf(stdout, string);

    SetConsoleTextAttribute(hConsole, saved_attributes);
}

void impl_print_debug(const char *format, ...)
{
    print_color_string(FG_LIGHT_GREY, "DEBUG: ");

    va_list vlist;
    va_start(vlist, format);
    vprintf(format, vlist);
    va_end(vlist);

    printf("\n");
}

void impl_print_warning(const char *file, int line, const char *format, ...)
{
    print_color_string(FG_YELLOW, "WARNING: ");
    printf("file: %s, line: %i\n", get_filename(file), line);

    va_list vlist;
    va_start(vlist, format);
    vprintf(format, vlist);
    va_end(vlist);

    printf("\n\n");
}

void impl_print_error(const char *file, int line, const char *format, ...)
{
    print_color_string(FG_MAGENTA, "ERROR: ");
    printf("file: %s, line: %i\n", get_filename(file), line);

    va_list vlist;
    va_start(vlist, format);
    vprintf(format, vlist);
    va_end(vlist);

    printf("\n\n");
}
const char *impl_tempf(int dummy_snprintf_result, const char *format, ...)
{
    // quick and dirty way to get a formatted char buffer 
    // without always having to declare a local one
    thread_local char buf[256];
    (void)dummy_snprintf_result;

    va_list vlist;
    va_start(vlist, format);
    vsnprintf(buf, 256, format, vlist);
    va_end(vlist);

    return &buf[0];
}

uint64 
fnv64_1a(const uint8 *ptr, size_t byte_count)
{	
    static const uint64 FNV_OFFSET = 14695981039346656037ULL;
    static const uint64 FNV_PRIME = 1099511628211ULL;
    uint64 digest = FNV_OFFSET; 

    for (size_t i = 0; i < byte_count; i++)
    {
        digest = (digest ^ ptr[i]) * FNV_PRIME;
    }

    return (digest);
}
