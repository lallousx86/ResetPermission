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
-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------
#include "stdafx.h"

//-------------------------------------------------------------------------
static LPCTSTR STR_SELECT_FOLDER     = TEXT("Please select a folder");
static LPCTSTR STR_ERROR             = TEXT("Error");
static LPCTSTR STR_RESET_FN          = TEXT("resetperm.bat");
static LPCTSTR STR_HKCR_CTXMENU_BASE = TEXT("\"HKCR\\Folder\\shell\\Reset Permission");
static LPCTSTR STR_HKCR_CTXMENU_CMD  = TEXT("\\command");
static LPCTSTR STR_CMD_PAUSE         = TEXT("pause\r\n");
static LPCTSTR STR_NEWLINE           = TEXT("\r\n");

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

        bi.ulFlags = BIF_EDITBOX | BIF_VALIDATE | BIF_RETURNONLYFSDIRS;
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


        TCHAR Path[MAX_PATH * 4];
        UINT len = GetDlgItemText(hDlg, IDTXT_FOLDER, Path, _countof(Path));
        if (len == 0)
            return;

        stringT folder = Path;

        // Add the wildcard mask
        if (*folder.rbegin() != TCHAR('\\'))
            folder += _TEXT("\\");

        folder += _TEXT("*");

        // Quote the folder if needed
        if (folder.find(_T(' ')) != stringT::npos)
        {
            folder = _TEXT("\"") + folder;
            folder += _TEXT("\"");
        }

        stringT cmd;

        if (bTakeOwn)
        {
            cmd += _TEXT("takeown");
            if (bRecurse)
                cmd += _TEXT(" /r ");

            cmd += _TEXT(" /f ");
            cmd += folder;
            cmd += STR_NEWLINE;
        }

        if (bResetPerm)
        {
            cmd += _TEXT("icacls ");
            cmd += folder;
            if (bRecurse)
                cmd += _TEXT(" /T ");
            if (bDontFollowLinks)
                cmd += _TEXT(" /L ");

            cmd += _TEXT(" /Q /C /RESET\r\n");
        }

        if (bRmHidSys)
        {
            cmd += _TEXT("attrib");
            if (bRecurse)
                cmd += _TEXT(" /s ");

            cmd += _TEXT(" -h -s ");
            cmd += folder;
            cmd += STR_NEWLINE;
        }

        cmd += STR_CMD_PAUSE;

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
        delete[] szCmd;

        return bOk;
    }

    //-------------------------------------------------------------------------
    static LPCTSTR GenerateTempBatchFileName()
    {
        // Make temp file name
        static TCHAR CmdFileName[MAX_PATH * 2];
        if (GetTempPath(_countof(CmdFileName), CmdFileName) == 0)
        {
            if (GetEnvironmentVariable(_TEXT("TEMP"), CmdFileName, _countof(CmdFileName)) == 0)
                return false;
        }

        if (CmdFileName[_tcslen(CmdFileName) - 1] != TCHAR('\\'))
            _tcsncat_s(CmdFileName, _TEXT("\\"), _countof(CmdFileName));

        _tcsncat_s(CmdFileName, STR_RESET_FN, _countof(CmdFileName));

        return CmdFileName;
    }

    //-------------------------------------------------------------------------
    // Execute the command typed in the command textbox
    static bool ExecuteCommand()
    {
        LPCTSTR CmdFileName = GenerateTempBatchFileName();
        DeleteFile(CmdFileName);

        // Write temp file
        FILE *fp;
        if (_tfopen_s(&fp, CmdFileName, _TEXT("w")) != 0)
        {
            MessageBox(hDlg, TEXT("Failed to write batch file"), TEXT("Error"), MB_OK | MB_ICONERROR);
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
                //else
                //    SetFolderText(_TEXT("C:\\Temp\\perm"));
    #endif
                if (Arg != NULL)
                    UpdateCommandText();
                return (INT_PTR)TRUE;
            }

            case WM_MENUCOMMAND:
                break;
            case WM_COMMAND:
            {
    #ifdef _DEBUG
                TCHAR b[1024];
                _sntprintf_s(b, _countof(b), _TEXT("WM_COMMAND: wmParam=%08X lParam=%08X\n"), wParam, lParam);
                OutputDebugString(b);
    #endif
                UINT wmId = LOWORD(wParam);
                UINT wmEvent = HIWORD(wParam);

                switch (wmId)
                {
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
                    case IDM_ADDTOEXPLORERFOLDERRIGHT:
                    case IDM_REMOVEFROMEXPLORERFOLDERCONTEXTMENU:
                    {
                        AddToExplorerContextMenu(wmId == IDM_ADDTOEXPLORERFOLDERRIGHT);
                        break;
                    }
                    case IDBTN_ABOUT:
                    {
                        DialogBox(
                            hInstance,
                            MAKEINTRESOURCE(IDD_ABOUTBOX),
                            hDlg,
                            AboutDlgProc);

                        return TRUE;
                    }
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
                    case IDBTN_MORE_ACTIONS:
                    {
                        ShowMoreActionsMenu();
                        return TRUE;
                    }
                    case IDOK:
                    {
                        ExecuteCommand();
                        return TRUE;
                    }
                }
                break;
            }
            // Close dialog
            case WM_CLOSE:
            {
                EndDialog(hDlg, 0);
                return TRUE;
            }
        }
        return FALSE;
    }
public:
    static void ShowDialog(HINSTANCE hInst)
    {
        GetModuleFileName(NULL, AppName, _countof(AppName));

        hInstance = hInst;
        DialogBox(
            hInstance,
            MAKEINTRESOURCE(IDD_DIALOG1),
            NULL,
            MainDialogProc);
    }

    static void SetFolderText(LPCTSTR Value)
    {
        SetDlgItemText(hDlg, IDTXT_FOLDER, Value);
    }
};

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
    ResetPermissionDialog::ShowDialog(hInstance);
}
