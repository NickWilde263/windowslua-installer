#ifndef _HEAD_LOGGER_H
#define _HEAD_LOGGER_H
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

void Logger_Info(const char* source, const char* msg);
void Logger_Warn(const char* source, const char* msg);
void Logger_Error(const char* source, const char* msg);
void Logger_Debug(const char* source, const char* msg);
void Logger_Trace(const char* source, const char* msg);

#endif