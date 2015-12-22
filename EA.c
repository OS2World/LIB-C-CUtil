/***************************************************************************
 *
 * PROGRAM NAME: EA.C
 * -------------
 *
 * REVISION LEVEL: 1.2
 * ---------------
 *
 * WHAT THIS PROGRAM DOES:
 * -----------------------
 *  Routinen fÅr die Behandlung von EAs.
 *
 * ROUTINES:
 * ---------
 *  CreateGEAList
 *  CreateFEAList
 *  CreateEAOPRd
 *  CreateEAOPWr
 *  EAWriteASCII
 *  EAReadASCII
 *  EAWriteMV
 *  EAReadMV
 *
 * COMPILE REQUIREMENTS:
 * ---------------------
 *  IBM C++ Set/2 Compiler Version 2.0
 *  IBM OS/2 2.1 Programmers Toolkit
 *
 * REQUIRED FILES:
 * ---------------
 *
 * REQUIRED LIBRARIES:
 * -------------------
 *  OS2386.LIB    -   OS/2 32-Bit import library
 *
 * CHANGE LOG:
 * -----------
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.20    02-19-94  First release
 *
 *  Copyright (C) 1994 Noller & Breining Software
 *
 ******************************************************************************/
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <string.h>
#include <stdlib.h>
#include <cutil.h>

#pragma pack (1)
typedef struct _STRUC_EAT_SV        /* Struktur fÅr EAT_ASCII */
    {
    USHORT usEAType;
    USHORT uscValue;
    CHAR   cValue[1];
    } STRUC_EAT_SV;
typedef STRUC_EAT_SV *PSTRUC_EAT_SV;

typedef struct _STRUC_MVST          /* EA-Struktur fÅr strucEA */
    {                               /*  in STRUC_EA_MV         */
    USHORT uscValue;                /*  bei EAT_MVST           */
    BYTE   bValue[1];
    } STRUC_MVST;
typedef STRUC_MVST *PSTRUC_MVST;

typedef struct _STRUC_MVMT          /* EA-Struktur fÅr strucEA */
    {                               /*  in STRUC_EA_MV         */
    USHORT usEAType;                /*  bei EAT_MVMT           */
    USHORT uscValue;
    BYTE   bValue[1];
    } STRUC_MVMT;
typedef STRUC_MVMT *PSTRUC_MVMT;

typedef struct _STRUC_EAT_MV        /* Struktur fÅr EAT_MVMT/MVST */
    {
    USHORT          usEAType;
    USHORT          usCodepage;
    USHORT          uscEA;
    STRUC_MVMT      strucEA[1];
    } STRUC_EAT_MV;
typedef STRUC_EAT_MV *PSTRUC_EAT_MV;

#pragma pack ()

/*****************************************************************************
 * Entspricht DosQueryFileInfo bzw. DosQueryPathInfo, je nach ulRefType
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          ulInfoLevel: Level der benîtigten Dateiinformation
 *          pInfo:       Adresse des Speicherbereichs fÅr das Ergebnis
 *          cbInfoBuf:   LÑnge in Bytes von pInfo
 * Ausgang: pInfo:       Ergebnis
 * return:  APIRET
 *****************************************************************************/
APIRET QueryFileInfo (ULONG ulRefType, PVOID pvFile, ULONG ulInfoLevel, PVOID pInfo, ULONG cbInfoBuf)
    {
    APIRET rc;

    switch (ulRefType)
        {
        case ENUMEA_REFTYPE_PATH:
            rc = DosQueryPathInfo ((PSZ)pvFile, ulInfoLevel, pInfo, cbInfoBuf);
            break;

        case ENUMEA_REFTYPE_FHANDLE:
            rc = DosQueryFileInfo (*((PHFILE)pvFile), ulInfoLevel, pInfo, cbInfoBuf);
            break;

        default:
            rc = ERROR_INVALID_PARAMETER;
        }

    return rc;
    }

