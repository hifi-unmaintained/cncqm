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

#define PORT 8054

#define START_GAME \
            "[Internet]\r\n" \
            "Address=%s\r\n" \
            "Host=%d\r\n" \
            "Port=%d\r\n" \
            "GameID=1234\r\n" \
            "StartTime=5678\r\n" \
            "HWND=0\r\n" \
            "[Options]\r\n" \
            "Handle=%s\r\n" \
            "Color=%d\r\n" \
            "Side=%d\r\n" \
            "Credits=3000\r\n" \
            "Bases=1\r\n" \
            "Tiberium=0\r\n" \
            "Crates=0\r\n" \
            "AI=0\r\n" \
            "CaptureTheFlag=0\r\n" \
            "BuildLevel=3\r\n" \
            "UnitCount=6\r\n" \
            "Seed=555\r\n" \
            "Scenario=1\r\n" \
            "[Timing]\r\n" \
            "MaxAhead=15\r\n" \
            "SendRate=5\r\n"

DWORD SwapFourBytes(DWORD dw)
{
    register DWORD tmp;
    tmp =  (dw & 0x000000FF);
    tmp = ((dw & 0x0000FF00) >> 0x08) | (tmp << 0x08);
    tmp = ((dw & 0x00FF0000) >> 0x10) | (tmp << 0x08);
    tmp = ((dw & 0xFF000000) >> 0x18) | (tmp << 0x08);
    return(tmp);
}

DWORD dde = 0;

HSZ strWCHAT;
HSZ strCONQUER;
HSZ strTOPIC;

char poke[512];

HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdata, ULONG_PTR dwData1, ULONG_PTR dwData2)
{
    DWORD size;
    char buf[512];

    if (uType == XTYP_POKE)
    {
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

            HCONV conv = DdeConnect(dde, strCONQUER, strTOPIC, NULL);

            if(conv)
            {
                printf("DDE: Connected, conversation id %p\n", conv);
                printf("DDE: Sending %d bytes (start game)\n", strlen(poke+8));

                DdeClientTransaction((LPBYTE)poke, strlen(poke+8), conv, strTOPIC, CF_TEXT, XTYP_POKE, 60000, NULL);

                printf("DDE: Disconnecting\n");
                DdeDisconnect(conv);
            }
            else
            {
                printf("DDE: Failed to connect, shutting down\n");
                DdeNameService(dde, strWCHAT, 0L, DNS_UNREGISTER);
                DdeUninitialize(dde);
                exit(0);
            }
        }
    }

    /* on winxp conquer does not send this?? */
    else if (uType == XTYP_UNREGISTER)
    {
        DdeQueryString(dde, hsz1, buf, 512, CP_WINANSI);

        if (strcmp(buf, "CONQUER") == 0)
        {
            printf("DDE: CONQUER unregistered, shutting down\n");

            DdeNameService(dde, strWCHAT, 0L, DNS_UNREGISTER);
            DdeUninitialize(dde);

            /* got a better idea? */
            exit(0);
        }
    }

    return (HDDEDATA)NULL;
}

int main(int argc, char **argv)
{
    DdeInitialize(&dde, DdeCallback, APPCLASS_STANDARD|CBF_FAIL_SELFCONNECTIONS|CBF_SKIP_CONNECT_CONFIRMS, 0);

    strWCHAT = DdeCreateStringHandle(dde, "WCHAT", CP_WINANSI);
    strCONQUER = DdeCreateStringHandle(dde, "CONQUER", CP_WINANSI);
    strTOPIC = DdeCreateStringHandle(dde, "POKE TOPIC", CP_WINANSI);

    if (argc < 3)
    {
        fprintf(stderr, "%s: usage: <is_host 0/1> <ip>\n", argv[0]);
        return 1;
    }

    DWORD len,tmp;
    int is_host = atoi(argv[1]) ? 1 : 0;
    char *remote_host = argv[2];

    char handle[16];
    sprintf(handle, "Player%d", is_host ? 1 : 2);

    /* zero the 8 byte header */
    memset(poke, 0, 8);

    /* write the actual poke data after the header */
    sprintf(poke+8, START_GAME, remote_host, is_host, PORT, handle, is_host+1, is_host);

    /* write the poke length in the header, little-endian */
    len = strlen(poke+8);
    tmp = SwapFourBytes(len);
    memcpy(poke, &tmp, 4);

    printf("CNCQM: We are %s, remote player at %s:%d\n", is_host ? "the host" : "the client", remote_host, PORT);

    DdeNameService(dde, strWCHAT, 0L, DNS_REGISTER);

    printf("DDE: Waiting for CONQUER...\n");

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    printf("DDE: Exit\n");

    return 0;
}
