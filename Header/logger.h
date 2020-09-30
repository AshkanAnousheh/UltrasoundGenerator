#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

/* 
    Logger configuration
*/
#define LOGGER_SWITCH_ON
#define LOGGER_PRINT_ON

#define LOGGER_MAX_BUFF_SIZE    100
#define LOGGER_MAX_HEADER_SIZE  8

#define LOGGER_MODE_DETAILED
// #define LOGGER_MODE_MESSAGE

#define LOGGER_LEVEL_INFO
#define LOGGER_LEVEL_WARNING
#define LOGGER_LEVEL_ERROR


/* Enums */
enum Logger_stream_line 
{
    dest_stream_0 = 100,
    dest_stream_1,
    dest_stream_2,
    dest_stream_3,
    dest_stream_4,
    dest_broadcast
};

enum Logger_importance_level
{
    level_info,
    level_warning,
    level_error
};

/* 
    Macros    
*/

#ifdef LOGGER_SWITCH_ON

#if     defined(LOGGER_MODE_DETAILED)
#define LOGGER( level,dest,... )      Logger_Publish_Detailed(level,dest,__func__,__VA_ARGS__ )
#elif   defined(LOGGER_MODE_MESSAGE)
#define LOGGER( level,dest,msg,... )  Logger_Publish_Message( level,dest,msg )
#else
#error Please define proper mode for LOGGER system (in logger.h file)
#endif

#if   defined(LOGGER_PRINT_ON)
#define LOGGER_PRINT( ... )           Logger_Publish_Simple( __VA_ARGS__ )
#endif

#else
#define LOGGER( ... )                        while(0)
#endif

// #if (LOGGER_GLOBAL_SWITCH && LOGGER_INFO_SWITCH)


/* Data Types */
// logger abstract data type (adt)
typedef char                            Logger_buff_t;

typedef enum Logger_stream_line         Logger_stream_line_t;
typedef enum Logger_importance_level    Logger_importance_level_t;
typedef void (*Stream_function_t) (const Logger_buff_t*);

/* Interface Functions */

void    Logger_Init(Stream_function_t streams[],Stream_function_t line);
void    Logger_Publish_Detailed(Logger_importance_level_t level,
                                Logger_stream_line_t dest,
                                const char* function_name,
                                const char* fmt, ...);

void    Logger_Publish_Message(Logger_importance_level_t level,
                              Logger_stream_line_t dest,
                              Logger_buff_t* msg
                              );
void    Logger_Publish_Simple(const char* fmt, ...);
void    Logger_DeInit();

#endif
