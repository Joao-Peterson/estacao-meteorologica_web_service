#ifndef _LOG_HEADER_
#define _LOG_HEADER_

#include <stdio.h>
#include <windows.h>

#define COLOR_RED_LOG       "\x1b[38;2;197;015;031m"
#define COLOR_GREEN_LOG     "\x1b[38;2;019;175;014m"
#define COLOR_BLUE_LOG      "\x1b[38;2;000;055;518m"
#define COLOR_CYAN_LOG      "\x1b[38;2;000;187;187m"
#define COLOR_YELLOW_LOG    "\x1b[38;2;193;175;000m"
#define COLOR_MAGENTA_LOG   "\x1b[38;2;211;056;211m"

#define END_ESC             "\x1b[0m"

#define LOG_FILE_LINE_STR   "[%s.%i] "

// NOTE: '##__VA_ARGS__' is a gcc implementation only
#define log_client(format_string, ...)          fprintf(log_out, COLOR_YELLOW_LOG LOG_FILE_LINE_STR format_string END_ESC,  __FILE__, __LINE__, ##__VA_ARGS__)
#define log_server(format_string, ...)          fprintf(log_out, COLOR_CYAN_LOG LOG_FILE_LINE_STR format_string END_ESC,    __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(format_string, ...)            fprintf(log_out, COLOR_GREEN_LOG LOG_FILE_LINE_STR format_string END_ESC ,  __FILE__, __LINE__, ##__VA_ARGS__)
#define log_error(format_string, ...)           fprintf(log_out, COLOR_RED_LOG LOG_FILE_LINE_STR format_string END_ESC ,    __FILE__, __LINE__, ##__VA_ARGS__)
#define log_colored(color, format_string, ...)  fprintf(log_out, color format_string END_ESC, ##__VA_ARGS__)

FILE *log_out = NULL;

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

#endif