;@C       /****************************************************************************\
;@C        *                                                                          *
;@C       \****************************************************************************/
          TITLE     long64.c
          .586p
          .model    flat, syscall


LONG64    struct
ulLo      DWORD     ?
lHi       DWORD     ?
LONG64    ends

LONG32    struct
ulLo      WORD      ?
lHi       WORD      ?
LONG32    ends

PLONG64   typedef   PTR LONG64
PLONG32   typedef   PTR LONG32

CODE32    SEGMENT   dword use32 PUBLIC 'CODE'
CODE32    ENDS

DATA32    SEGMENT   dword use32 PUBLIC 'DATA'
DATA32    ENDS

CODE32    SEGMENT
          ASSUME    CS: CODE32, DS: DATA32, SS: DATA32

;-----    LONG64 _System long64Add (const LONG64 *pa, const LONG64 *pb)
long64Add PROC      NEAR32 SYSCALL PUBLIC USES ESI,
                    retVal: PLONG64,
                    pa: PLONG64,
                    pb: PLONG64

          lea       ESI, pa                       ; load *pa into EBX:EAX
          mov       ESI, [ESI]
          mov       EAX, [ESI] + LONG64.ulLo
          mov       EBX, [ESI] + LONG64.lHi

          lea       ESI, pb                       ; add *pb to EBX:EAX
          mov       ESI, [ESI]
          add       EAX, [ESI] + LONG64.ulLo
          adc       EBX, [ESI] + LONG64.lHi

          lea       ESI, retVal                   ; write result to retVal
          mov       ESI, [ESI]
          mov       [ESI] + LONG64.ulLo, EAX
          mov       [ESI] + LONG64.lHi, EBX

          ret
long64Add ENDP

;-----    LONG64 _System long64Div (const LONG64 *pa, LONG b, PLONG pr)
long64Div PROC      NEAR32 SYSCALL PUBLIC USES ESI,
                    retVal: PLONG64,
                    pa: PLONG64,
                    b: DWORD,
                    pr: PTR DWORD

;-----    kkkkllll:mmmmnnnn = wwwwxxxx:yyyyzzzz : aaaabbbb, remainder ppppqqqq
          lea       ESI, pa                       ; load *pa
          mov       ESI, [ESI]
          mov       EBX, [ESI] + LONG64.ulLo      ; EBX contains yyyyzzzz
          mov       EAX, [ESI] + LONG64.lHi       ; EDX:EAX contains 00000000:wwwwxxxx
          xor       EDX, EDX

          lea       EDI, retVal                   ; pointer to result
          mov       EDI, [EDI]

;-----    1. step:        d = 00000000:wwwwxxxx : aaaabbbb, remainder r1
;         after division: EAX contains kkkkllll
;                         EDX contains remainder r1 = rrrrrrrr
;                      => retVal = kkkkllll:00000000
          idiv      b
          mov       [EDI] + LONG64.lHi, EAX

;-----    2. step:        EDX = rrrrrrrr,  EAX = yyyyzzzz
;         shift right:    EDX:EAX = 0000rrrr:rrrryyyy
;         divide:         d = 0000rrrr:rrrryyyy : aaaabbbb, remainder r2
;         after division: EAX contains 0000mmmm
;                         EDX contains remainder r2 = rrrrrrrr
;                      => retVal = kkkkllll:mmmm0000
          mov       EAX, EBX                      ; EAX = yyyyzzzz
          shrd      EAX, EDX, 16                  ; EDX:EAX = rrrr:rrrryyyy
          shr       EDX, 16
          idiv      b
          shl       EAX, 16
          mov       [EDI] + LONG64.ulLo, EAX

;-----    3. step:        EDX = rrrrrrrr, EAX = yyyyzzzz
;         shift right     EDX:EAX = 0000rrrr:rrrrzzzz
;         divide:         d = 0000rrrr:rrrrzzzz : aaaabbbb, remainder r3
;         after division: EAX contains 0000nnnn
;                         EDX contains remainder r3 = ppppqqqq
;                      => retVal = kkkkllll:mmmmnnnn
          mov       EAX, EBX
          shl       EAX, 16                       ; EAX = zzzz0000
          shrd      EAX, EDX, 16                  ; EDX:EAX = 0000rrrr:rrrrzzzz
          shr       EDX, 16
          idiv      b
          add       [EDI] + LONG64.ulLo, EAX

          lea       ESI, pr                       ; return remainder
          mov       ESI, [ESI]
          mov       [ESI], EDX

          ret
long64Div ENDP

CODE32    ENDS
          END
