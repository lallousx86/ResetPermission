#pragma once

#define MAX_PATH2 (MAX_PATH * 2)
//-------------------------------------------------------------------------
class ResetPermissionDialog
{
    HINSTANCE hInstance;
    HWND hDlg;
    TCHAR AppName[MAX_PATH2];

    bool bRecurse;
    bool bResetPerm;
    bool bRmHidSys;
    bool bTakeOwn;
    bool bDontFollowLinks;

    bool BrowseFolder(
        HWND hOwner,
        LPCTSTR szCaption,
        stringT &folderpath);
    static void QuotePath(stringT &Path);

    static INT_PTR CALLBACK AboutDlgProc(
        HWND hAboutDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    void SetCommandWindowText(LPCTSTR Str);

    void UpdateCommandText();

    bool GetCommandWindowText(stringT &Cmd);

    LPCTSTR GenerateWorkBatchFileName();

    bool ExecuteCommand(stringT &Cmd);
    bool ExecuteWindowCommand(bool bValidatePath);

    void ShowPopupMenu(
        int IdMenu,
        int IdBtnPos);

    void AddToExplorerContextMenu(bool bAdd);
    void BackRestorePermissions(bool bBackup);

    void UpdateCheckboxes(bool bGet);

    LPCTSTR GetArgs();

    static INT_PTR CALLBACK s_MainDialogProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    INT_PTR CALLBACK MainDialogProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    bool BrowseFileName(
        bool bSave,
        LPCTSTR Caption,
        LPCTSTR Extension,
        LPCTSTR DefaultFile,
        stringT &out);

public:
    static INT_PTR ShowDialog(HINSTANCE hInst);

    bool GetFolderText(
        stringT &Folder,
        bool bWarnRoot,
        bool bAddWildCard,
        bool bQuoteIfNeeded);

    void SetFolderText(LPCTSTR Value);
    void InitCommand(stringT &cmd);
};