/*****************************************************************************
 * Entspricht DosSetFileInfo bzw. DosSetPathInfo, je nach ulRefType
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          ulInfoLevel: Level der benîtigten Dateiinformation
 *          pInfo:       Adresse des Speicherbereichs fÅr die Daten
 *          cbInfoBuf:   LÑnge in Bytes von pInfo
 *          flOptions:   Angabe, wie die Operation durchgefÅhrt werden soll
 *                       Wenn ulRefType = ENUMEA_REFTYPE_PATH,
 *                       ist DSPI_WRTTHRU die einzige Option.
 * return:  APIRET
 *****************************************************************************/
APIRET SetFileInfo (ULONG ulRefType, PVOID pvFile, ULONG ulInfoLevel, PVOID pInfo, ULONG cbInfoBuf, ULONG flOptions)
    {
    APIRET rc;

    switch (ulRefType)
        {
        case ENUMEA_REFTYPE_PATH:
            rc = DosSetPathInfo ((PSZ)pvFile, ulInfoLevel, pInfo, cbInfoBuf, flOptions);
            break;

        case ENUMEA_REFTYPE_FHANDLE:
            rc = DosSetFileInfo (*((PHFILE)pvFile), ulInfoLevel, pInfo, cbInfoBuf);
            break;

        default:
            rc = ERROR_INVALID_PARAMETER;
        }

    return rc;
    }

/*****************************************************************************
 * Erzeugen einer Get-EA-Liste (GEAList)
 * Die GEAList erhÑlt nur einen GEA-Eintrag. Es wird der Wert in pszValue
 * eingetragen.
 * Eingang: pszName: Token fÅr EA-Wert
 * return:  Zeiger auf GEAList
 *****************************************************************************/
PGEA2LIST CreateGEAList (PCHAR pszName)
    {
    PGEA2LIST pGEAl;

    DosAllocMem ((PPVOID)&pGEAl, sizeof (GEA2LIST) + strlen (pszName),
        PAG_COMMIT | PAG_READ | PAG_WRITE);
    pGEAl->cbList = sizeof (GEA2LIST) + strlen (pszName);
    pGEAl->list->oNextEntryOffset = 0;          /* letzer Eintrag */
    pGEAl->list->cbName = (CHAR) strlen (pszName);
    strcpy (pGEAl->list->szName, pszName);

    return pGEAl;
    }

/*****************************************************************************
 * Erzeugen einer Full-EA-Liste (FEAList)
 * Die FEAList erhÑlt nur einen FEA-Eintrag. Es wird der Wert in pszValue
 * eingetragen.
 * Eingang: pszName : Name des Attributes
 *          pValue  : Token fÅr EA-Wert
 *          uscValue: LÑnge des EA-Wertes
 * return:  Zeiger auf GEAList
 *****************************************************************************/
PFEA2LIST CreateFEAList (PCHAR pszName, PBYTE pValue, USHORT uscValue)
    {
    PFEA2LIST pFEAl;

    DosAllocMem ((PPVOID)&pFEAl, sizeof (FEA2LIST) + strlen (pszName) + uscValue,
        PAG_COMMIT | PAG_READ | PAG_WRITE);
    pFEAl->cbList = sizeof (FEA2LIST) + strlen (pszName) + uscValue;
    pFEAl->list->oNextEntryOffset = 0;          /* letzter Eintrag */
    pFEAl->list->fEA = 0;                       /* keine Flags */
    pFEAl->list->cbName = (CHAR) strlen (pszName);
    pFEAl->list->cbValue = uscValue;
    strcpy (pFEAl->list->szName, pszName);
    memcpy ((PBYTE)pFEAl->list->szName+strlen(pszName)+1, pValue, uscValue);

    return pFEAl;
    }

