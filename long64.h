#ifndef LONG64_H
#define LONG64_H

typedef struct
    {
    ULONG ulLo;
    LONG  lHi;
    } LONG64, *PLONG64;

/* Arithmetic function prototypes */
LONG64 _System long64Add (const LONG64 *pa, const LONG64 *pb);
LONG64 _System long64Div (const LONG64 *pa, LONG b, PLONG pr);
char * _System long64toa (const LONG64 *val, char *buf, USHORT base);

#endif /* LONG64_H */
