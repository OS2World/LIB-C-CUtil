/*
 *
 *
 *   Module Name: BuildEnv
 *
 *   OS/2 Work Place Shell Administration Program
 *
 *
 */

#define INCL_DOS
#include <os2.h>
#include <string.h>

#define CCHMAXENVIRONMENT   0x100000

ULONG ulPageSize = 0x1000;

const PCHAR pszEnvVarSep = "=";
const PCHAR pszEnvSep    = "=\r\n";

COUNTRYCODE strucCountry = {0,0};           /* Countrycode des Aufrufers */

PCHAR SetEnv (PCHAR pszEnvOrg, PSZ pszEnvSrc);

/*****************************************************************************
 * Programm addiert die im Shared-Mem bergebene Environment zur aktuellen
 * Env. Fr die Ziel-Environment wird Speicher allokiert. Das Ergebnis wird
 * einem doppelten Terminierungszeichen "\0\0" abgeschlossen.
 * Die neuen Umgebungseintr„ge mssen mit einem Zeilentrenner '\n' voneinander
 * getrennt sein (oder '\r' oder "\r\n").
 * die neuen Umgebungseintr„ge drfen Variable der Form %VARIABLE% enthalten.
 * Die Auswertung geschieht von oben nach unten, Originalumgebung zuerst.
 * Eingang: pcEnvSource: neue Umgebungsvariable
 * return:  Zeiger auf die neue Environment
 *          NULL: Pufferbereich nicht allokierbar
 *****************************************************************************/
PCHAR MakeEnv (PCHAR pszEnvSource)
    {
    PTIB ptib;
    PPIB ppib;
    PCHAR pszEnvOrg;

    /* Aktuelle Environment des Prozesses bestimmen */
    DosGetInfoBlocks (&ptib, &ppib);
    pszEnvOrg = ppib->pib_pchenv;

    DosQuerySysInfo (QSV_PAGE_SIZE, QSV_PAGE_SIZE, &ulPageSize, sizeof (ULONG));

    /* neue Environment erzeugen */
    return SetEnv (pszEnvOrg, pszEnvSource);
    }

/*****************************************************************************
 * interne Funktion: Bestimmt die L„nge einer Envrionmentvariable
 * (Stringl„nge bis zum Trennzeichen '=').
 * Eingang: pcEnv:      Environmentstring
 *          lcEnv:      L„nge von pcEnv
 * return:  L„nge der Variable
 *          0: fehlerhafter String
 *****************************************************************************/
LONG ExtractEnvVar (PCHAR pcEnv, LONG lcEnv)
    {
    PCHAR pc;

    pc = (PCHAR)memchr (pcEnv, '=', lcEnv);
    if (pc)
        return pc - pcEnv;
    else
        return 0;
    }

/*****************************************************************************
 * interne Funktion: Sucht einen Environmenteintrag.
 * Eingang: pcEnv:      Environment
 *          lcEnv:      L„nge von pcEnv
 *          pcVar:      zu suchende Variable
 *          lcVar:      L„nge von pcVar
 * return:  Zeiger auf den Anfang des gesuchten Strings
 *          NULL: Variable nicht gefunden
 *****************************************************************************/
PSZ SearchEnvVar (PCHAR pcEnv, LONG lcEnv, PCHAR pcVar, LONG lcVar)
    {
    PCHAR pc;
    LONG  lc;

    while (lcEnv)
        {
        pc = (PCHAR)strchr (pcEnv, '=');
        if (pc && (pc - pcEnv == lcVar))
            {
            if (memicmp (pcEnv, pcVar, lcVar) == 0)
                return pc + 1;
            }
        lc = strlen (pcEnv);
        lcEnv -= lc + 1;
        pcEnv += lc + 1;
        }

    return NULL;
    }

/*****************************************************************************
 * interne Funktion: Erweitert den String pcDest um den String in pcSource.
 * Falls der Zielpuffer zu klein ist, werden weitere Pages allokiert.
 * Ein '\0' am Ende des Zielpuffers bleibt erhalten.
 * Eingang: pcDest:      Zielpuffer
 *          plSizeDest:  L„nge des Zielstrings (incl. '\0')
 *          pcSource:    Quellpuffer
 *          lSizeSource: L„nge des Quellstrings (ohne '\0')
 *          bTerminate:  TRUE:  String terminieren
 *                       FALSE: String nicht terminieren
 * Ausgang: plSizeDest:  L„nge des Zielstrings (incl. '\0') nach Einfgen
 * return:  Zeiger auf den Anfang des eingefgten String
 *          NULL: Pufferbereich nicht allokierbar
 *****************************************************************************/
