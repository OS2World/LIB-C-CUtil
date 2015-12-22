/*******************************************************************
 * OS/2 Utility Funktionen
 *
 * CHANGE LOG:
 * -----------
 *
 *  Ver.    Date      Comment
 *  ----    --------  -------
 *  1.00              First release
 *  1.10    02-11-02  GetLang hinzugefÅgt
 *
 *  Copyright (C) 2002 Noller & Breining Software
 *******************************************************************/
#ifndef __CUTIL__
  #define __CUTIL__

  /* Definitionen fÅr die History-Funktion */
  #define HISTORYTEMPLATE "%1: %2 (%3 / %4)"    /* Aufbau der History-Zeile */

  #define CCHMAXHISTORY         64              /* Maximale ZeilenlÑnge im Protokoll-Feld */
  #define MAXHISTORYLINES       256             /* Maximale Zeilenzahl im Protokoll-Feld */

  #define EA_HISTORYNAME    ".HISTORY"          /* EA-Name fÅr History */

  /* Struktur fÅr die öbergabe von Werten an EAWriteMVMT */
  typedef struct _STRUC_EAT_DATA
      {
      USHORT          usEAType;
      USHORT          uscValue;
      PBYTE           pValue;
      } STRUC_EAT_DATA;
  typedef STRUC_EAT_DATA *PSTRUC_EAT_DATA;

  /* Definitionen fÅr GetDateTime */
  #define MODE_YEAR_2D      0x0001              /* Datum nur Einer und Zehner anzeigen */
  #define MODE_PM           0x0002              /* Country-Info aus INI-Dateien holen  */

  PCHAR   _System MakeEnv (PCHAR);
  BOOL    _System EAWriteASCII (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PCHAR pszString);
  BOOL    _System EAReadASCII  (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PCHAR pszString, PUSHORT puscValue);
  BOOL    _System EAWrite (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue);
  BOOL    _System EARead  (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue);
  BOOL    _System EAWriteMV (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, USHORT usEAType, PSTRUC_EAT_DATA pstrucValue);
  BOOL    _System EAReadMV  (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, USHORT usEAType, PSTRUC_EAT_DATA pstrucValue);
  BOOL    _System EAWriteRaw (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue);
  BOOL    _System EAReadRaw (ULONG ulRefType, PVOID pvFile, PCHAR pszEAName, PSTRUC_EAT_DATA pstrucValue);
  BOOL    _System History (PCHAR, PCHAR, PCHAR);
  BOOL    _System FileExist (PCHAR);
  APIRET  _System DelDir (PCHAR);
  APIRET  _System GetDateTime (PDATETIME, ULONG, PCHAR, PCHAR);
  HMODULE _System LoadLanguageModule (PSZ pszModuleName);
  BOOL    _System DetermineHelpFileName (PSZ pszHelpFileName);

#endif /* __CUTIL__ */