/*****************************************************************************
 * Erzeugen einer EAOP-Struktur mit FEA-Puffer am Ende. Dieser Puffer kann fÅr
 * DosFind*, DosGetFileInfo oder DosGetPathInfo-Aufrufe verwendet werden.
 * Eingang: ulcBuffer: Grî·e des Puffers (EAOP2 + FEAList)
 *          pGEAl:     Zeiger auf GEAList
 * return:  Zeiger auf EAOP-Struktur
 *****************************************************************************/
PEAOP2 CreateEAOPRd (ULONG ulcBuffer, PGEA2LIST pGEAl)
    {
    PEAOP2 pEAOP;

    DosAllocMem ((PPVOID)&pEAOP, ulcBuffer, PAG_COMMIT | PAG_READ | PAG_WRITE);
    pEAOP->fpGEA2List = pGEAl;
    pEAOP->fpFEA2List = (FEA2LIST *)(pEAOP + 1);
    pEAOP->fpFEA2List->cbList = ulcBuffer - sizeof (EAOP2);

    return pEAOP;
    }

/*****************************************************************************
 * Erzeugen einer EAOP-Struktur mit FEA-Puffer am Ende. Dieser Puffer kann fÅr
 * DosSetFileInfo oder DosSetPathInfo-Aufrufe verwendet werden.
 * Eingang: pFEAl:     Zeiger auf FEAList
 * return:  Zeiger auf EAOP-Struktur
 *****************************************************************************/
PEAOP2 CreateEAOPWr (PFEA2LIST pFEAl)
    {
    PEAOP2 pEAOP;

    DosAllocMem ((PPVOID)&pEAOP, sizeof (PEAOP2), PAG_COMMIT | PAG_READ | PAG_WRITE);
    pEAOP->fpGEA2List = NULL;
    pEAOP->fpFEA2List = pFEAl;

    return pEAOP;
    }

/*****************************************************************************
 * Schreiben eines EAT_ASCII-EAs.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          pszString:   Wert des EAs (ASCIIZ-String)
 * return:  TRUE:  Schreiben des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAWriteASCII (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PCHAR pszString)
    {
    BOOL          bRC;
    PSTRUC_EAT_SV peaASCII;
    PFEA2LIST     pFEAl;
    PEAOP2        pEAOP;

    /* AusfÅllen des erweiterten Attribut-Wertes */
    DosAllocMem ((PPVOID)&peaASCII, sizeof (STRUC_EAT_SV) + strlen (pszString) - 1,
        PAG_COMMIT | PAG_READ | PAG_WRITE);
    peaASCII->usEAType = EAT_ASCII;
    peaASCII->uscValue = (USHORT) strlen (pszString);
    memcpy (peaASCII->cValue, pszString, strlen (pszString));

    /* Erzeugen einer FEA-Liste */
    pFEAl = CreateFEAList (pszEAName, (PBYTE) peaASCII,
        sizeof (STRUC_EAT_SV) + (USHORT) strlen (pszString) - 1);

    /* Erzeugen der EAOP-Struktur */
    pEAOP = CreateEAOPWr (pFEAl);

    /* Schreiben der erweiterten Attribute */
    bRC = (SetFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE,
                pEAOP, sizeof (EAOP2), DSPI_WRTTHRU)) ? FALSE : TRUE;

    /* Deallokieren der Puffer */
    DosFreeMem (pEAOP);
    DosFreeMem (pFEAl);
    DosFreeMem (peaASCII);

    return bRC;
    }

