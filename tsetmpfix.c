//
//  Fix TEMP folder bug in Windows NT 4.0 Terminal Server Edition
//  Copyright (c) 1999 by Antoni Sawicki
//
#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"advapi32")
#pragma comment(lib,"user32")

VOID ERRPT(WCHAR *msg, ...) {
    va_list valist;
    WCHAR vaBuff[1024]={0};
    WCHAR errBuff[1024]={0};
    DWORD err;

    err=GetLastError();

    va_start(valist, msg);
    _vsnwprintf(vaBuff, sizeof(vaBuff), msg, valist);
    va_end(valist);

    wprintf(L"ERROR: %s\n", vaBuff);

    if(err) {
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errBuff, sizeof(errBuff) , NULL );
        wprintf(L"%08X : %s\n\n", err, errBuff);
    }    
    
    wprintf(L"\n\n");

    ExitProcess(1);
}


int wmain(int argc, WCHAR **argv) {
    HKEY EnvKey;
    DWORD attrs;
    WCHAR *UserProfilePath;
    WCHAR CurrentTempPath[MAX_PATH*sizeof(WCHAR)]={0};
    WCHAR ExpectedPath[MAX_PATH*sizeof(WCHAR)]={0};
    WCHAR NewTempPath[MAX_PATH*sizeof(WCHAR)]={0};
    int changed=0;
    LONG len;

    wprintf(L"TSE TMP FIX v1.0a - Copyright (c) 1999 by Antoni Sawicki\n");

    if(argc>1)
        ERRPT(L"\rUsage: place tsetmpfix in c:\\wtsrv\\system32 and add it to UsrLogon.cmd\n\n");

        UserProfilePath=_wgetenv(L"USERPROFILE");
    if(!UserProfilePath)
        ERRPT(L"Unable to read USERPROFILE environment variable!");

    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Environment", 0, KEY_SET_VALUE|KEY_QUERY_VALUE, &EnvKey))
        ERRPT(L"Unable to open HKEY_CURRENT_USER\\Environment!");
    
    if(RegQueryValueExW(EnvKey, L"TEMP", NULL, NULL, (BYTE *)&CurrentTempPath, &len))
        ERRPT(L"Unable to read current TEMP value from Environment Key");

    swprintf(ExpectedPath, L"%s\\TEMP", UserProfilePath);

    attrs=GetFileAttributesW(ExpectedPath);

    if(attrs!=0xFFFFFFFF && (attrs & FILE_ATTRIBUTE_DIRECTORY) && wcscmp(CurrentTempPath, L"%%USERPROFILE%%\\TEMP")) {

        wprintf(L"OK: The fix is active!\n");

    }
    else {
        
        wprintf(L"TEMP folder is outside USERPROFILE - applying fix!!\n");

        if(!CreateDirectoryW(ExpectedPath, NULL))
            ERRPT(L"Unable to create TEMP directory: %s", ExpectedPath);

        swprintf(NewTempPath, L"%%USERPROFILE%%\\TEMP\0");
    
        if(RegSetValueExW(EnvKey, L"TEMP", 0, REG_EXPAND_SZ, (BYTE *) NewTempPath, wcslen(NewTempPath)*sizeof(WCHAR) ))
            ERRPT(L"Unable to set value key!");
            
        if(RegSetValueExW(EnvKey, L"TMP", 0, REG_EXPAND_SZ, (BYTE *) NewTempPath, wcslen(NewTempPath)*sizeof(WCHAR) ))
            ERRPT(L"Unable to set value key!");
        
        RegCloseKey(EnvKey);

        wprintf(L"Done!\nThe fix will be active next time you log in.\nYou will be logged off now.\n");

        Sleep(5000);

        ExitWindows(0,0);
    }

    RegCloseKey(EnvKey);

    return 0;
}