PSZ InsertString (PCHAR pcDest, PLONG plcDest, PCHAR pcSource, LONG lcSource, BOOL bTerminate)
    {
    LONG  lcOldPages, lcNewPages;
    LONG  lcNewDest;
    PCHAR pcNew;

    lcNewDest = *plcDest + lcSource + (bTerminate ? 1 : 0);

    if (lcNewDest > CCHMAXENVIRONMENT)
        return NULL;

    lcOldPages = (ulPageSize + *plcDest   - 1) / ulPageSize;
    lcNewPages = (ulPageSize +  lcNewDest - 1) / ulPageSize;

    if (lcNewPages - lcOldPages)
        DosSetMem (pcDest + lcOldPages * ulPageSize,
                   lcNewDest - lcOldPages * ulPageSize,
                   PAG_COMMIT | PAG_DEFAULT);

    pcNew = pcDest + *plcDest;
    if (pcSource && lcSource)
        memcpy (pcNew, pcSource, lcSource);
    if (bTerminate)
        pcNew[lcSource] = '\0';

    *plcDest = lcNewDest;

    return pcDest + *plcDest;
    }

/*****************************************************************************
 * interne Funktion: L”st in einem Environmentstring Variable vom Typ
 * %VARIABLE% auf. Das Ergebnis wird in einem allokierten String abgelegt.
 * Eingang: pcEnv:       aktuelle Environment (zur Aufl”sung der Variablen)
 *          lcEnv:       L„nge von pcEnv
 *          pcSrc:       Quellstring (m”glicherweise mit Variablen)
 *          lcSrc:       L„nge von pcSrc
 * return:  Zeiger auf den Ergebnisstring
 *          NULL: Pufferbereich nicht allokierbar
 *****************************************************************************/
PSZ InsertVariables (PCHAR pcEnv, LONG lcEnv, PCHAR pcSrc, LONG lcSrc)
    {
    PCHAR pcDest, pc, pcV1, pcV2;
    PSZ   pszEnvVar;
    LONG  lcDest, lc;
    PCHAR pcTemp = NULL;

    if (DosAllocMem ((PPVOID)&pcDest, CCHMAXENVIRONMENT, PAG_READ | PAG_WRITE))
        return NULL;

    lcDest = 0;

    /* Variablennamen bis einschl. "=" kopieren */
    pc = (PCHAR)memchr (pcSrc, '=', (size_t)lcSrc);
    if (pc == NULL)
        goto Exit;

    lc = ++pc - pcSrc;
    pcTemp = InsertString (pcDest, &lcDest, pcSrc, lc, FALSE);
    if (!pcTemp)
        goto Exit;
    DosMapCase (lc, &strucCountry, pcDest);

    /* schrittweise Variableninhalt kopieren, jeweils bis zum Beginn der n„chsten Variablen */
    while (lc < lcSrc)
        {
        pcV1 = (PCHAR)memchr (pc, '%', (size_t)(lcSrc - lc));
        if (pcV1 && (pcV1 - pcSrc < lcSrc - 1))
            {
            /* Variablenbeginn gefunden; Ende suchen */
            pcV2 = (PCHAR)memchr (pcV1+1, '%', (size_t)(lcSrc - (pcV1+1 - pcSrc)));
            if (pcV2 == NULL)
                {
                /* keine Variable vorhanden */
                pcV1 = pcV2 = pcSrc + lcSrc;
                }
            }
        else
            {
            /* keine Variable vorhanden */
            pcV1 = pcV2 = pcSrc + lcSrc;
            }

        /* Puffer bis zum Beginn der n„chsten Variablen kopieren */
        pcTemp = InsertString (pcDest, &lcDest, pc, pcV1 - pc, FALSE);
        if (!pcTemp)
            goto Exit;
        lc += pcV1 - pc;
        pc = pcV1;

        /* evtl. vorhandene Variable aufl”sen */
        if (pcV1 < pcV2)
            {
            pszEnvVar = SearchEnvVar (pcEnv, lcEnv, pcV1 + 1, pcV2 - pcV1 - 1);
            if (pszEnvVar)
                {
                pcTemp = InsertString (pcDest, &lcDest, pszEnvVar, strlen (pszEnvVar), FALSE);
                if (!pcTemp)
                    goto Exit;
                }
            lc += pcV2 - pcV1 + 1;
            pc = pcV2 + 1;
            }
        }

    pcTemp = InsertString (pcDest, &lcDest, NULL, 0, TRUE);

Exit:
    if (!pcTemp)
        {
        DosFreeMem (pcDest);
        pcDest = NULL;
        }

    return pcDest;
    }

