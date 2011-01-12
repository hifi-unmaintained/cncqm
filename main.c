/*
 * Copyright (c) 2011 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <time.h>

#define TITLE "CnC Quick Match"

/* mingw32 does not have this function in it's wincrypt lib, using one from Win2003 SDK */
#if 0
    #include <wincrypt.h>
#else
    WINBOOL WINAPI CryptStringToBinaryA(LPCSTR pszString,DWORD cchString,DWORD dwFlags,BYTE *pbBinary,DWORD *pcbBinary,DWORD *pdwSkip,DWORD *pdwFlags);
    #define CRYPT_STRING_BASE64 0x1
#endif

DWORD dde = 0;

HSZ strWCHAT;
HSZ strCONQUER;
HSZ strTOPIC;

char *poke = NULL;

BOOL game_started = FALSE;
time_t cncqm_started = 0;

void cncqm_exit()
{
    if (dde)
    {
        DdeNameService(dde, strWCHAT, 0L, DNS_UNREGISTER);
        DdeUninitialize(dde);
    }

    if (poke)
    {
        HeapFree(GetProcessHeap(), 0, poke);
    }

    exit(0);
}

void cncqm_start()
{
    HCONV conv = DdeConnect(dde, strCONQUER, strTOPIC, NULL);

    if(conv)
    {
        printf("DDE: Connected, conversation id %p\n", conv);
        printf("DDE: Sending %d bytes (start game)\n", strlen(poke+8)+8);

        DdeClientTransaction((LPBYTE)poke, strlen(poke+8)+8, conv, strTOPIC, CF_TEXT, XTYP_POKE, 60000, NULL);

        FILE *fh = fopen("test.dmp", "wb");
        fwrite(poke, strlen(poke+8)+8, 1, fh);
        fclose(fh);

        printf("DDE: Disconnecting\n");
        DdeDisconnect(conv);

        game_started = TRUE;
    }
    else
    {
        printf("DDE: Failed to connect, shutting down\n");
        cncqm_exit();
    }
}

HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdata, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
    char buf[512];

    if (uType == XTYP_POKE)
    {
#if 0
        DWORD size;
        time_t t = time(NULL);

        DdeQueryString(dde, hsz1, buf, 512, CP_WINANSI);

        if (strcmp(buf, "POKE TOPIC"))
        {
            return (HDDEDATA)DDE_FNOTPROCESSED;
        }

        LPBYTE ptr = DdeAccessData(hdata, &size);

        /* cancel connect, the packet is "Hello", lol */
        if (size == 13)
        {
            printf("DDE: CONQUER told us the connection failed\n");
            DdeUnaccessData(hdata);
            return (HDDEDATA)DDE_FACK;
        }

        strftime(buf, 512, "results-%Y-%m-%d_%H_%M_%S.dmp", localtime(&t));

        printf("DDE: CONQUER sending results (%d bytes), writing to %s\n", (int)size, buf);

        FILE *fh = fopen(buf, "wb");
        if (fh)
        {
            fwrite(ptr, size, 1, fh);
            fclose(fh);
        }

        DdeUnaccessData(hdata);
#endif

        return (HDDEDATA)DDE_FACK;
    }

    else if (uType == XTYP_CONNECT)
    {
        DdeQueryString(dde, hsz1, buf, 512, CP_WINANSI);

        if (strcmp(buf, "POKE TOPIC"))
        {
            return (HDDEDATA)FALSE;
        }

        DdeQueryString(dde, hsz2, buf, 512, CP_WINANSI);

        if (strcmp(buf, "WCHAT") != 0)
        {
            return (HDDEDATA)FALSE;
        }

        printf("DDE: CONQUER connected to us\n");
        return (HDDEDATA)TRUE;
    }

    else if (uType == XTYP_REGISTER)
    {
        DdeQueryString(dde, hsz1, buf, 512, CP_WINANSI);

        if (strcmp(buf, "CONQUER") == 0)
        {
            printf("DDE: CONQUER registered, connecting...\n");

            /* wait for a while, seems the game is not ready to accept connections immediately */
            Sleep(1000);

            cncqm_start();
        }
    }

    return (HDDEDATA)NULL;
}

