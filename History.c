/*
 *
 *
 *   Module Name: History
 *
 *
 */

#define INCL_DOSMEMMGR
#define INCL_DOSDATETIME
#define INCL_DOSMISC
#include <os2.h>
#include <string.h>
#include <cutil.h>

BOOL History (PCHAR szFileName, PCHAR szUser, PCHAR szEvent)
    {
    STRUC_EAT_DATA  arValue[MAXHISTORYLINES + 2];
    DATETIME        DateTime;
    CHAR            szNewEntry[CCHMAXHISTORY];
    CHAR            szDate[20], szTime[20];
    PUCHAR          IvTable[4];
    ULONG           IvCount = 4;
    ULONG           ul;
    BOOL            bRC;
    IvTable[0] = szUser;
    IvTable[1] = szEvent;
    IvTable[2] = szDate;
    IvTable[3] = szTime;

    DosGetDateTime (&DateTime);

    /* Wegen Fullscreen-Programmen nicht MODE_PM angeben! */
    GetDateTime (&DateTime, MODE_YEAR_2D, szTime, szDate);

    DosInsertMessage (IvTable, IvCount,
                      HISTORYTEMPLATE, sizeof (HISTORYTEMPLATE) + 1,
                      szNewEntry, CCHMAXHISTORY-1, &ul);
    szNewEntry[ul] = '\0';

    /* Speicher zum Lesen des History-Feldes vorbereiten */
    DosAllocMem ((PPVOID) &arValue[0].pValue, MAXHISTORYLINES * CCHMAXHISTORY,
         PAG_COMMIT | PAG_READ | PAG_WRITE);

    memset (arValue[0].pValue, '\0', MAXHISTORYLINES * CCHMAXHISTORY);

    arValue[0].uscValue = CCHMAXHISTORY;

    for (ul=1; ul<MAXHISTORYLINES; ul++)
       {
       arValue[ul].uscValue = CCHMAXHISTORY;
       arValue[ul].pValue   = arValue[ul-1].pValue + CCHMAXHISTORY;
       }

    arValue[MAXHISTORYLINES].pValue = NULL;
    bRC = EAReadMV (ENUMEA_REFTYPE_PATH, (PVOID)szFileName, EA_HISTORYNAME, EAT_MVMT, arValue);

    if (bRC == FALSE)
        ul = 0;
    else
        {
        /* Suchen des letzten benutzten Elements */
        for (ul=0; (ul<MAXHISTORYLINES) && (arValue[ul].uscValue!=0); ul++)
            ;
        }

    /* neue History-Zeile eintragen */
    arValue[ul].pValue   = szNewEntry;
    arValue[ul].uscValue = strlen(szNewEntry);
    arValue[ul].usEAType = EAT_ASCII;

    /* History-Feld zurckschreiben */
    arValue[ul+1].pValue   = NULL;
    arValue[ul+1].uscValue = 0;
    bRC = EAWriteMV (ENUMEA_REFTYPE_PATH, (PVOID)szFileName, EA_HISTORYNAME, EAT_MVMT,
        (ul == MAXHISTORYLINES) ? arValue+1 : arValue);

    return bRC;
    }
