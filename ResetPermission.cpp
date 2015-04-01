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
-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------
#include "stdafx.h"
#include "ResetPermission.h"

//-------------------------------------------------------------------------
#ifdef _UNICODE
#define stringT std::wstring
#else
#define stringT std::string
#endif

//-------------------------------------------------------------------------
static HINSTANCE g_hInstance;
static LPCTSTR STR_SELECT_FOLDER = TEXT("Please select a folder");
static LPCTSTR STR_ERROR = _TEXT("Error");
static LPCTSTR STR_RESET_FN = _TEXT("resetperm.bat");

//-------------------------------------------------------------------------
static bool GetFolder(
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
    HWND hDlg,
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
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------
static void UpdateCommandText(HWND hDlg)
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
    GetDlgItemText(hDlg, IDTXT_FOLDER, Path, _countof(Path));

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
        cmd += _TEXT("\r\n");
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
        cmd += _TEXT("\r\n");
    }

    cmd += _TEXT("pause\r\n");
    SetDlgItemText(hDlg, IDTXT_COMMAND, cmd.c_str());
}

//-------------------------------------------------------------------------
static void GetCommandWindowText(HWND hDlg, stringT &Cmd)
{
    HWND hwndCtrl = GetDlgItem(hDlg, IDTXT_COMMAND);
    int len = GetWindowTextLength(hwndCtrl);

    TCHAR *szCmd = new TCHAR[len + 1];

    GetWindowText(hwndCtrl, szCmd, len);

    Cmd = szCmd;
    delete[] szCmd;
}

//-------------------------------------------------------------------------
static bool ExecuteCommand(HWND hWndOwn)
{
    // Make temp file name
    TCHAR CmdFileName[MAX_PATH * 2];
    if (GetTempPath(_countof(CmdFileName), CmdFileName) == 0)
    {
        if (GetEnvironmentVariable(_TEXT("TEMP"), CmdFileName, _countof(CmdFileName)) == 0)
            return false;
    }

    if (CmdFileName[_tcslen(CmdFileName) - 1] != TCHAR('\\'))
        _tcsncat_s(CmdFileName, _TEXT("\\"), _countof(CmdFileName));

    _tcsncat_s(CmdFileName, STR_RESET_FN, _countof(CmdFileName));

    // Write temp file
    FILE *fp;
    if (_tfopen_s(&fp, CmdFileName, _TEXT("w")) != 0)
        return false;

	stringT Cmd;
	GetCommandWindowText(hWndOwn, Cmd);
    //SetDlgItemText(hDlg, IDTXT_COMMAND, cmd.c_str());

    _ftprintf(fp, _TEXT("%s\n"), Cmd.c_str());
    fclose(fp);

    // Execute the temp file
    return SUCCEEDED(
        ShellExecute(
        hWndOwn,
        _TEXT("open"),
        CmdFileName,
        NULL,
        NULL,
        SW_SHOW)
    );
}

//-------------------------------------------------------------------------
static INT_PTR CALLBACK MainDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
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

            HICON t = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SMALL));
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)t);
    #ifdef _DEBUG
            SetDlgItemText(hDlg, IDTXT_FOLDER, _TEXT("C:\\Temp\\perm"));
            UpdateCommandText(hDlg);
    #endif
            return (INT_PTR)TRUE;
        }

        case WM_COMMAND:
        {
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
                        UpdateCommandText(hDlg);
                        return TRUE;
                    }
                    break;
                }
                case IDBTN_ABOUT:
                {
                    DialogBox(
                        g_hInstance,
                        MAKEINTRESOURCE(IDD_ABOUTBOX),
                        hDlg,
                        AboutDlgProc);
                    return TRUE;
                }
                case IDBTN_CHOOSE_FOLDER:
                {
                    stringT out;
					if (GetFolder(hDlg, STR_SELECT_FOLDER, out))
					{
						SetDlgItemText(hDlg, IDTXT_FOLDER, out.c_str());
						UpdateCommandText(hDlg);
					}
                    return TRUE;
                }
                case IDOK:
                {
                    // Get the selected folder value
                    UINT Len = SendDlgItemMessage(hDlg, IDTXT_FOLDER, WM_GETTEXTLENGTH, 0, 0);
                    if (Len == 0)
                    {
                        MessageBox(hDlg, STR_SELECT_FOLDER, STR_ERROR, MB_OK | MB_ICONERROR);
                        return TRUE;
                    }
                    ExecuteCommand(hDlg);
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

//-------------------------------------------------------------------------
int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    g_hInstance = hInstance;
    DialogBox(
        hInstance,
        MAKEINTRESOURCE(IDD_DIALOG1),
        NULL,
        MainDialogProc);
}
