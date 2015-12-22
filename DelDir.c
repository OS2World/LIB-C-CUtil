#define INCL_FILEMGR
#include <os2.h>
#include <string.h>

BOOL FileExist (PCHAR szFileName)
    {
    HDIR         hDir;
    FILEFINDBUF3 ffbFile;
    ULONG        ulSearchCount, ulReturn;

    hDir          = HDIR_CREATE;        /* Neues Handle erzeugen */
    ulSearchCount = 1;                  /* 1 Datei suchen */

    ulReturn = DosFindFirst(szFileName, &hDir,
        FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY,
        &ffbFile, sizeof (FILEFINDBUF3), &ulSearchCount, FIL_STANDARD);

    DosFindClose (hDir);

    return ulReturn ? FALSE : TRUE;
    }


APIRET DelDir (PCHAR szFileName)
    {
    HDIR         hDir;
    FILESTATUS3  flStatus;
    FILEFINDBUF3 ffbMyFile;
    USHORT       usIndex;
    ULONG        ulSearchCount, ulReturn;
    CHAR         szFullPath[CCHMAXPATH];


    hDir          = HDIR_CREATE;        /* Neues Handle erzeugen */
    ulSearchCount = 1;                  /* 1 Datei suchen */

    ulReturn = DosFindFirst(szFileName, &hDir,
        FILE_DIRECTORY | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY | FILE_ARCHIVED,
        &ffbMyFile, sizeof (FILEFINDBUF3), &ulSearchCount, FIL_STANDARD);

    if (ulReturn)
        return ulReturn;                /* keine Datei gefunden */

    if (ulSearchCount == 1)
        {
        usIndex = strlen(szFileName);

        while ((szFileName[usIndex] != '\\')
            && (szFileName[usIndex] != ':')
            && (usIndex != 0))
             usIndex--;
        usIndex++;                      /* Stringl„nge ohne letzte Komponente */

        do
            {
            memcpy (szFullPath, szFileName, usIndex);
            strcpy (szFullPath + usIndex, ffbMyFile.achName);
            if ((strcmp(ffbMyFile.achName, ".") != 0) &&
                (strcmp(ffbMyFile.achName, "..") != 0))
                {
                /* Fehlerbehandlung soll das Betriebssystem machen! */
                DosQueryPathInfo (szFullPath, FIL_STANDARD,
                    &flStatus, sizeof (FILESTATUS3));
                flStatus.attrFile = 0;
                DosSetPathInfo (szFullPath, FIL_STANDARD,
                    &flStatus, sizeof (FILESTATUS3), DSPI_WRTTHRU);
                if (ffbMyFile.attrFile & 0x0010)        /* Directory */
                    {
                    strcat(szFullPath, "\\*");
                    DelDir (szFullPath);
                    szFullPath [strlen (szFullPath)-2] = '\0';
                    DosDeleteDir (szFullPath);
                    }
                else                                    /* File */
                    DosDelete (szFullPath);
                }

            /* n„chste Datei suchen */
            ulSearchCount = 1;
            ulReturn = DosFindNext(hDir, &ffbMyFile,
                sizeof (FILEFINDBUF3), &ulSearchCount);
            if (ulReturn)
                break;
            }
        while (ulSearchCount == 1);
        }

    DosFindClose (hDir);
    return ulReturn;
    }
