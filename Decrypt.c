/*
 *
 *   Module Name: DECRYPT
 *
 */

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "crypt.h"

extern ULONG key[];

ULONG RegCheck (PCHAR pszUser, PCHAR pszPasswd,
                PUSHORT pusSerial, BYTE bMajor, BYTE bMinor)
    {
    ULONG rc;
    BYTE bVerMajor, bVerMinor;
    PCHAR psz;

    psz = malloc ((size_t) strlen (pszUser)+1);

    if ((*pszUser == '\0') && (*pszPasswd == '\0'))
        rc = REGCHECK_SW;                       /* Shareware Version */
    else
        {
        strcpy (psz, pszUser);
        if (!decrypt (pszPasswd, psz, &bVerMajor, &bVerMinor, pusSerial))
            rc = REGCHECK_FAILED;               /* Registrierkennwort falsch */
        else
            {
            if (bMajor < bVerMajor)
                rc = REGCHECK_OK;
            else if ((bMajor == bVerMajor) && (bMinor <= bVerMinor))
                rc = REGCHECK_OK;
            else
                rc = REGCHECK_WV;               /* Registrierkennwort abgelaufen */
            }
        }

    if ((rc != REGCHECK_OK) && (rc != REGCHECK_WV))
        *pusSerial = 0;
    free (psz);
    return rc;
    }

BOOL decrypt   (PCHAR szPasswd, PCHAR szUser,
                PBYTE pbMajor, PBYTE pbMinor, PUSHORT pusSerial)
    {
    ULONG i, ul, ulpw1, ulpw2;

    DeChiffre (szPasswd, &ulpw1, &ulpw2);
    *pusSerial = (USHORT) (ulpw1 & 0xFFFF);
    *pbMinor   = (BYTE) ((ulpw1>>16) & 0xFF);
    *pbMajor   = (BYTE) ((ulpw1>>24) & 0xFF);

    ChiffreSZ (szUser);
    for (ul=i=0; i<strlen (szUser); i++)
        ul ^= szUser[i]<<(8*(i%4));

    return (ulpw2 == ul);
    }

void DeChiffre (PCHAR buffer, PULONG pulpw1, PULONG pulpw2)
    {
    CHAR buffer1[8], buffer2[8];
    ULONG i;

    for (i=0; i<7; i++)
        {
        buffer1[i] = buffer[2*i];
        buffer2[i] = buffer[2*i+1];
        }
    buffer1[7] = buffer2[7] = '\0';

    *pulpw1 = strtoul (buffer1, NULL, 36);
    *pulpw2 = strtoul (buffer2, NULL, 36);
    DeDES (pulpw1, pulpw2, *pulpw1, *pulpw2);

    return;
    }

void DeDES (PULONG plo, PULONG pro, ULONG li, ULONG ri)
    {
    ULONG i;

    /* EntschlÅsselung */
    for (i=0; i<16; i++)
        {
        DeDESstep (pro, plo, ri, li, (15-i<8?key[0]:key[1])>>((15-i&7)*4));
        li = *plo;
        ri = *pro;
        }

    /* RÅckvertauschung */
    *pro = _lrotr (*pro, 20);
    *plo = _lrotr (*plo, 20);
    for (i=0; i<4; i++, *plo=_lrotr (*plo, 8), *pro=_lrotr (*pro, 8))
        {
        *pro = (*pro&0xFFFFFF00) + ((*pro-*plo)&0xFF);
        *plo = (*plo&0xFFFFFF00) + ((*plo-*pro)&0xFF);
        }

    return;
    }

void DeDESstep (PULONG pl1, PULONG pr1, ULONG l0, ULONG r0, ULONG ulKey)
    {
    ulKey &= 0xF;
    *pl1 = r0;
    *pr1 = l0 ^ fDES (ulKey, r0);
    return;
    }

