
/*
 *
 *   Module Name: ENCRYPT
 *
 */

#include <os2.h>

ULONG fDeDES (ULONG ulKey, ULONG ul)
    {
    ULONG rc;
    ULONG bNew;
    SHORT i;

    /* Startwert des Descramblers */
    ulKey += ulKey+1<<4;
    ulKey += 2*ulKey<<8;
    ulKey += ulKey-1<<16;

    /* Descrambler fr Optimalabgriffe 0, 26, 27, 31 */
    for (i=31; i>=0; i--)
        {
        bNew = ulKey ^ (ulKey>>26) ^ (ulKey>>27) ^ (ulKey>>31) ^ (ul>>i);
        ulKey = (ulKey<<1) + ((ul>>i)&1);
        rc = (rc>>1) + (bNew&1)*0x80000000;
        }

    return rc;
    }
