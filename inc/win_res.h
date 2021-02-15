#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

char* get_win_error()
{
    DWORD error_code = GetLastError();

    char *error_msg = (char*)malloc(sizeof(*error_msg)*200);
    
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, // format as error from system
                    NULL, // aditional parameter
                    error_code, // DWORD error code
                    MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // language to format string
                    (LPSTR)error_msg, // string, cast before passing
                    200, // size
                    NULL // aditional parameters
                    );

    return error_msg;
}

char* get_win_resource_binary_data(char* token){

    HRSRC res_handle = FindResourceA(NULL, token, (LPCSTR)RT_RCDATA); // MAKEINTRESOURCEA(10) == RCDATA resource 

    HGLOBAL my_resource = LoadResource(NULL, res_handle);

    char* stream = (char*)LockResource(my_resource);

    size_t size = SizeofResource(NULL, res_handle);

    char *return_string = (char*)calloc(size + 1, sizeof(char));

    strncpy(return_string, stream, size);

    FreeResource(my_resource);
    CloseHandle(res_handle);

    return_string[size] = '\0';

    return return_string;
}