/*****************************************************************************
 * Lesen eines EAT_ASCII-EAs.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          pszString:   Puffer fÅr EA
 *          puscValue:   Puffergrî·e
 * Ausgang: pszString:   Wert des EAs (ASCIIZ)
 *          puscValue:   StringlÑnge des Quellstrings im EA
 * return:  TRUE:  Lesen des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAReadASCII (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PCHAR pszString, PUSHORT puscValue)
    {
    BOOL        bRC;
    LONG        lcBytes;
    FILESTATUS4 ffb4;
    PGEA2LIST   pGEAl;
    PEAOP2      pEAOP;
    union _pEA
        {
        PFEA2   pFEA;
        PUSHORT pWord;
        PBYTE   pByte;
        } pEA;

    if (*puscValue > 0)             /* Zielstring fÅr Fehlerfall vorbereiten */
        *pszString = '\0';

    if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE, &ffb4, sizeof (FILESTATUS4)))
        {
        pGEAl = CreateGEAList (pszEAName);
        pEAOP = CreateEAOPRd (sizeof (EAOP2) + ffb4.cbList, pGEAl);
        if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASFROMLIST,
                pEAOP, sizeof (EAOP2)))
            {
            pEA.pFEA = pEAOP->fpFEA2List->list;
            if (pEA.pFEA->cbValue != 0)
                {
                pEA.pByte = (PBYTE)&(pEA.pFEA->szName) + pEA.pFEA->cbName + 1;
                if (*pEA.pWord++ == EAT_ASCII)
                    {
                    lcBytes = min (*pEA.pWord, (LONG) *puscValue-1);
                    if (lcBytes > 0)
                        {
                        memcpy (pszString, pEA.pWord + 1, lcBytes);
                        pszString[lcBytes] = '\0';
                        }
                    *puscValue = *pEA.pWord;
                    bRC = TRUE;
                    }
                }
            else
                {
                /* EA ist nicht vorhanden */
                *puscValue = 0;
                bRC = TRUE;
                }
            }
        DosFreeMem (pEAOP);
        DosFreeMem (pGEAl);
        }

    /* Im Fehlerfall eine StringlÑnge von 0 zurÅckgeben */
    if (!bRC)
        *puscValue = 0;

    return bRC;
    }

/*****************************************************************************
 * Schreiben eines Single-Value-EAs.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          pstrucValue: Eingangs-Datenstruktur
 * return:  TRUE:  Schreiben des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAWrite (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue)
    {
    BOOL          bRC;
    PSTRUC_EAT_SV peaData;
    PFEA2LIST     pFEAl;
    PEAOP2        pEAOP;

    /* AusfÅllen des erweiterten Attribut-Wertes */
    DosAllocMem ((PPVOID)&peaData,
        sizeof (STRUC_EAT_SV) + pstrucValue->uscValue - 1,
        PAG_COMMIT | PAG_READ | PAG_WRITE);
    peaData->usEAType = pstrucValue->usEAType;
    peaData->uscValue = pstrucValue->uscValue;
    memcpy (peaData->cValue, pstrucValue->pValue, pstrucValue->uscValue);

    /* Erzeugen einer FEA-Liste */
    pFEAl = CreateFEAList (pszEAName, (PBYTE) peaData,
        sizeof (STRUC_EAT_SV) + pstrucValue->uscValue - 1);

    /* Erzeugen der EAOP-Struktur */
    pEAOP = CreateEAOPWr (pFEAl);

    /* Schreiben der erweiterten Attribute */
    bRC = (SetFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE,
            pEAOP, sizeof (EAOP2), DSPI_WRTTHRU)) ? FALSE : TRUE;

    /* Deallokieren der Puffer */
    DosFreeMem (pEAOP);
    DosFreeMem (pFEAl);
    DosFreeMem (peaData);

    return bRC;
    }

