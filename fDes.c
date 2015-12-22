 
/*
 *
 *   Module Name: ENCRYPT
 *
 */

#include <os2.h>

ULONG fDES (ULONG ulKey, ULONG ul)
    {
    ULONG bNew;
    SHORT i;

    /* Startwert des Scramblers */
    ulKey += ulKey+1<<4;
    ulKey += 2*ulKey<<8;
    ulKey += ulKey-1<<16;

    /* Scrambler mit den Optimalabgriffen 0, 26, 27, 31 */
    for (i=0; i<32; i++)
        {
        bNew = ulKey ^ (ulKey>>26) ^ (ulKey>>27) ^ (ulKey>>31) ^ (ul>>i);
        ulKey = (ulKey<<1) + (bNew&1);
        }

    return ulKey;
    }

