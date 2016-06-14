#pragma once

#include "stdafx.h"

//-------------------------------------------------------------------------
class ResetPermissionDialog
{
    static HINSTANCE hInstance;
    static HWND hDlg;
    static TCHAR AppName[MAX_PATH * 2];

    static bool bRecurse;
    static bool bResetPerm;
    static bool bRmHidSys;
    static bool bTakeOwn;
    static bool bDontFollowLinks;

    static bool BrowseFolder(
        HWND hOwner,
        LPCTSTR szCaption,
        stringT &folderpath);
    static void QuotePath(stringT &Path);

    static INT_PTR CALLBACK AboutDlgProc(
        HWND hAboutDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    static void SetCommandWindowText(LPCTSTR Str);

    static void UpdateCommandText();

    static bool GetCommandWindowText(stringT &Cmd);

    static LPCTSTR GenerateTempBatchFileName();

    static bool ExecuteCommand(bool bValidateFolder);

    static void ShowPopupMenu(
        int IdMenu,
        int IdBtnPos);

    static void AddToExplorerContextMenu(bool bAdd);
    static void BackRestorePermissions(bool bBackup);

    static void UpdateCheckboxes(bool bGet);

    static LPCTSTR GetArgs();

    static INT_PTR CALLBACK MainDialogProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    static bool BrowseFileName(
        bool bSave,
        LPCTSTR Caption,
        LPCTSTR Extension,
        LPCTSTR DefaultFile,
        stringT &out);

public:
    static INT_PTR ShowDialog(HINSTANCE hInst);

    static bool GetFolderText(
        stringT &Folder,
        bool bWarnRoot,
        bool bAddWildCard,
        bool bQuoteIfNeeded);

    static void SetFolderText(LPCTSTR Value);
    static void InitCommand(stringT &cmd);
};
