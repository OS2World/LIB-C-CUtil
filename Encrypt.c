/*
 *
 *   Module Name: ENCRYPT
 *
 */

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "crypt.h"

extern ULONG key[];

void encrypt (PCHAR szPasswd, PCHAR szString, BYTE bMajor, BYTE bMinor, USHORT usSerial)
    {
    ULONG i, ulpw1, ulpw2;

    ulpw1  = (bMajor<<24) + (bMinor<<16) + usSerial;

    ChiffreSZ (szString);
    for (ulpw2=i=0; i<strlen (szString); i++)
        ulpw2 ^= szString[i]<<(8*(i%4));

    Chiffre (szPasswd, ulpw1, ulpw2);

    return;
    }

void Chiffre (PCHAR buffer, ULONG ulpw1, ULONG ulpw2)
    {
    CHAR buffer1[8], buffer2[8];
    ULONG i, ulc1, ulc2;

    DES (&ulpw1, &ulpw2, ulpw1, ulpw2);
    ulc1 = strlen (_ultoa (ulpw1, buffer1, 36));
    ulc2 = strlen (_ultoa (ulpw2, buffer2, 36));
    for (i=0; i<7; i++)
        {
        buffer[2*i]   = (7-i>ulc1) ? '0' : buffer1[i-7+ulc1];
        buffer[2*i+1] = (7-i>ulc2) ? '0' : buffer2[i-7+ulc2];
        }
    buffer[14] = '\0';
    return;
    }

void DES (PULONG plo, PULONG pro, ULONG li, ULONG ri)
    {
    ULONG i;

    /* Anfangsvertauschung */
    for (i=0; i<4; i++, li=_lrotr (li, 8), ri=_lrotr (ri, 8))
        {
        li = (li&0xFFFFFF00) + ((li+ri)&0xFF);
        ri = (ri&0xFFFFFF00) + ((ri+li)&0xFF);
        }
    li = _lrotl (li, 20);
    ri = _lrotl (ri, 20);

    /* Verschlsselung */
    for (i=0; i<16; i++)
        {
        /* jeder Teilschlssel hat 4 Bit */
        DESstep (plo, pro, li, ri, (i<8?key[0]:key[1])>>((i&7)*4));
        li = *plo;
        ri = *pro;
        }

    return;
    }

void DESstep (PULONG pl1, PULONG pr1, ULONG l0, ULONG r0, ULONG ulKey)
    {
    ulKey &= 0xF;
    *pl1 = r0;
    *pr1 = l0 ^ fDES (ulKey, r0);
    return;
    }

