/*
 *
 *   Module Name: CRYPT
 *
 */

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "crypt.h"

BOOL cryptString (PCHAR pBuffer, ULONG ulcBuffer, CryptAction action)
    {
    LONG  lc;
    ULONG ul1, ul2;

    if (ulcBuffer & 7)
        return FALSE;

    if (action == CA_Crypt)
        {
        for (lc = 0; lc < ulcBuffer >> 3; lc++)
            {
            DES (&ul1, &ul2, *(PULONG)&pBuffer[lc*8], *(PULONG)&pBuffer[lc*8+4]);
            if (lc+1 < ulcBuffer / 8)
                {
                *(PULONG)&pBuffer[(lc+1)*8+0] ^= ul1;
                *(PULONG)&pBuffer[(lc+1)*8+4] ^= ul2;
                }
            *(PULONG)&pBuffer[lc*8+0] = ul1;
            *(PULONG)&pBuffer[lc*8+4] = ul2;
            }
        }
    else
        {
        for (lc = (ulcBuffer >> 3) - 1; lc >= 0; lc--)
            {
            DeDES (&ul1, &ul2, *(PULONG)&pBuffer[lc*8], *(PULONG)&pBuffer[lc*8+4]);
            if (lc > 0)
                {
                ul1 ^= *(PULONG)&pBuffer[(lc-1)*8+0];
                ul2 ^= *(PULONG)&pBuffer[(lc-1)*8+4];
                }
            *(PULONG)&pBuffer[lc*8+0] = ul1;
            *(PULONG)&pBuffer[lc*8+4] = ul2;
            }
        }

    return TRUE;
    }
