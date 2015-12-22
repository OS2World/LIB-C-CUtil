/***************************************************************************
 *
 * PROGRAM NAME: GETLANG.C
 * -------------
 *
 * REVISION LEVEL: 1.0
 * ---------------
 *
 * WHAT THIS PROGRAM DOES:
 * -----------------------
 *  Determines the language of the system and loads the appropriate language
 *  DLL file. It consists of a base name xxxx and a three digit language
 *  code. The provided default name will be used, if none is found. Language
 *  code is determined from <HINI_USERPROFILE\PM_National\iCountry>.
 *
 * ROUTINES:
 * ---------
 *
 * COMPILE REQUIREMENTS:
 * ---------------------
 *  IBM Visual Age C++
 *
 * REQUIRED FILES:
 * ---------------
 *
 * REQUIRED LIBRARIES:
 * -------------------
 *
 * CHANGE LOG:
 * -----------
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00    02-11-02  First release
 *
 *  Copyright (C) 2002 Noller & Breining Software
 *
 ******************************************************************************/
#define INCL_WINSHELLDATA
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>

#include <string.h>
#include <stdlib.h>
#include <cutil.h>

VOID determineLangFile (PSZ pszModuleName, BOOL bIsDefault)
    {
    LONG    lCountry;
    CHAR    szCountry[4];

    if (!bIsDefault)
        {
        lCountry = PrfQueryProfileInt (HINI_USERPROFILE, "PM_National", "iCountry", 1);

        if (lCountry > 0 && lCountry < 999)
            {
            _itoa (lCountry, szCountry, 10);
            strcpy (pszModuleName + strlen (pszModuleName) - strlen (szCountry), szCountry);
            }
        }

    return;
    }

BOOL checkHelpFile (PSZ pszHelpFileName)
    {
    CHAR   szPath[CCHMAXPATH];
    APIRET rc;

    rc = DosSearchPath (SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT,
                        "HELP",
                        pszHelpFileName,
                        szPath,
                        CCHMAXPATH);

    if (rc == NO_ERROR)
        return TRUE;

    rc = DosSearchPath (SEARCH_IGNORENETERRS,
                        ".",
                        pszHelpFileName,
                        szPath,
                        CCHMAXPATH);

    return (rc == NO_ERROR) ? TRUE : FALSE;
    }

/*******************************************************************\
    LoadLanguageModule: loads the language dependent module.
    Eingang: pszModuleName: default name of the module. It consists
                            of the base name and a three digit
                            language code. The provided name will
                            be used as default name and modified
                            on return, if different system language
                            has been determined.
    Return:  module handle of language module
             NULLHANDLE if error
\*******************************************************************/
HMODULE LoadLanguageModule (PSZ pszModuleName)
    {
    CHAR    szModuleName[CCHMAXPATHCOMP];
    HMODULE hModule;
    APIRET  rc;

    if (strlen (pszModuleName) >= CCHMAXPATHCOMP)
        return NULLHANDLE;

    strcpy (szModuleName, pszModuleName);
    determineLangFile (szModuleName, FALSE);

    rc = DosLoadModule (NULL, 0, szModuleName, &hModule);
    if ((rc != NO_ERROR) || (hModule == NULLHANDLE))
        {
        strcpy (szModuleName, pszModuleName);
        determineLangFile (szModuleName, TRUE);

        rc = DosLoadModule (NULL, 0, szModuleName, &hModule);
        if (rc != NO_ERROR)
            hModule = NULLHANDLE;
        }

    strcpy (pszModuleName, szModuleName);
    return hModule;
    }

/*******************************************************************\
    DetermineHelpFileName: determines the name of the help file
                           depending on the language of the system
    Eingang: pszModuleName: default name of the file. It consists
                            of the base name, a three digit
                            language code and an optional ending.
                            The provided name will be used as
                            default name and modified
                            on return, if different system language
                            has been determined.
    Return:  TRUE  if valid help file found
             FALSE if no help file found
\*******************************************************************/
BOOL DetermineHelpFileName (PSZ pszHelpFileName)
    {
    CHAR szHelpFileName[CCHMAXPATHCOMP];
    PSZ  pszFT;
    BOOL bIsValidFile;

    if (strlen (pszHelpFileName) >= CCHMAXPATHCOMP)
        return FALSE;

    pszFT = strrchr (pszHelpFileName, '.');

    strcpy (szHelpFileName, pszHelpFileName);
    if (pszFT)
        szHelpFileName[pszFT - pszHelpFileName] = '\0';
    determineLangFile (szHelpFileName, FALSE);
    if (pszFT)
        strcat (szHelpFileName, pszFT);

    bIsValidFile = checkHelpFile (szHelpFileName);
    if (!bIsValidFile)
        {
        strcpy (szHelpFileName, pszHelpFileName);
        if (pszFT)
            szHelpFileName[pszFT - pszHelpFileName] = '\0';
        determineLangFile (szHelpFileName, TRUE);
        if (pszFT)
            strcat (szHelpFileName, pszFT);

        bIsValidFile = checkHelpFile (szHelpFileName);
        }

    strcpy (pszHelpFileName, szHelpFileName);
    return bIsValidFile;
    }

