/*
 *
 *   Module Name: CHIFFRE
 *
 */

#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include "crypt.h"

PCHAR ChiffreSZ (PCHAR psz)
    {
    PCHAR psz1;
    CHAR  cKey = strlen (psz);
    CHAR  cCount = 0x55;

    psz1 = psz;
    while (*psz1)
        {
        *psz1 ^= cCount++;
        *psz1 += ++cKey;

        if (*psz1 == '\0')
            *psz1 = ' ';

        cKey ^= *psz1;
        psz1++;
        }

    return psz;
    }