/*****************************************************************************
 * Lesen eines Single-Value-EAs.
 * Wenn pstrucValue->uscValue nach RÅckkehr grî·er ist als der öbergabewert
 * beim Einsprung, war der Puffer zu klein. Der Puffer wurde bis zum Ende mit
 * dem korrekten Wert gefÅllt, bei EAT_ASCII mit '\0' terminiert.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          pstrucValue: Eingangs-Datenstruktur
 * Ausgang: pstrucValue: Ergebnis
 * return:  TRUE:  Lesen des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EARead (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue)
    {
    BOOL        bRC;
    LONG        lcBytes;
    FILESTATUS4 ffb4;
    PGEA2LIST   pGEAl;
    PEAOP2      pEAOP;
    union _pEA
        {
        PFEA2   pFEA;
        PUSHORT pWord;
        PBYTE   pByte;
        } pEA;

    bRC = FALSE;                    /* RÅckgabewert fÅr Fehlerfall vorbereiten */

    if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE, &ffb4, sizeof (FILESTATUS4)))
        {
        pGEAl = CreateGEAList (pszEAName);
        pEAOP = CreateEAOPRd (sizeof (EAOP2) + ffb4.cbList, pGEAl);
        if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASFROMLIST,
                pEAOP, sizeof (EAOP2)))
            {
            pEA.pFEA = pEAOP->fpFEA2List->list;
            if (pEA.pFEA->cbValue != 0)
                {
                pEA.pByte = (PBYTE)&(pEA.pFEA->szName) + pEA.pFEA->cbName + 1;
                pstrucValue->usEAType = *pEA.pWord++;
                if (pstrucValue->usEAType != EAT_MVMT &&
                    pstrucValue->usEAType != EAT_MVST &&
                    pstrucValue->usEAType != EAT_ASN1)
                    {
                    lcBytes = min (*pEA.pWord, (LONG)pstrucValue->uscValue);
                    if (lcBytes > 0)
                        {
                        memcpy (pstrucValue->pValue, pEA.pWord + 1, lcBytes);
                        if (pstrucValue->usEAType == EAT_ASCII)
                            {
                            /* EAT_ASCII-EAs werden immer mit '\0' terminiert */
                            lcBytes = min (*pEA.pWord, (LONG)pstrucValue->uscValue - 1);
                            pstrucValue->pValue[lcBytes] = '\0';
                            }
                        }
                    pstrucValue->uscValue = *pEA.pWord;
                    bRC = TRUE;
                    }
                }
            else
                {
                /* EA ist nicht vorhanden */
                pstrucValue->uscValue = 0;
                bRC = TRUE;
                }
            }
        DosFreeMem (pEAOP);
        DosFreeMem (pGEAl);
        }

    /* Im Fehlerfall eine StringlÑnge von 0 zurÅckgeben */
    if (!bRC)
        pstrucValue->uscValue = 0;

    return bRC;
    }

