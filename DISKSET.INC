;       ******************************
;       *                            *
;       * Diskettenformat einstellen *
;       *                            *
;       ******************************
;
;25.11.2022
;
DISK:   LD   A,(DSKSET)
        INC  A
        RES  1,A
        LD   (DSKSET),A   ;0=800K, 1=780K
        LD   HL,DPB800
        BIT  0,A
        JR   Z,DISK1
        LD   HL,DPB780
DISK1:  LD   DE,DPB1      ;Disk Parameter Block LW B:
        LD   BC,17
        LDIR
        LD   HL,DSK800    ;800K Floppy-Disk
        CP   0
        JR   Z,DISK2
        LD   HL,DSK780    ;780K Floppy-Disk
DISK2:  CALL OUTTXT
        JP   WBOOT        ;Warmstart
;
DSKSET: DEFB 1            ;beim Kaltstart: 780K
;
DPB800: DEFW 80           ;80 Sectoren/Track
        DEFB 4            ;Blockgroesse = 2 Kbyte
        DEFB 15
        DEFB 0
        DEFW 399          ;400 * 2 Kbyte - Bloecke
        DEFW 127          ;128 DIR - Eintraege
        DEFW 0C0H         ;2 DIR-Bloecke
        DEFW 32
        DEFW 0            ;0 Kbyte System
;
        DEFW 0
;
DPB780: DEFW 80           ;80 Sectoren/Track
        DEFB 4            ;Blockgroesse = 2 Kbyte
        DEFB 15
        DEFB 0
        DEFW 389          ;390 * 2 Kbyte - Bloecke
        DEFW 127          ;128 DIR - Eintraege
        DEFW 0C0H         ;pro DIR-Block 1Bit gesetzt
        DEFW 32
        DEFW 2            ;20 Kbyte System
;
        DEFW 0
;
DSK800: DEFB 0DH,0AH
        DEFM '---> Drive B:=800K'
        DEFB 0DH,0AH,00H
DSK780: DEFB 0DH,0AH
        DEFM '---> Drive B:=780K'
        DEFB 0DH,0AH,00H
;
