#ifndef _LOG_HEADER_
#define _LOG_HEADER_

#include <stdio.h>
#include <windows.h>
#include <stdarg.h>

/* ----------------------------------------- Defines ------------------------------------------ */

#define COLOR_RED_LOG       "\x1b[38;2;197;015;031m"
#define COLOR_GREEN_LOG     "\x1b[38;2;019;175;014m"
#define COLOR_BLUE_LOG      "\x1b[38;2;000;055;518m"
#define COLOR_CYAN_LOG      "\x1b[38;2;000;187;187m"
#define COLOR_YELLOW_LOG    "\x1b[38;2;193;175;000m"
#define COLOR_MAGENTA_LOG   "\x1b[38;2;211;056;211m"

#define END_ESC             "\x1b[0m"

#define LOG_FILE_LINE_STR   "[%s.%i] "

/* ----------------------------------------- Globals ------------------------------------------ */

FILE *log_out = NULL;

int log_level_internal = 2;

int log_debug_internal = 0;

/* ----------------------------------------- Prototypes --------------------------------------- */

void logprintf(int log_level, FILE *out, const char *format, ...);

/* ----------------------------------------- Macros ------------------------------------------- */

// NOTE: '##__VA_ARGS__' is a gcc implementation only

// log level 2
#define log_info(format_string, ...)            logprintf(2, log_out, COLOR_GREEN_LOG LOG_FILE_LINE_STR format_string END_ESC ,  __FILE__, __LINE__, ##__VA_ARGS__)
// log level 1
#define log_colored(color, format_string, ...)  logprintf(1, log_out, color format_string END_ESC, ##__VA_ARGS__)
#define log_client(format_string, ...)          logprintf(1, log_out, COLOR_YELLOW_LOG LOG_FILE_LINE_STR format_string END_ESC,  __FILE__, __LINE__, ##__VA_ARGS__)
#define log_server(format_string, ...)          logprintf(1, log_out, COLOR_CYAN_LOG LOG_FILE_LINE_STR format_string END_ESC,    __FILE__, __LINE__, ##__VA_ARGS__)
// log level 0
#define log_error(format_string, ...)           logprintf(0, log_out, COLOR_RED_LOG LOG_FILE_LINE_STR format_string END_ESC ,    __FILE__, __LINE__, ##__VA_ARGS__)
// log level debug, -1
#define log_debug(format_string, ...)           logprintf(-1, log_out, COLOR_MAGENTA_LOG LOG_FILE_LINE_STR "[DEBUG] " format_string END_ESC, __FILE__, __LINE__, ##__VA_ARGS__)

/* ----------------------------------------- Functions ---------------------------------------- */

void logprintf(int log_level, FILE *out, const char *format, ...){
    va_list arg_list;
    va_start(arg_list, format);
    
    if((log_level == -1) && (log_debug_internal == 1))
        vfprintf(out, format, arg_list);
    else if(log_level != -1 && log_level <= log_level_internal)
        vfprintf(out, format, arg_list);

    va_end(arg_list);
}

void log_out_set(FILE *out){
    log_out = out;
}

// enable win10 virtual terminal options
void set_cmd_colors(void){
    HANDLE cmd_screen = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD cmd_mode = 0;
    GetConsoleMode(cmd_screen, &cmd_mode);
    cmd_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(cmd_screen, cmd_mode);        
}

void set_log_level(int log_level){
    log_level_internal = log_level;
}

void set_debug_level(int log_level){
    log_debug_internal = log_level;
}


#endif