/*****************************************************************************
 * Schreiben von Multi-Value EAs (EAT_MVST und EAT_MVMT). Die Daten werden
 * in der Struktur arValue[] Åbergeben. Der letzte Eintrag mu·
 * arValue[].pValue = NULL enthalten. Bei EAT_MVST ist nur der erste Wert von
 * arValue[].usEAType relevant.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          usEAType:    EAT_MVST oder EAT_MVMT
 *          arValue:     Eingangs-Datenstruktur
 * return:  TRUE:  Schreiben des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAWriteMV (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName,
                USHORT usEAType, STRUC_EAT_DATA arValue[])
    {
    BOOL            bRC;
    ULONG           ulcValue, ulcBytes, i;
    PSTRUC_EAT_MV   peaMV;
    union _pEA
        {
        PSTRUC_MVMT pMT;                /* Multitype EA  */
        PSTRUC_MVST pST;                /* Singletype EA */
        } pEA;
    PFEA2LIST       pFEAl;
    PEAOP2          pEAOP;

    /* ZÑhlen der EA-EintrÑge; Ergebnis ist in ulcValue */
    for (ulcValue=0; arValue[ulcValue].pValue != NULL; ulcValue++);
    if (ulcValue == 0)
        return TRUE;

    /* Speicher allokieren fÅr die Attribute; STRUC_EAT_MV enthÑlt */
    /* defaultmÑ·ig eine MVMT-Struktur, auch bei MVST-Attributen.  */
    ulcBytes = sizeof (STRUC_EAT_MV);
    switch (usEAType)
        {
        case EAT_MVST:
            ulcBytes += (ulcValue-1)*sizeof (STRUC_MVST);
            break;

        case EAT_MVMT:
            ulcBytes += (ulcValue-1)*sizeof (STRUC_MVMT);
            break;

        default:
            return FALSE;
        }
    for (i=0; i<ulcValue; i++)
        ulcBytes += arValue[i].uscValue - 1;
    DosAllocMem ((PPVOID)&peaMV, ulcBytes, PAG_COMMIT | PAG_READ | PAG_WRITE);

    /* AusfÅllen des erweiterten Attribut-Wertes */
    peaMV->usEAType   = usEAType;
    peaMV->usCodepage = 0;
    peaMV->uscEA      = ulcValue;
    pEA.pMT = peaMV->strucEA;
    if (usEAType == EAT_MVST)                       /* Bei EA_MVST: Typ des ersten */
        {                                           /*  EAs fÅr gesamte EA         */
        pEA.pMT->usEAType = arValue[0].usEAType;    /*  Åbernehmen                 */
        pEA.pMT = (PSTRUC_MVMT)&(peaMV->strucEA[0].uscValue);
        }
    for (i=0; i<ulcValue; i++)
        {
        if (usEAType == EAT_MVMT)
            {
            pEA.pMT->usEAType = arValue[i].usEAType;
            pEA.pST = (PSTRUC_MVST)&(pEA.pMT->uscValue);
            }
        /* bei EA_MVMT ist usEAType geschrieben. Ab sofort */
        /* wird pEA als PSTRUC_MVST-Zeiger verwendet.      */
        pEA.pST->uscValue  = arValue[i].uscValue;
        memcpy (pEA.pST->bValue, arValue[i].pValue, pEA.pST->uscValue);
        pEA.pST = (PSTRUC_MVST)(pEA.pST->bValue + pEA.pST->uscValue);
        }

    /* Erzeugen einer FEA-Liste */
    pFEAl = CreateFEAList (pszEAName, (PBYTE) peaMV, ulcBytes);

    /* Erzeugen der EAOP-Struktur */
    pEAOP = CreateEAOPWr (pFEAl);

    /* Schreiben der erweiterten Attribute */
    bRC = (SetFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE,
        pEAOP, sizeof (EAOP2), DSPI_WRTTHRU)) ? FALSE : TRUE;

    /* Deallokieren der Puffer */
    DosFreeMem (pEAOP);
    DosFreeMem (pFEAl);
    DosFreeMem (peaMV);

    return bRC;
    }

