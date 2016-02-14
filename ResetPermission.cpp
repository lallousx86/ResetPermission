/*-------------------------------------------------------------------------
Reset files permission (c) Elias Bachaalany

lallousz-x86@yahoo.com


History
---------

08/24/2013 - Initial version
08/30/2013 - Enclose the folder with quotes if it contains at least one space character
09/17/2013 - Added "Reset files permission" as a optional action
           - Added "Reset hidden and system files"
03/31/2014 - Fixed double backslash when folder is root
01/08/2014 - Added "Do not follow symbolic links" option
03/31/2015 - Allow editing of the generated command textbox
           - Added "More actions" to add Explorer shell context menu
11/03/2015 - Added /SKIPSL switch to takeown.exe

11/15/2015 - v1.1.3
           - Added HELP button to redirect to blog entry
           - Added warning when attempting to change permission of a root folder

02/13/2016 - v1.1.4
           - Minor code changes
           - Update the console window title when the commands execute
-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------
#include "stdafx.h"

//-------------------------------------------------------------------------
static LPCTSTR STR_HELP_URL          = _TEXT("http://lallouslab.net/2013/08/26/resetting-ntfs-files-permission-in-windows-graphical-utility/");
static LPCTSTR STR_SELECT_FOLDER     = TEXT("Please select a folder");
static LPCTSTR STR_ERROR             = TEXT("Error");
static LPCTSTR STR_RESET_FN          = TEXT("resetperm.bat");
static LPCTSTR STR_HKCR_CTXMENU_BASE = TEXT("\"HKCR\\Folder\\shell\\Reset Permission");
static LPCTSTR STR_HKCR_CTXMENU_CMD  = TEXT("\\command");
static stringT STR_CMD_PAUSE         = TEXT("pause\r\n");
static stringT STR_NEWLINE           = TEXT("\r\n");
static stringT STR_NEWLINE2          = STR_NEWLINE + STR_NEWLINE;

static LPCTSTR STR_WARNING           = _TEXT("Warning!");
static LPCTSTR STR_ROOT_WARNING =
        _TEXT("You are about to change the permission of a root folder!\n")
        _TEXT("This is a **DANGEROUS** operation! It is better to choose a specific folder instead!\n\n")
        _TEXT("!! If you choose to proceed then you might render your system unstable !!\n\n")
        _TEXT("Are you sure you want to continue?");

//-------------------------------------------------------------------------
class ResetPermissionDialog
{
    static HINSTANCE hInstance;
    static HWND hDlg;
    static TCHAR AppName[MAX_PATH * 2];

    //-------------------------------------------------------------------------
    static bool BrowseFolder(
        HWND hOwner,
        LPCTSTR szCaption,
        stringT &folderpath)
    {
        BROWSEINFO bi;
        memset(&bi, 0, sizeof(bi));

        bi.ulFlags = BIF_EDITBOX | BIF_VALIDATE | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.hwndOwner = hOwner;
        bi.lpszTitle = szCaption;

        LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

        if (pIDL == NULL)
            return false;

        TCHAR buffer[_MAX_PATH] = { '\0' };
        bool bOk = ::SHGetPathFromIDList(pIDL, buffer) != 0;

        if (bOk)
            folderpath = buffer;

        // free the item id list
        CoTaskMemFree(pIDL);

        return bOk;
    }

    //-------------------------------------------------------------------------
    static INT_PTR CALLBACK AboutDlgProc(
        HWND hAboutDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        if (message == WM_INITDIALOG)
        {
            return TRUE;
        }
        else if (message == WM_COMMAND && LOWORD(wParam) == IDOK)
        {
            EndDialog(hAboutDlg, 0);
            return TRUE;
        }
        return FALSE;
    }

    //-------------------------------------------------------------------------
    static void SetCommandWindowText(LPCTSTR Str)
    {
        SetDlgItemText(hDlg, IDTXT_COMMAND, Str);
    }

    //-------------------------------------------------------------------------
    // Update the command text
    static void UpdateCommandText()
    {
        bool bRecurse = SendDlgItemMessage(
            hDlg,
            IDCHK_RECURSE,
            BM_GETCHECK,
            0,
            0) == BST_CHECKED;

        bool bResetPerm = SendDlgItemMessage(
            hDlg,
            IDCHK_RESETPERM,
            BM_GETCHECK,
            0,
            0) == BST_CHECKED;

        bool bRmHidSys = SendDlgItemMessage(
            hDlg,
            IDCHK_RM_HS,
            BM_GETCHECK,
            0,
            0) == BST_CHECKED;

        bool bTakeOwn = SendDlgItemMessage(
            hDlg,
            IDCHK_TAKEOWN,
            BM_GETCHECK,
            0,
            0) == BST_CHECKED;

        bool bDontFollowLinks = SendDlgItemMessage(
            hDlg,
            IDCHK_DONTFOLLOWLINKS,
            BM_GETCHECK,
            0,
            0) == BST_CHECKED;


        stringT folder;
        if (GetFolderText(folder) == 0)
            return;

        // Add the wildcard mask
        if (*folder.rbegin() != TCHAR('\\'))
            folder += _TEXT("\\");

        folder += _TEXT("*");

        // Quote the folder if needed
        if (folder.find(_T(' ')) != stringT::npos)
            folder = _TEXT("\"") + folder + _TEXT("\"");

        stringT cmd;
        LPCTSTR TempScript = GenerateTempBatchFileName();
        if (TempScript != nullptr)
        {
            cmd += _TEXT("REM Temp script location: ");
            cmd += TempScript + STR_NEWLINE2;
        }

        // Form takeown.exe command
        if (bTakeOwn)
        {
            // Update the command prompt's title
            cmd += _TEXT("TITLE taking ownership of folder: ") + folder + STR_NEWLINE;

            cmd += _TEXT("takeown");
            if (bRecurse)
                cmd += _TEXT(" /r ");

            if (bDontFollowLinks)
                cmd += _TEXT(" /SKIPSL ");

            cmd += _TEXT(" /f ") + folder + STR_NEWLINE2;
        }

        // Form icacls.exe command
        if (bResetPerm)
        {
            // Update the command prompt's title
            cmd += _TEXT("TITLE Taking ownership of folder: ") + folder + STR_NEWLINE;

            cmd += _TEXT("icacls ") + folder;
            if (bRecurse)
                cmd += _TEXT(" /T ");
            if (bDontFollowLinks)
                cmd += _TEXT(" /L ");

            cmd += _TEXT(" /Q /C /RESET") + STR_NEWLINE2;
        }

        // Form attribute.exe command
        if (bRmHidSys)
        {
            // Update the command prompt's title
            cmd += _TEXT("TITLE Changing files attributes in folder: ") + folder + STR_NEWLINE;

            cmd += _TEXT("attrib");
            if (bRecurse)
                cmd += _TEXT(" /s ");

            cmd += _TEXT(" -h -s ") + folder + STR_NEWLINE2;
        }

        // Always add a pause
        cmd += STR_CMD_PAUSE;

        // Update the 
        SetCommandWindowText(cmd.c_str());
    }

    //-------------------------------------------------------------------------
    static bool GetCommandWindowText(stringT &Cmd)
    {
        HWND hwndCtrl = GetDlgItem(hDlg, IDTXT_COMMAND);
        if (hwndCtrl == NULL)
            return false;

        int len = GetWindowTextLength(hwndCtrl);
        if (GetLastError() != NO_ERROR)
            return false;

        TCHAR *szCmd = new TCHAR[len + 1];
        if (szCmd == NULL)
            return false;

        GetWindowText(hwndCtrl, szCmd, len);
        bool bOk = GetLastError() == NO_ERROR;

        Cmd = szCmd;
        delete [] szCmd;

        return bOk;
    }

    //-------------------------------------------------------------------------
    static LPCTSTR GenerateTempBatchFileName()
    {
        // Make temp file name
        static TCHAR CmdFileName[MAX_PATH * 2] = { 0 };

        // Compute if it was not already computed
        if (CmdFileName[0] == _TCHAR('\0'))
        {
            if (GetTempPath(_countof(CmdFileName), CmdFileName) == 0)
            {
                if (GetEnvironmentVariable(_TEXT("TEMP"), CmdFileName, _countof(CmdFileName)) == 0)
                    return nullptr;
            }

            if (CmdFileName[_tcslen(CmdFileName) - 1] != TCHAR('\\'))
                _tcsncat_s(CmdFileName, _TEXT("\\"), _countof(CmdFileName));

            _tcsncat_s(CmdFileName, STR_RESET_FN, _countof(CmdFileName));
        }

        return CmdFileName;
    }

    //-------------------------------------------------------------------------
    // Execute the command typed in the command textbox
    static bool ExecuteCommand()
    {
        // Warn if this is a root folder
        stringT Path;
        if (    GetFolderText(Path)
             && Path.length() == 3 && Path[1] == _TCHAR(':') && Path[2] == _TCHAR('\\'))
        {
            if (MessageBox(hDlg, STR_ROOT_WARNING, STR_WARNING, MB_YESNO | MB_ICONWARNING) == IDNO)
                return false;
        }

        // Delete previous batch file
        LPCTSTR CmdFileName = GenerateTempBatchFileName();
        DeleteFile(CmdFileName);

        // Write temp file
        FILE *fp;
        if (_tfopen_s(&fp, CmdFileName, _TEXT("w")) != 0)
        {
            stringT err_msg = TEXT("Failed to write batch file to: ");
            err_msg += CmdFileName;

            MessageBox(
                hDlg, 
                err_msg.c_str(), 
                TEXT("Error"), 
                MB_OK | MB_ICONERROR);
            return false;
        }

        stringT Cmd;
        if (!GetCommandWindowText(Cmd))
        {
            MessageBox(hDlg, TEXT("Failed to get command text"), TEXT("Error"), MB_OK | MB_ICONERROR);
            return false;
        }

        _ftprintf(fp, _TEXT("%s\n"), Cmd.c_str());
        fclose(fp);

        // Execute the temp batch file
        return SUCCEEDED(
            ShellExecute(
            hDlg,
            _TEXT("open"),
            CmdFileName,
            NULL,
            NULL,
            SW_SHOW));
    }

    //-------------------------------------------------------------------------
    static void ShowMoreActionsMenu()
    {
        HMENU hMoreActionsMenu = LoadMenu(
            hInstance,
            MAKEINTRESOURCE(IDR_MOREACTIONS_MENU));

        HWND hBtn = GetDlgItem(hDlg, IDBTN_MORE_ACTIONS);
        HMENU hSubMenu = GetSubMenu(hMoreActionsMenu, 0);
        RECT rect;
        GetClientRect(hBtn, &rect);

        POINT pt = { rect.left, rect.top };
        ClientToScreen(hBtn, &pt);

        TrackPopupMenu(
            hSubMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON,
            pt.x,
            pt.y,
            0,
            hDlg, NULL);

        DestroyMenu(hMoreActionsMenu);
    }

    //-------------------------------------------------------------------------
    static void AddToExplorerContextMenu(bool bAdd)
    {
        stringT cmd = TEXT("reg ");

        if (bAdd)
            cmd += TEXT("ADD ");
        else
            cmd += TEXT("DELETE ");

        cmd += STR_HKCR_CTXMENU_BASE;

        if (bAdd)
            cmd += STR_HKCR_CTXMENU_CMD;

        cmd += TEXT("\" /f ");

        if (bAdd)
        {
            cmd += TEXT("/ve /t REG_SZ /d \"\\\"");

            cmd += AppName;

            cmd += TEXT("\\\" %%1\"");
        }

        cmd += STR_NEWLINE;

        cmd += STR_CMD_PAUSE;
        SetCommandWindowText(cmd.c_str());
    }

    //-------------------------------------------------------------------------
    static LPCTSTR GetArgs()
    {
        LPCTSTR szCmdLine = GetCommandLine();
        bool bQuoted = szCmdLine[0] == _TCHAR('"');
        if (bQuoted)
            ++szCmdLine;

        // Skip application name
        szCmdLine += _tcslen(AppName);
        if (bQuoted)
            ++szCmdLine;

        ++szCmdLine;
        if (_tcslen(szCmdLine) == 0)
            return NULL;
        else
            return szCmdLine;
    }

    //-------------------------------------------------------------------------
    static INT_PTR CALLBACK MainDialogProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                ResetPermissionDialog::hDlg = hWnd;

                SendDlgItemMessage(
                    hDlg,
                    IDCHK_RESETPERM,
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

                SendDlgItemMessage(
                    hDlg,
                    IDCHK_DONTFOLLOWLINKS,
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

                SendDlgItemMessage(
                    hDlg,
                    IDCHK_RECURSE,
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

                HICON t = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
                SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)t);

                LPCTSTR Arg = GetArgs();

                if (Arg != NULL)
                    SetFolderText(Arg);
    #ifdef _DEBUG
                else
                    SetFolderText(_TEXT("C:\\Temp\\perm"));
    #endif
                if (Arg != NULL)
                    UpdateCommandText();

                return TRUE;
            }

            case WM_MENUCOMMAND:
                break;

            case WM_COMMAND:
            {
    #ifdef _DEBUG
                TCHAR b[1024];
                _sntprintf_s(
                    b, 
                    _countof(b), 
                    _TEXT("WM_COMMAND: wmParam=%08X lParam=%08X\n"), 
                    wParam, 
                    lParam);
                OutputDebugString(b);
    #endif
                UINT wmId    = LOWORD(wParam);
                UINT wmEvent = HIWORD(wParam);

                switch (wmId)
                {
                    //
                    // Handle checkboxes
                    //
                    case IDCHK_RECURSE:
                    case IDCHK_DONTFOLLOWLINKS:
                    case IDCHK_TAKEOWN:
                    case IDCHK_RESETPERM:
                    case IDCHK_RM_HS:
                    {
                        if (wmEvent == BN_CLICKED)
                        {
                            UpdateCommandText();
                            return TRUE;
                        }
                        break;
                    }
                    //
                    // Handle context menu
                    //
                    case IDM_ADDTOEXPLORERFOLDERRIGHT:
                    case IDM_REMOVEFROMEXPLORERFOLDERCONTEXTMENU:
                    {
                        AddToExplorerContextMenu(wmId == IDM_ADDTOEXPLORERFOLDERRIGHT);
                        break;
                    }
                    //
                    // About box
                    //
                    case IDBTN_ABOUT:
                    {
                        DialogBox(
                            hInstance,
                            MAKEINTRESOURCE(IDD_ABOUTBOX),
                            hDlg,
                            AboutDlgProc);

                        return TRUE;
                    }
                    //
                    // Choose folder
                    //
                    case IDBTN_CHOOSE_FOLDER:
                    {
                        stringT out;
                        if (BrowseFolder(hDlg, STR_SELECT_FOLDER, out))
                        {
                            SetFolderText(out.c_str());
                            UpdateCommandText();
                        }
                        return TRUE;
                    }
                    //
                    // Trigger the actions menu
                    //
                    case IDBTN_MORE_ACTIONS:
                    {
                        ShowMoreActionsMenu();
                        return TRUE;
                    }
                    //
                    // GO button
                    //
                    case IDOK:
                    {
                        ExecuteCommand();
                        return TRUE;
                    }
                    // HELP button
                    case IDBTN_HELP:
                    {
                        ShellExecute(
                            hDlg,
                            _TEXT("open"),
                            STR_HELP_URL,
                            nullptr,
                            nullptr,
                            SW_SHOW);

                        return TRUE;
                    }
                }
                break;
            }
            // Close dialog
            case WM_CLOSE:
            {
                EndDialog(hDlg, IDOK);
                return TRUE;
            }
        }
        return FALSE;
    }
public:
    static INT_PTR ShowDialog(HINSTANCE hInst)
    {
        GetModuleFileName(NULL, AppName, _countof(AppName));

        hInstance = hInst;
        return DialogBox(
            hInstance,
            MAKEINTRESOURCE(IDD_DIALOG1),
            NULL,
            MainDialogProc);
    }

    static bool GetFolderText(stringT &Folder)
    {
        TCHAR Path[MAX_PATH * 4];
        UINT len = GetDlgItemText(
            hDlg,
            IDTXT_FOLDER,
            Path,
            _countof(Path));
        if (len == 0)
        {
            return false;
        }
        else
        {
            Folder = Path;
            return true;
        }
    }

    static void SetFolderText(LPCTSTR Value)
    {
        SetDlgItemText(hDlg, IDTXT_FOLDER, Value);
    }
};

//-------------------------------------------------------------------------
// Class variables
TCHAR ResetPermissionDialog::AppName[MAX_PATH * 2];
HINSTANCE ResetPermissionDialog::hInstance;
HWND ResetPermissionDialog::hDlg = NULL;

//-------------------------------------------------------------------------
int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    ::OleInitialize(NULL);
    return (int)ResetPermissionDialog::ShowDialog(hInstance);
}
