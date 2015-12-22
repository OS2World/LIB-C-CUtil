
#ifndef __CRYPT__
  #define __CRYPT__

#ifdef __cplusplus
    extern "C" {
#endif

  #define REGCHECK_FAILED   0           /* falsches Registrier-Kennwort */
  #define REGCHECK_SW       1           /* Shareware */
  #define REGCHECK_WV       2           /* Registrierkennwort abgelaufen */
  #define REGCHECK_OK       3           /* Registrierung ok */

  typedef enum {CA_Crypt, CA_Decrypt} CryptAction;

  ULONG _System RegCheck      (PCHAR, PCHAR, PUSHORT, BYTE, BYTE);
  VOID  _System Chiffre       (PCHAR, ULONG, ULONG);
  VOID  _System DeChiffre     (PCHAR, PULONG, PULONG);
  VOID  _System DES           (PULONG, PULONG, ULONG, ULONG);
  VOID  _System DeDES         (PULONG, PULONG, ULONG, ULONG);
  VOID  _System DESstep       (PULONG, PULONG, ULONG, ULONG, ULONG);
  VOID  _System DeDESstep     (PULONG, PULONG, ULONG, ULONG, ULONG);
  ULONG _System fDES          (ULONG, ULONG);
  ULONG _System fDeDES        (ULONG, ULONG);
  PCHAR _System ChiffreSZ     (PCHAR psz);

  VOID  _System encrypt       (PCHAR, PCHAR, BYTE, BYTE, USHORT);
  BOOL  _System decrypt       (PCHAR, PCHAR, PBYTE, PBYTE, PUSHORT);
  BOOL  _System cryptString   (PCHAR pBuffer, ULONG ulcBuffer, CryptAction action);

  #ifdef DOCRYPT
  #define CRYPT_STRING(p,c,a)   cryptString (p, c, a)
  #else
  #define CRYPT_STRING(p,c,a)
  #endif

  #ifndef ULKEY1
      #define ULKEY1  0
  #endif
  #ifndef ULKEY2
      #define ULKEY2  0
  #endif

#ifdef __cplusplus
    }
#endif

#endif /* __CRYPT__ */

