#include <os2.h>

#include <stdlib.h>

#include "long64.h"

// checks, if *pVal contains 0 value
BOOL isZero (const LONG64 *pVal)
    {
    if (pVal->ulLo | pVal->lHi)
        return FALSE;

    return TRUE;
    }

char *long64toa (const LONG64 *pVal,        // value to be converted
                 PCHAR pBuf,                // output string
                 USHORT usBase)             // conversion usBase
    {
    LONG64 l64q;                            // result of pVal / usBase
    LONG   lr;                              // remainder of pVal / usBase

    if (usBase > 36 || usBase < 2)              /* no conversion if wrong base */
        {
        *pBuf = '\0';
        return pBuf;
        }

    /* division 64bit/32bit */
    l64q = long64Div (pVal, usBase, &lr);

    /* output digits of pVal/usBase first */
    if (!isZero (&l64q))
        pBuf = long64toa (&l64q, pBuf, usBase);

    /* output last digit */
    *pBuf++ = "0123456789abcdefghijklmnopqrstuvwxyz"[(int)lr];
    *pBuf   = '\0';

    return pBuf;
    }