/*****************************************************************************
 * Lesen von Multi-Value EAs (EAT_MVST und EAT_MVMT). Die Daten werden
 * in der Struktur arValue[] Åbergeben. Der letzte Eintrag mu·
 * arValue[].pValue = NULL enthalten. Bei EAT_MVST wird der EA-Typ in
 * alle arValue[0].usEAType eingetragen. Ist die arValue-Struktur zu lang,
 * werden alle Åbrigen arValue[].uscValue-Werte zu 0 gesetzt.
 * EAT_ASCII-EAs werden immer mit '\0' terminiert. Ist der Puffer zu klein,
 * (arValue[].uscValue), so wird der String abgehackt; '\0' wird trotzdem
 * geschrieben.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          usEAType:    EAT_MVST oder EAT_MVMT
 *          arValue:     Eingangs-Datenstruktur
 * Ausgang: arValue:     Ergebnis
 * return:  TRUE:  Lesen des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAReadMV (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName,
               USHORT usEAType, STRUC_EAT_DATA arValue[])
    {
    USHORT      usEAType2;
    BOOL        bRC;
    ULONG       ulcValue, ulcMaxEA, ulcBytes, i;
    FILESTATUS4 ffb4;
    PGEA2LIST   pGEAl;
    PEAOP2      pEAOP;
    union _pEA
        {
        PFEA2           pFEA;
        PSTRUC_EAT_MV   peaMV;
        PSTRUC_MVST     pST;
        PSTRUC_MVMT     pMT;
        PUSHORT         pWord;
        PBYTE           pByte;
        } pEA;

    /* ZÑhlen der ASCII-Strings; Ergebnis ist in ulcValue */
    for (ulcValue=0; arValue[ulcValue].pValue != NULL; ulcValue++);
    if (ulcValue == 0)
        return TRUE;

    /* Lesen der Attributinformation */
    bRC = FALSE;
    if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE, &ffb4, sizeof (FILESTATUS4)))
        {
        pGEAl = CreateGEAList (pszEAName);
        pEAOP = CreateEAOPRd (sizeof (EAOP2) + ffb4.cbList, pGEAl);
        if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASFROMLIST,
                pEAOP, sizeof (EAOP2)))
            {
            pEA.pFEA = pEAOP->fpFEA2List->list;
            /* Attribut gefunden? */
            if (pEA.pFEA->cbValue != 0)
                {
                pEA.pByte = (PBYTE)&(pEA.pFEA->szName) + pEA.pFEA->cbName + 1;
                /* Attributtypen korrekt? */
                if (pEA.peaMV->usEAType == usEAType)
                    {
                    ulcMaxEA = pEA.peaMV->uscEA;
                    pEA.pMT = pEA.peaMV->strucEA;
                    if (usEAType == EAT_MVST)
                        {
                        usEAType2 = arValue[0].usEAType = pEA.pMT->usEAType;
                        pEA.pMT = (PSTRUC_MVMT)&(pEA.pMT->uscValue);
                        }
                    for (i=0; i<ulcValue; i++)
                        {
                        if (i<ulcMaxEA)
                            {
                            if (usEAType == EAT_MVMT)
                                {
                                arValue[i].usEAType = pEA.pMT->usEAType;
                                pEA.pST = (PSTRUC_MVST)&(pEA.pMT->uscValue);
                                }
                            else
                                arValue[i].usEAType = usEAType2;
                            ulcBytes = min (pEA.pST->uscValue, arValue[i].uscValue);
                            /* EAT_ASCII-EAs werden immer mit '\0' terminiert */
                            if (arValue[i].usEAType == EAT_ASCII)
                                {
                                ulcBytes = min (ulcBytes, arValue[i].uscValue-1);
                                arValue[i].pValue[ulcBytes] = '\0';
                                }
                            arValue[i].uscValue = ulcBytes;
                            memcpy (arValue[i].pValue, pEA.pST->bValue, ulcBytes);
                            }
                        else
                            arValue[i].uscValue = 0;
                        pEA.pST = (PSTRUC_MVST)(pEA.pST->bValue + pEA.pST->uscValue);
                        }
                    arValue[ulcValue].uscValue = 0;
                    bRC = TRUE;
                    }
                }
            else
                {
                /* EA ist nicht vorhanden */
                bRC = TRUE;
                for (i=0; i<ulcValue; i++)
                    {
                    arValue[i].uscValue = 0;
                    arValue[i].usEAType = 0;
                    }
                }
            }

        DosFreeMem (pEAOP);
        DosFreeMem (pGEAl);
        }

    /* Im Fehlerfall alle LÑngenvariable mit 0 fÅllen */
    if (!bRC)
        for (i=0; i<ulcValue; i++)
            arValue[i].uscValue = 0;

    return bRC;
    }

