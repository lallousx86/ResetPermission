// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string>

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// C RunTime Header Files
#pragma warning(push)
#pragma warning(disable: 4091)
#include <ShlObj.h>
#pragma warning(pop)

#include <shellapi.h>
#include <CommDlg.h>
#include <tchar.h>

#include "resource.h"
//#include "ResetPermission.h"

//-------------------------------------------------------------------------
#ifdef _UNICODE
    #define stringT std::wstring
#else
    #define stringT std::string
#endif
