/*
 *
 *
 *   Module Name: DateTime
 *
 *   OS/2 Work Place Shell Administration Program
 *
 *
 */

#define INCL_DOS
#define INCL_WINSHELLDATA

#include <os2.h>
#include <string.h>
#include <cutil.h>

#define INI_APPNAME     "PM_National"
#define KEY_DATEFMT     "iDate"
#define KEY_DATESEP     "sDate"
#define KEY_TIMEFMT     "iTime"
#define KEY_TIMESEP     "sTime"
#define KEY_AM          "s1159"
#define KEY_PM          "s2359"

/* modulinterne Funktion */
PCHAR itoNc (PCHAR, USHORT, USHORT);

/*******************************************************************
   Ausgabe von Datum und Uhrzeit im l„nderspezifischen Format
   Wird ein Zeiger als NULL bergeben, wird der entsprechende
   String nicht berechnet.
   Eingang: pDateTime: Zeiger auf DATETIME-Struktur
   Ausgang: szTime: String mit Uhrzeit (Mindestl„nge 12 Zeichen)
            szDate: String mit Datum (Mindestl„nge 11 Zeichen)
   return:  Rckgabewert von DosQueryCtryInfo
 *******************************************************************/
APIRET GetDateTime (PDATETIME pDateTime, ULONG ulMode, PCHAR szTime, PCHAR szDate)
    {
    ULONG  ulLen, ulYearLen, ulYear;
    CHAR   sz[4];
    APIRET rc;
    PCHAR  pszString;
    COUNTRYCODE CtryCode;
    COUNTRYINFO CtryInfo;
    static CHAR szAM[4] = "AM";
    static CHAR szPM[4] = "PM";

    if (szDate != NULL)
        *szDate = '\0';
    if (szTime != NULL)
        *szTime = '\0';

    ulLen = sizeof (COUNTRYINFO);
    CtryCode.country  = 0;
    CtryCode.codepage = 0;
    rc = DosQueryCtryInfo (ulLen, &CtryCode, &CtryInfo, &ulLen);
    if (ulMode & MODE_PM)
        {
        if (PrfQueryProfileString (HINI_USERPROFILE, INI_APPNAME, KEY_DATEFMT,
            NULL, sz, 4) > 0)
            CtryInfo.fsDateFmt = sz[0] - '0';
        if (PrfQueryProfileString (HINI_USERPROFILE, INI_APPNAME, KEY_TIMEFMT,
            NULL, sz, 4) > 0)
            CtryInfo.fsTimeFmt = sz[0] - '0';
        if (PrfQueryProfileString (HINI_USERPROFILE, INI_APPNAME, KEY_DATESEP,
            NULL, sz, 4) > 0)
            CtryInfo.szDateSeparator[0] = sz[0];
        if (PrfQueryProfileString (HINI_USERPROFILE, INI_APPNAME, KEY_TIMESEP,
            NULL, sz, 4) > 0)
            CtryInfo.szTimeSeparator[0] = sz[0];
        if (PrfQueryProfileString (HINI_USERPROFILE, INI_APPNAME, KEY_AM,
            szAM, sz, 4) > 0)
            strcpy (szAM, sz);
        if (PrfQueryProfileString (HINI_USERPROFILE, INI_APPNAME, KEY_PM,
            szPM, sz, 4) > 0)
            strcpy (szPM, sz);
        }

    if (rc == 0)
        {
        if (szDate != NULL)
            {
            /* Datum zusammenbauen */
            ulYear = pDateTime->year;
            if (ulMode & MODE_YEAR_2D)
                {
                ulYear %= 100;
                ulYearLen = 10;
                }
            else
                ulYearLen = 1000;

            switch (CtryInfo.fsDateFmt)
                {
                case 1:
                    pszString = itoNc (szDate, pDateTime->day, 10) + 1;
                    pszString = itoNc (pszString, pDateTime->month, 10) + 1;
                    pszString = itoNc (pszString, ulYear, ulYearLen);
                    szDate[2] = szDate[5] = CtryInfo.szDateSeparator[0];
                    break;

                case 2:
                    pszString = itoNc (szDate, ulYear, ulYearLen) + 1;
                    pszString = itoNc (pszString, pDateTime->month, 10) + 1;
                    pszString = itoNc (pszString, pDateTime->day, 10);
                    szDate[ulMode & MODE_YEAR_2D ? 2 : 4] =
                    szDate[ulMode & MODE_YEAR_2D ? 5 : 7] =
                        CtryInfo.szDateSeparator[0];
                    break;

                default:
                    pszString = itoNc (szDate, pDateTime->month, 10) + 1;
                    pszString = itoNc (pszString, pDateTime->day, 10) + 1;
                    pszString = itoNc (pszString, ulYear, ulYearLen);
                    szDate[2] = szDate[5] = CtryInfo.szDateSeparator[0];
                }
            *pszString = '\0';
            }

        if (szTime != NULL)
            {
            /* Uhrzeit zusammenbauen */
            if (CtryInfo.fsTimeFmt)
                pszString = itoNc (szTime, pDateTime->hours, 10);
            else
                pszString = itoNc (szTime, ((pDateTime->hours+11) % 12) + 1, 10);
            *pszString++ = CtryInfo.szTimeSeparator[0];
            pszString = itoNc (pszString, pDateTime->minutes, 10);
            *pszString++ = CtryInfo.szTimeSeparator[0];
            pszString = itoNc (pszString, pDateTime->seconds, 10);
            *pszString = '\0';
            if (CtryInfo.fsTimeFmt == 0)
                {
                *pszString++ = ' ';
                strcpy (pszString, (pDateTime->hours-1 < 13) ? szAM : szPM);
                }
            }
        }

    return rc;
    }

PCHAR itoNc (PCHAR psz, USHORT usVal, USHORT N)
    {
    USHORT i;

    for (i=N; i>0; i/=10)
        {
        *psz++ = (CHAR)(usVal / i) + '0';
        usVal %= i;
        }

    return psz;
    }
