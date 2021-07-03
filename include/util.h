#ifndef HEADERS_UTIL_H
#define HEADERS_UTIL_H
#include <minwindef.h>

BOOLEAN Util_GetFolderSelection(char* pathSelected, const char* message, const char* initialPath);
BOOLEAN Util_GetFileSelection(char* fileSelected, const char* initialDir);

#endif