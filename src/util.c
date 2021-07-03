#include <windows.h>
#include <shlobj.h> 
#include <minwindef.h>
#include "util.h"
#include "logger/Logger.h"
#include "asprintf.h"

int FAR PASCAL Util_BrowseCallback(HWND hwnd, UINT uMsg,LPARAM lp, LPARAM pData) {
  if (uMsg == BFFM_INITIALIZED)  {
    SendMessage(hwnd, BFFM_SETSELECTION, 1, pData);
    return 1;
  }
  return 0;
}

BOOLEAN Util_GetFolderSelection(char* folderPath, const char* message, const char* initialPath) {
  BROWSEINFO bi;
  ZeroMemory(&bi, sizeof(bi));
  bi.hwndOwner = GetConsoleWindow();
  bi.lpszTitle = message;
  bi.lParam = (LPARAM) initialPath;
  bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_BROWSEFORCOMPUTER | 
               BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_RETURNFSANCESTORS;
  bi.lpfn = (BFFCALLBACK) Util_BrowseCallback;
  
  PIDLIST_ABSOLUTE list = SHBrowseForFolder(&bi);
  
  if (list == NULL) {return FALSE;}
  
  SHGetPathFromIDList(list, folderPath);
  return TRUE;
}

BOOLEAN Util_GetFileSelection(char* fileSelected, const char* initialDir) {
  OPENFILENAME openFileNameStruct;
  ZeroMemory(&openFileNameStruct, sizeof(OPENFILENAME));
  
  openFileNameStruct.lStructSize = sizeof(openFileNameStruct);
  openFileNameStruct.hwndOwner = GetConsoleWindow();
  openFileNameStruct.hInstance = NULL;
  openFileNameStruct.lpstrFilter = "All Files (*.*)\0*.*\0";
  openFileNameStruct.lpstrCustomFilter = NULL;
  openFileNameStruct.nMaxCustFilter = 0;
  openFileNameStruct.nFilterIndex = 0;
  openFileNameStruct.lpstrFile = fileSelected;
  openFileNameStruct.lpstrFile[0] = '\0';
  openFileNameStruct.nMaxFile = MAX_PATH;
  openFileNameStruct.lpstrInitialDir = initialDir;
  openFileNameStruct.lpstrFileTitle = NULL;
  openFileNameStruct.nMaxFileTitle = MAX_PATH;
  openFileNameStruct.lpstrInitialDir = NULL;
  openFileNameStruct.lpstrTitle = NULL;
  openFileNameStruct.Flags = 0;
  openFileNameStruct.nFileOffset = 0;
  openFileNameStruct.nFileExtension = 0;
  openFileNameStruct.lpstrDefExt = NULL;
  openFileNameStruct.lCustData = 0L;
  openFileNameStruct.lpfnHook = NULL;
  openFileNameStruct.lpTemplateName = NULL;
  openFileNameStruct.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
  
  BOOLEAN isSuccess = GetOpenFileName(&openFileNameStruct);
  int exitCode = CommDlgExtendedError();
  
  if (!isSuccess && exitCode > 0) {
    Logger_Error(__FILENAME__, "Cant open 'open' dialog");
    {
      char* message;
      asprintf(&message, "Reason 0x%04X", (int) CommDlgExtendedError());
      Logger_Error(__FILENAME__, message);
      free(message);
    }
    return FALSE;
  } else {
    if (exitCode == 0) {
      return FALSE;
    } else {
      return FALSE;
    }
  }
}