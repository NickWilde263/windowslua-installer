#include <stdio.h>
#include <minwindef.h>
#include <windows.h>
#include <shlobj.h>  
#include <winuser.h>
#include <fileapi.h>
#include <winreg.h>

#include "logger/Logger.h"
#include "data_lua_5_4_2_win32_bin_zip.h"
#include "asprintf.h"
#include "util.h"
#include "zip.h"
#include "regex.h"

#define LUA_VERSION "5.4.2 32-bit"

//This is Lua 5.4 installer
const char* defaultLuaInstallFolder = "C:\\Program Files\\Lua";
const char* lua54_Install_Folder = "C:\\Program Files\\Lua\\5.4\\";
char tmpFilePath[MAX_PATH] = {'\0'};
char helperScriptPath[MAX_PATH] = {'\0'};
BOOLEAN isTempFileCreated = FALSE;

int onExtractFile(const char* filename, void* arg_) {
  int* arg = (int*) arg_;
  arg[0]++;
  
  char* message;
  asprintf(&message, "Extracted file %s (%d of %d)", filename, arg[0], arg[1]);
  Logger_Info(__FILENAME__, message);
  free(message);
  
  return 0;
}

int actualMain() {
  char installDir[MAX_PATH];
  
  if (GetConsoleWindow() == NULL) {
    //Creating new console
    AllocConsole();
  }
  
  Logger_Info(__FILENAME__, "This installer for Lua " LUA_VERSION);
  {
    char* message;
    asprintf(&message, "Archive size %d bytes", data_lua_5_4_2_win32_bin_zip_length);
    Logger_Info(__FILENAME__, message);
    free(message);
  }
  
  {
    int response = MessageBox(GetConsoleWindow(), "Setup will guide you through Lua " LUA_VERSION " installation process (click ok to continue).\r\n"
                                                  "Keep the console open setup ask question in the console.\r\n"
                                                  "If setup crash try set compatibility mode to Windows XP", "Setup", MB_OKCANCEL);
    
    if (response == IDCANCEL) {
      Logger_Info(__FILENAME__, "User cancelled the setup");
      return 1;
    }
  }
  
  {
    Logger_Info(__FILENAME__, "Windows probably will prompt about UAC");
    if (CreateDirectory(defaultLuaInstallFolder, NULL) == 0 && GetLastError() == ERROR_PATH_NOT_FOUND) {
      char* message;
      asprintf(&message, "Cannot create %s directory", defaultLuaInstallFolder);
      Logger_Info(__FILENAME__, message);
      free(message);
      return 1;
    }
    
    if (CreateDirectory(lua54_Install_Folder, NULL) == 0 && GetLastError() == ERROR_ALREADY_EXISTS) {
      Logger_Warn(__FILENAME__, "Existing Lua5.4 folder installed!");
      int response = MessageBox(GetConsoleWindow(), "Warning: exiting Lua " LUA_VERSION " installation already exist. Do you want overwrite it?", "Setup", MB_YESNO | MB_ICONWARNING | MB_SYSTEMMODAL);
      if (response == IDNO) {
        Logger_Info(__FILENAME__, "User cancelled the setup (User dont want overwrite existing lua " LUA_VERSION " installation!)");
        return 1;
      }
      Logger_Warn(__FILENAME__, "Overwritting existing Lua " LUA_VERSION " install directory!");
    }
    
    re_t regex = re_compile("^[A-Z]:");
    
    while (1) {
      BOOLEAN isSelected = Util_GetFolderSelection(installDir, "Please select Lua " LUA_VERSION " installation directory\r\n"
                                                               "Note: Leave it default if you dont know what this mean", lua54_Install_Folder);
      if (!isSelected) {
        Logger_Info(__FILENAME__, "No folder selected (user canceled)");
        Logger_Info(__FILENAME__, "Exiting...");
        return 1;
      }
      
      //Verify its not network directory
      int matchLen;
      re_matchp(regex, installDir, &matchLen);
      if (matchLen > 0) {
        break;
      } else {
        Logger_Error(__FILENAME__, "Please select directory that begin in C:\\, A:\\, etc");
      }
    }
  }
  
  {
    char* message;
    asprintf(&message, "Folder %s selected as install directory", installDir);
    Logger_Info(__FILENAME__, message);
    free(message);
  }
  
  {
    MessageBox(GetConsoleWindow(), "Setup will begin installing press ok to continue", "Setup", MB_OK);
    Logger_Info(__FILENAME__, "Extracting...");
    char tmpDirectory[MAX_PATH];
    GetTempPath(MAX_PATH, tmpDirectory);
    GetTempFileName(tmpDirectory, "LUA_SETUP", 0, tmpFilePath);
    {
      char* message;
      asprintf(&message, "Preparing temporary files at %s", tmpFilePath);
      Logger_Info(__FILENAME__, message);
      free(message);
    }
    /*
    FILE* handle = fopen(tmpFilePath, "w+b");
    size_t byteWritten = fwrite(data_lua_5_4_2_win32_bin_zip, 1, data_lua_5_4_2_win32_bin_zip_length, handle);
    if (byteWritten < data_lua_5_4_2_win32_bin_zip_length) {
      printf("%d", byteWritten);
      Logger_Error(__FILENAME__, "Cannot write to temporary file");
      return 1;
    }
    fclose(handle);
    */
    
    HANDLE fileHandle = CreateFile(tmpFilePath,
                                   FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                   0,
                                   NULL,
                                   CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
    
    if (fileHandle == INVALID_HANDLE_VALUE) {
      char* message;
      asprintf(&message, "Cannot open tmp file (%s) reason 0x%08X", tmpFilePath, GetLastError());
      Logger_Error(__FILENAME__, message);
      free(message);
      return 1;
    }
    
    for (size_t i = 0; i < data_lua_5_4_2_win32_bin_zip_length; i++) {
      DWORD bytesWritten = 0;
      char singleByte = data_lua_5_4_2_win32_bin_zip[i];
      BOOLEAN result = WriteFile(fileHandle, 
                                &singleByte, 
                                1, 
                                &bytesWritten, 
                                NULL);
      
      if (result == FALSE) {
        char* message;
        asprintf(&message, "Cannot write tmp file (%s) reason 0x%08X", tmpFilePath, GetLastError());
        Logger_Error(__FILENAME__, message);
        free(message);
        return 1;
      }
    }
    
    CloseHandle(fileHandle); 
    Logger_Info(__FILENAME__, "Done!");
  }
  
  {
    Logger_Info(__FILENAME__, "Extracting...");
    int progress[] = {0, 0};
    struct zip_t* zipFile = zip_open(tmpFilePath, 0, 'r');
    progress[1] = zip_entries_total(zipFile);
    zip_close(zipFile);
    
    zip_extract(tmpFilePath, installDir, onExtractFile, progress);
    
    Logger_Info(__FILENAME__, "Done!");
  }
  
  {
    int response = MessageBox(GetConsoleWindow(), "Would you like set PATH to be system wide? (answering no mean setting it to current user only)", "Setup", MB_YESNOCANCEL);
    const char* keyPath;
    if (response == IDCANCEL) {
      Logger_Info(__FILENAME__, "Exiting setup");
      return 0;
    } else if (response == IDYES) {
      Logger_Info(__FILENAME__, "Setting system wide PATH variable!");
      keyPath = "HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Environment";
    } else {
      Logger_Info(__FILENAME__, "Setting user only PATH variable!");
      keyPath = "HKEY_CURRENT_USER\\Environment";
    }
    
    char* helperScript;
    asprintf(&helperScript, "setlocal ENABLEEXTENSIONS\r\n"
            "set KEY_NAME=\"%s\"\r\n"
            "set VALUE_NAME=Path\r\n"
            "FOR /F \"usebackq tokens=2,* skip=2\" %%L IN ( `reg query %%KEY_NAME%% /v %%VALUE_NAME%%` ) DO SET \"ValueValue=%%M\"\r\n"
            "if defined ValueValue (\r\n"
            "reg add %%KEY_NAME%% /f /v %%VALUE_NAME%% /t REG_EXPAND_SZ /d \"%%ValueValue%%;%s\"\r\n"
            ") else (\r\n"
            "reg add %%KEY_NAME%% /f /v %%VALUE_NAME%% /t REG_EXPAND_SZ /d \"%s;\"\r\n"
            ")\r\n"
            "exit\r\n", keyPath, installDir, installDir);
    
    char tmpDirectory[MAX_PATH];
    GetTempPath(MAX_PATH, tmpDirectory);
    GetTempFileName(tmpDirectory, "LUA_SETUP_HELPER", 0, helperScriptPath);
    {
      char* message;
      asprintf(&message, "Preparing temporary files at %s", tmpFilePath);
      Logger_Info(__FILENAME__, message);
      free(message);
    }
    
    HANDLE fileHandle = CreateFile(helperScriptPath,
                                  FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                  0,
                                  NULL,
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
    
    if (fileHandle == INVALID_HANDLE_VALUE) {
      char* message;
      asprintf(&message, "Cannot open tmp file (%s) reason 0x%08X", helperScriptPath, GetLastError());
      Logger_Error(__FILENAME__, message);
      free(message);
      return 1;
    }
    
    for (size_t i = 0; i < strlen(helperScript); i++) {
      DWORD bytesWritten = 0;
      char singleByte = helperScript[i];
      BOOLEAN result = WriteFile(fileHandle, 
                                &singleByte, 
                                1, 
                                &bytesWritten, 
                                NULL);
      
      if (result == FALSE) {
        char* message;
        asprintf(&message, "Cannot write helper script file (%s) reason 0x%08X", helperScriptPath, GetLastError());
        Logger_Error(__FILENAME__, message);
        free(message);
        return 1;
      }
    }
    
    CloseHandle(fileHandle); 
    free(helperScript);
    
    
    Logger_Info(__FILENAME__, "Running helper script...");
    char* command;
    asprintf(&command, "cmd.exe < \"%s\"", helperScriptPath);
    Logger_Info(__FILENAME__, "Command:");
    Logger_Info(__FILENAME__, command);
    int result = system(command);
    if (result != 0) {
      char* msg;
      asprintf(&msg, "Exit code: %d", result);
      Logger_Error(__FILENAME__, "Cannot run helper script!");
      Logger_Error(__FILENAME__, msg);
      free(msg);
    }
    
    free(command);
    
    Logger_Info(__FILENAME__, "Done!");
  }
  
  {
    const char* message = "Author: Fox\r\nInstaller written in C\r\n"
                          "Installer for Lua " LUA_VERSION "\r\n"
                          "Libraries used:\r\n"
                          "\r\n"
                          "Zip: https://github.com/kuba--/zip/ (Author: kuba--)\r\n"
                          "Regex: https://github.com/kokke/tiny-regex-c (Author: kokke)\r\n"
                          "\r\n"
                          "Please relogin for installation to take affect\r\n"
                          "Enjoy your Lua " LUA_VERSION " installation!";
    Logger_Info(__FILENAME__, message);
    MessageBox(GetConsoleWindow(), message, "About", MB_OK | MB_ICONINFORMATION);
  }
  
	return 0;
}

int main() {
  actualMain();
  system("pause");
  return 0;
}