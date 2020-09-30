#include <stdlib.h>
#include <stdarg.h>
#include "logger.h"
#include "printf.h"

static void Copy(Logger_buff_t *dest, const Logger_buff_t *src);
static size_t Len(const Logger_buff_t* src);

struct Logger_ADT
{
    Logger_buff_t           _buff[LOGGER_MAX_BUFF_SIZE];
    Stream_function_t       _default_stream_line;
    Stream_function_t       _output_stream[2];
};

// typedef struct Logger_ADT*              Logger;
static struct Logger_ADT logger;

static Logger_buff_t logger_header_info[] = "[info]: ";
static Logger_buff_t logger_header_error[] = "[Erro]: ";
static Logger_buff_t logger_header_warning[] = "[Warn]: ";

void Logger_Init(Stream_function_t streams[],Stream_function_t line)
{
    logger._output_stream[0] = streams[0];
    logger._output_stream[1] = streams[1];
    logger._default_stream_line = line;
}


void Logger_Publish_Detailed(Logger_importance_level_t level,
                             Logger_stream_line_t dest,
                             const char *function_name,
                             const char *fmt, ...)
{
    if (level == level_info)
        Copy(logger._buff, logger_header_info);
    else if (level == level_warning)
        Copy(logger._buff, logger_header_warning);
    else
        Copy(logger._buff, logger_header_error);

    uint16_t idx = LOGGER_MAX_HEADER_SIZE;
    logger._buff[idx++] = '[';
    Copy(logger._buff+idx,function_name);
    size_t len = Len(function_name);
    idx += len;
    logger._buff[idx++] = ']';
    logger._buff[idx++] = ' ';

    va_list va;
    va_start(va, fmt);
    vsnprintf(logger._buff + idx, 
             (size_t)(LOGGER_MAX_BUFF_SIZE - LOGGER_MAX_HEADER_SIZE - len),
             fmt, va);
    va_end(va);

    if (dest == dest_stream_0)
        logger._output_stream[0](logger._buff);
    if (dest == dest_stream_1)
        logger._output_stream[1](logger._buff);
}


void Logger_Publish_Message(Logger_importance_level_t level,
                           Logger_stream_line_t dest,
                           Logger_buff_t *msg)
{
    if (level == level_info)
        Copy(logger._buff, logger_header_info);
    else if (level == level_warning)
        Copy(logger._buff, logger_header_warning);
    else
        Copy(logger._buff, logger_header_error);

    Copy(logger._buff + LOGGER_MAX_HEADER_SIZE , msg);

    if (dest == dest_stream_0)
        logger._output_stream[0](logger._buff);
    if (dest == dest_stream_1)
        logger._output_stream[1](logger._buff);
}

void Logger_Publish_Simple(const char* fmt, ...)
{
    Copy(logger._buff, logger_header_info);
    va_list va;
    va_start(va, fmt);
    vsnprintf(logger._buff + LOGGER_MAX_HEADER_SIZE,
             (size_t)(LOGGER_MAX_BUFF_SIZE - LOGGER_MAX_HEADER_SIZE),
             fmt, va);
    va_end(va);
    logger._default_stream_line(logger._buff);
}

static void Copy(Logger_buff_t *dest, const Logger_buff_t *src)
{
    while (*src)
        *dest++ = *src++;
}

static size_t Len(const Logger_buff_t* src)
{
    int len = 0;
    
    while(*src++)
        len++;

    return len;
}
void Logger_DeInit()
{
}

void _putchar(char character)
{
}