/*****************************************************************************
 * Schreiben der Rohdaten eines EAs
 * pstrucValue.uscValue enthÑlt die GesamtlÑnge eines EAs inklusive
 * EAT_*-Wert.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          pstrucValue: Eingangs-Datenstruktur
 * return:  TRUE:  Schreiben des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAWriteRaw (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue)
    {
    BOOL          bRC;
    PBYTE         peaData;
    PFEA2LIST     pFEAl;
    PEAOP2        pEAOP;

    /* AusfÅllen des erweiterten Attribut-Wertes */
    DosAllocMem ((PPVOID)&peaData,
        pstrucValue->uscValue,
        PAG_COMMIT | PAG_READ | PAG_WRITE);
    memcpy (peaData, pstrucValue->pValue, pstrucValue->uscValue);

    /* Erzeugen einer FEA-Liste */
    pFEAl = CreateFEAList (pszEAName, (PBYTE) peaData, pstrucValue->uscValue);

    /* Erzeugen der EAOP-Struktur */
    pEAOP = CreateEAOPWr (pFEAl);

    /* Schreiben der erweiterten Attribute */
    bRC = (SetFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE,
        pEAOP, sizeof (EAOP2), DSPI_WRTTHRU)) ? FALSE : TRUE;

    /* Deallokieren der Puffer */
    DosFreeMem (pEAOP);
    DosFreeMem (pFEAl);
    DosFreeMem (peaData);

    return bRC;
    }


/*****************************************************************************
 * Lesen der Rohdaten eines EAs.
 * Wenn pstrucValue->uscValue nach RÅckkehr grî·er ist als der öbergabewert
 * beim Einsprung, war der Puffer zu klein. Der Puffer wurde bis zum Ende mit
 * dem korrekten Wert gefÅllt.
 * Am Ausgang enthÑlt der Puffer das komplette EA inklusive dem EA-Type EAT_*.
 * Eingang: ulRefType:   Gibt an, ob pvFile auf ein Dateihandle
 *                       (ENUMEA_REFTYPE_FHANDLE) oder Dateinamen
 *                       (ENUMEA_REFTYPE_PATH) zeigt.
 *          pvFile:      Adresse des Dateihandles oder Name einer
 *                       Datei bzw. Verzeichnisses
 *          pszEAName:   Name des EAs
 *          pstrucValue: Eingangs-Datenstruktur
 * Ausgang: pstrucValue: Ergebnis
 * return:  TRUE:  Lesen des EAs ok
 *          FALSE: Fehler aufgetreten
 *****************************************************************************/
BOOL EAReadRaw (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue)
    {
    BOOL        bRC;
    LONG        lcBytes;
    FILESTATUS4 ffb4;
    PGEA2LIST   pGEAl;
    PEAOP2      pEAOP;
    union _pEA
        {
        PFEA2   pFEA;
        PUSHORT pWord;
        PBYTE   pByte;
        } pEA;

    bRC = FALSE;                    /* RÅckgabewert fÅr Fehlerfall vorbereiten */

    if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASIZE, &ffb4, sizeof (FILESTATUS4)))
        {
        pGEAl = CreateGEAList (pszEAName);
        pEAOP = CreateEAOPRd (sizeof (EAOP2) + ffb4.cbList, pGEAl);
        if (!QueryFileInfo (ulRefType, pvFile, FIL_QUERYEASFROMLIST,
                pEAOP, sizeof (EAOP2)))
            {
            pEA.pFEA = pEAOP->fpFEA2List->list;
            if (pEA.pFEA->cbValue != 0)
                {
                lcBytes = min (pEA.pFEA->cbValue, (LONG)pstrucValue->uscValue);
                pstrucValue->uscValue = pEA.pFEA->cbValue;
                pEA.pByte = (PBYTE)&(pEA.pFEA->szName) + pEA.pFEA->cbName + 1;
                /* pEA.pByte zeigt jetzt auf den EA-Typ und damit den Anfang des EAs */
                pstrucValue->usEAType = *pEA.pWord;
                memcpy (pstrucValue->pValue, pEA.pWord, lcBytes);
                bRC = TRUE;
                }
            else
                {
                /* EA ist nicht vorhanden */
                pstrucValue->uscValue = 0;
                bRC = TRUE;
                }
            }
        DosFreeMem (pEAOP);
        DosFreeMem (pGEAl);
        }

    /* Im Fehlerfall eine StringlÑnge von 0 zurÅckgeben */
    if (!bRC)
        pstrucValue->uscValue = 0;

    return bRC;
    }