/*****************************************************************************
 * interne Funktion: Erweitert die aktuelle Environment um die Eintr„ge
 * in pszEnvSrc; das Ergebnis steht in einem allokierten Puffer, der als
 * Rckgabewert zurckgegeben wird.
 * Eingang: pszEnvOrg:  Originalumgebung
 *          pszEnvSrc:  neue Variable; Eintr„ge durch '\n' getrennt
 * return:  Zeiger auf die neue Environment
 *          NULL: Pufferbereich nicht allokierbar
 *****************************************************************************/
PCHAR SetEnv (PCHAR pszEnvOrg, PSZ pszEnvSrc)
    {
    LONG  lLen, lcEnvSrc;
    LONG  lcEnvTmp, lcEnvOrg, lcEnvDst;
    PCHAR pszSrc, pszSrcTmp, pszEnv, pszFinalString;
    PCHAR pszEnvTmp    = NULL;
    PCHAR pszEnvDst    = NULL;
    BOOL  bRC          = TRUE;

    /* Speicher allokieren */
    if (DosAllocMem ((PPVOID)&pszEnvTmp, CCHMAXENVIRONMENT, PAG_READ | PAG_WRITE))
        goto Exit;

    /* 1.) Alle Strings aus Original-Environment kopieren */
    lcEnvOrg = lcEnvTmp = 0;
    for (pszEnv = pszEnvOrg; *pszEnv != '\0'; pszEnv = pszEnvOrg + lcEnvOrg)
        {
        lcEnvOrg += strlen (pszEnv) + 1;
        }
    bRC = InsertString (pszEnvTmp, &lcEnvTmp, pszEnvOrg, lcEnvOrg, FALSE) ? TRUE : FALSE;
    if (!bRC)
        goto Exit;

    /* 2.) Alle Strings aus Environment-Erweiterung hinten anh„ngen, wenn Syntax korrekt ist */
    pszSrc = pszEnvSrc;
    while (*pszSrc != '\0')
        {
        /* ersten '\r' oder '\n' suchen */
        pszSrcTmp = pszSrc;
        for (lLen = 0; *pszSrc!='\n' && *pszSrc!='\r' && *pszSrc!='\0'; lLen++)
           pszSrc++;

        /* Syntax prfen: <VARIABLE=inhalt>; Im Fehlerfall Variable ignorieren */
        pszEnv = (PCHAR)memchr (pszSrcTmp, '=', (size_t) lLen);
        if (pszEnv != NULL)                 /* Enth„lt String ein '='? */
            {
            lcEnvSrc = pszEnv - pszSrcTmp;
            if (lcEnvSrc < lLen-1)          /* Ist nach '=' noch etwas? */
                {
                /* eventuell vorhandene Variable ("%var%") aufl”sen */
                pszFinalString = InsertVariables (pszEnvTmp, lcEnvTmp, pszSrcTmp, lLen);
                if (pszFinalString)
                    {
                    /* String in den Zielpuffer kopieren, falls Platz ist */
                    bRC = InsertString (pszEnvTmp, &lcEnvTmp, pszFinalString, strlen (pszFinalString), TRUE) ? TRUE : FALSE;
                    DosFreeMem (pszFinalString);
                    if (!bRC)
                        goto Exit;
                    }
                }
            }
        /* Hinter letzten '\r' oder '\n' positionieren */
        while (*pszSrc == '\n' || *pszSrc == '\r')
            pszSrc++;
        }

    /* 3.) Alle Variablen umkopieren, Duplikate entfernen (letzter Eintrag gilt) */
    /* Speicher allokieren */
    if (DosAllocMem ((PPVOID)&pszEnvDst, CCHMAXENVIRONMENT, PAG_READ | PAG_WRITE))
        goto Exit;

    lcEnvDst = 0;
    for (pszEnv = pszEnvTmp; pszEnv - pszEnvTmp < lcEnvTmp; pszEnv += lLen)
        {
        lLen = strlen (pszEnv) + 1;
        if (SearchEnvVar (pszEnv+lLen,
                          lcEnvTmp - (pszEnv - pszEnvTmp) - lLen,
                          pszEnv,
                          ExtractEnvVar (pszEnv, lLen)) == NULL)
            {
            bRC = InsertString (pszEnvDst, &lcEnvDst, pszEnv, lLen, FALSE) ? TRUE : FALSE;
            if (!bRC)
                goto Exit;
            }
        }

    /* Terminierung '\0' anh„ngen, da Environment mit "\0\0" schlieáen muá */
    bRC = InsertString (pszEnvDst, &lcEnvDst, NULL, 0, TRUE) ? TRUE : FALSE;

Exit:
    if (!bRC)
        {
        if (pszEnvTmp)
            DosFreeMem (pszEnvTmp);
        if (pszEnvDst)
            DosFreeMem (pszEnvDst);
        }

    return bRC ? pszEnvDst : NULL;
    }
