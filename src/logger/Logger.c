#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "logger/Logger.h"

void Logger_CommonStub(const char* format, const char* source, const char* msg) {
  char currentTime[512];
  time_t timeStruct = time(NULL);
  strftime(currentTime, 512, "%d-%m-%Y %H:%M:%S", localtime(&timeStruct));
  
  char* line = malloc(sizeof(char) * 1);
  size_t lineSize = 0; 
  line[0] = '\0';
  
  for (int i = 0; msg[i] != 0; i++) {
    if (msg[i] == '\n') {
      printf(format, currentTime, source, line);
      free(line);
      line = malloc(sizeof(char) * 1);
      line[0] = '\0';
      lineSize = 0;
    } else {
      line[lineSize] = msg[i];
      lineSize = lineSize + 1;
      line = realloc(line, lineSize + 1);
      line[lineSize] = '\0';
    }
  }
  printf(format, currentTime, source, line);
  free(line);
}

void Logger_Info(const char* source, const char* msg) {
  Logger_CommonStub("[%s][%s/INFO] %s\n", source, msg);
}

void Logger_Warn(const char* source, const char* msg) {
  Logger_CommonStub("[%s][%s/WARN] %s\n", source, msg);
}

void Logger_Error(const char* source, const char* msg) {
  Logger_CommonStub("[%s][%s/ERROR] %s\n", source, msg);
}

void Logger_Debug(const char* source, const char* msg) {
  Logger_CommonStub("[%s][%s/DEBUG] %s\n", source, msg);
}

void Logger_Trace(const char* source, const char* msg) {
  Logger_CommonStub("[%s][%s/TRACE] %s\n", source, msg);
}