HANDLE hProcess = NULL;
HANDLE hThread = NULL;

// replacement for dirname() POSIX function (also keeps internal copy of the path)
char *GetDirectory(const char *path)
{
    static char buf[MAX_PATH];
    char *ptr;
    strncpy(buf, path, MAX_PATH);
    ptr = strrchr(buf, '\\');
    if(ptr)
    {
        *(ptr+1) = 0;
        return buf;
    }

    return NULL;
}

char *GetFile(const char *path)
{
    static char buf[MAX_PATH];
    char *ptr;
    strncpy(buf, path, MAX_PATH);
    ptr = strrchr(buf, '\\');
    if(ptr)
    {
        return (ptr+1);
    }

    return buf;
}

void cncqm_watchdog()
{
    while(1)
    {
        if (game_started)
        {
            if (FindWindow(NULL, "Command & Conquer") == NULL)
            {
                printf("C&C closed, going along\n");
                break;
            }
        }
        else
        {
            if (time(NULL) > cncqm_started + 10)
            {
                printf("C&C did not start, giving up\n");
                break;
            }
        }
        Sleep(1000);
    }
    cncqm_exit();
}

int main(int argc, char **argv)
{
    cncqm_started = time(NULL);

    DdeInitialize(&dde, DdeCallback, APPCLASS_STANDARD|CBF_FAIL_SELFCONNECTIONS|CBF_SKIP_CONNECT_CONFIRMS, 0);

    strWCHAT = DdeCreateStringHandle(dde, "WCHAT", CP_WINANSI);
    strCONQUER = DdeCreateStringHandle(dde, "CONQUER", CP_WINANSI);
    strTOPIC = DdeCreateStringHandle(dde, "POKE TOPIC", CP_WINANSI);

    if (argc < 2)
    {
#if 0
[HKEY_CLASSES_ROOT\cncqm]
@="URL:Command & Conquer Quick Match Protocol"
"URL Protocol"=""

[HKEY_CLASSES_ROOT\cncqm\shell]

[HKEY_CLASSES_ROOT\cncqm\shell\open]

[HKEY_CLASSES_ROOT\cncqm\shell\open\command]
@="\"Z:\\home\\hifi\\work\\cncqm\\cncqm.exe\"" 
#endif
        HKEY hKey;

        if (RegOpenKey(HKEY_CLASSES_ROOT, "cncqm", &hKey) == ERROR_SUCCESS)
        {
            if (MessageBoxA(NULL, "Do you want to unregister CnCQM?", TITLE, MB_YESNO|MB_ICONQUESTION) == IDYES)
            {
                RegDeleteKey(HKEY_CLASSES_ROOT, "cncqm\\shell\\open\\command");
                RegDeleteKey(HKEY_CLASSES_ROOT, "cncqm\\shell\\open");
                RegDeleteKey(HKEY_CLASSES_ROOT, "cncqm\\shell");
                if (RegDeleteKey(HKEY_CLASSES_ROOT, "cncqm") == ERROR_SUCCESS)
                {
                    MessageBox(NULL, "CnCQM unregistered!", TITLE, MB_OK|MB_ICONINFORMATION);
                }
                else
                {
                    MessageBoxA(NULL, "Error unregistering CnCQM, are you an Administrator?", TITLE, MB_OK|MB_ICONERROR);
                }
            }
        }

        else if (MessageBoxA(NULL, "Do you want to register CnCQM?", TITLE, MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            if (RegCreateKey(HKEY_CLASSES_ROOT, "cncqm\\shell\\open\\command", &hKey) == ERROR_SUCCESS)
            {
                char buf[MAX_PATH];
                GetModuleFileName(NULL, buf+1, MAX_PATH);
                buf[0] = '"';
                strcat(buf, "\" \"%1\"");
                printf("cncqm at %s\n", buf);
                RegSetValue(HKEY_CLASSES_ROOT, "cncqm", REG_SZ, "URL:Command & Conquer Quick Match Protocol", 0);

                RegOpenKey(HKEY_CLASSES_ROOT, "cncqm", &hKey);
                RegSetValueEx(hKey, "URL Protocol", 0, REG_SZ, (const BYTE *)"", 1);

                if (RegSetValue(HKEY_CLASSES_ROOT, "cncqm\\shell\\open\\command", REG_SZ, buf, 0) == ERROR_SUCCESS)
                {
                    MessageBox(NULL, "CnCQM successfully registered!", TITLE, MB_OK|MB_ICONINFORMATION);
                }
                else
                {
                    MessageBoxA(NULL, "Error registering CnCQM, are you an Administrator?", TITLE, MB_OK|MB_ICONERROR);
                }
            }
        }
        return 0;
    }

    if (strlen(argv[1]) < 32)
    {
        MessageBoxA(NULL, "Payload too short, expected more", TITLE, MB_OK|MB_ICONERROR);
        return 1;
    }

    DWORD poke_len = strlen(argv[1]);
    poke = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, poke_len);

    if (CryptStringToBinaryA(argv[1]+8, 0, CRYPT_STRING_BASE64, (LPBYTE)(poke+8), &poke_len, NULL, NULL))
    {
        /* validate configuration to be ASCII */
        int i;
        for (i=8;i<poke_len+8;i++)
        {
            if ( (poke[i] < 32 || poke[i] > 126) && poke[i] != 0x0D && poke[i] != 0x0A)
            {
                MessageBoxA(NULL, "Invalid characters in config", TITLE, MB_OK|MB_ICONERROR);
                HeapFree(GetProcessHeap(), 0, poke);
                return 1;
            }
        }

        /* write the header */
        DWORD len = htonl(strlen(poke+8)+8);
        memcpy(poke, &len, 4);
        memset(poke+4, 0, 4);

        //MessageBox(NULL, poke+8, TITLE, MB_OK|MB_ICONINFORMATION);

        DdeNameService(dde, strWCHAT, 0L, DNS_REGISTER);

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cncqm_watchdog, NULL, 0, NULL);

        printf("DDE: Waiting for CONQUER...\n");

        if (FindWindow(NULL, "Command & Conquer"))
        {
            printf("Found C&C window, starting game...\n");
            cncqm_start();
        }
        else
        {
            printf("No game window, launching it\n");
            HKEY hKey;
            char tmp[MAX_PATH];
            DWORD dwWritten;
            if (RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Westwood\\Command & Conquer Windows 95 Edition", &hKey) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hKey, "InstallPath", NULL, NULL, (unsigned char*)tmp, &dwWritten) == ERROR_SUCCESS)
                {
                    SetCurrentDirectoryA(GetDirectory(tmp));
                    PROCESS_INFORMATION pInfo;
                    STARTUPINFOA sInfo;

                    ZeroMemory(&sInfo, sizeof(STARTUPINFO));
                    sInfo.cb = sizeof(sInfo);
                    ZeroMemory(&pInfo, sizeof(PROCESS_INFORMATION));

                    if (CreateProcess(NULL, tmp, NULL, NULL, TRUE, 0, NULL, NULL, &sInfo, &pInfo) == 0)
                    {
                        MessageBoxA(NULL, "Failed to launch C&C", TITLE, MB_OK|MB_ICONERROR);
                        cncqm_exit();
                    }
                    else
                    {
                        printf("Game launched from %s\n", tmp);
                    }
                }
                else
                {
                    MessageBoxA(NULL, "Could not find C&C install path value", TITLE, MB_OK|MB_ICONERROR);
                    cncqm_exit();
                }
            }
            else
            {
                MessageBoxA(NULL, "Could not find C&C install path", TITLE, MB_OK|MB_ICONERROR);
                cncqm_exit();
            }
        }

        MSG msg;
        while(GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    }
    else
    {
        HeapFree(GetProcessHeap(), 0, poke);
        MessageBoxA(NULL, "Error decoding payload", TITLE, MB_OK|MB_ICONERROR);
        return 1;
    }

    return 0;
}
