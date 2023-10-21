;       ***********************
;       *                     *
;       * Floppy - Controller *
;       *                     *
;       ***********************
;
;16.10.2023
;
;Sectorberechnung Floppy
;-----------------------
;IN:  SECTOR aktueller BDOS-Sector
;     SECTR letzter Disk-Sector
;OUT: SECTR aktueller Disk-Sector
;     DE DMA-Zeiger
;     Z=0 neuer Disksector
;     Z=1 Sector weiter bearbeiten
;
CFDC    EQU  40H
DFDC    EQU  41H
DL175   EQU  44H          ;ehem. 48H
AKWAIT  EQU  43H          ;ehem. 50H
;
CPSEC:  LD   HL,SECTOR
        PUSH HL
        LD   A,(N)        ;Diskettenformat
        LD   C,A
        LD   B,A          ;Maske errechnen
        XOR  A
CPSEC1: SCF
        RLA
        DJNZ CPSEC1
        DEC  (HL)
        AND  (HL)         ;n. Sec. im Diskblock
        LD   HL,FDMA-80H  ;FDMA-Zeiger ber.
        LD   DE,80H
        LD   B,A          ;n. Sector im FDMA
        INC  B
CPSEC2: ADD  HL,DE
        DJNZ CPSEC2
        EX   DE,HL
        LD   B,C          ;(N)
        POP  HL           ;SECTOR
        LD   A,(HL)
        INC  (HL)
CPSEC3: SRL  A
        DJNZ CPSEC3       ;B=0 (Kopf1)
;
;Kopf berechnen
;--------------
;
        LD   C,A
        LD   A,(EOT)      ;DSectoren je Spur
        DEC  A
        LD   HL,SECTR
        SUB  C
        JR   NC,INHEAD    ;Kopf 1
        CPL               ;bitweise negieren
        LD   B,1          ;Kopf 2
        LD   C,A
INHEAD: LD   A,C
        DEC  (HL)
        CP   (HL)
        PUSH AF
        LD   (HL),C       ;DSector eintragen
        INC  (HL)         ;Sec. rel. zu 1
        DEC  HL
        LD   (HL),B       ;Kopf eintragen
        SLA  B
        SLA  B
        DEC  HL
        DEC  HL           ;UNIT
        LD   (HL),B       ;Kopf n, LW 0
        POP  AF
        RET
;
;Vergleich letzter LW-Zugriff
;----------------------------
;
CPLW:   LD   HL,CTAB      ;Vergl. Operation
        CP   (HL)
        JR   NZ,CPLW1
CPLW3:  LD   HL,CTAB
        LD   DE,RESLT     ;Vergleich LW
        LD   A,(DE)
        INC  HL
        XOR  (HL)
        AND  3            ;nur Bit 0 und 1 vergl.
        JR   NZ,CPLW1
        INC  HL           ;Vergleich Spur
        INC  DE
        INC  DE
        INC  DE
        LD   A,(RESLT+5)  ;Sector+1
        DEC  A            ;1. Sec. der next Spur?
        LD   A,(DE)
        JR   NZ,CPLW2
        DEC  A            ;1 Spur zurueck
CPLW2:  CP   (HL)
        JR   NZ,CPLW1
        INC  HL
        INC  DE           ;Vergleich Kopf
        LD   A,(DE)
        CP   (HL)
        RET  Z
CPLW1:  INC  B
        RET
;
;Sector schreiben
;----------------
;
WRFLO:  DI
        LD   (SAVSTK),SP
        LD   SP,BIOSTK
        PUSH HL
        PUSH DE
        PUSH BC
        PUSH BC           ;C=1->Direktory schreiben
        XOR  A            ;Fehlermeldung loeschen
        LD   (RWTEST),A
        CALL CPSEC        ;Vergleich Sector
        PUSH DE           ;Position im HST-Puffer
        LD   B,0
        JR   Z,WRITE1     ;gleicher Sector
        INC  B
WRITE1: CALL CPLW3        ;restl. Verleich
        LD   A,(BLTEST)
        OR   A
        JR   Z,WRITE2     ;nicht schr. aber lesen
        LD   A,B
        OR   A            ;neuer HST-Sector?
        JR   Z,WRITE3     ;weder lesen noch schr.
        CALL WRITE4       ;alten HST-Sector schr.
WRITE2: CALL R8272        ;neuen HST-Sector lesen
        LD   HL,CTAB      ;Werte merken
        LD   DE,CTAB2
        LD   BC,9         ;9 Kommandobytes
        LDIR
        LD   A,1
        LD   (BLTEST),A   ;Kennung ruecksetzen
WRITE3: POP  DE           ;Pos. im HST-Sector
        LD   HL,(DMA)
        LD   BC,80H
        LDIR
        POP  AF           ;C=1 bei UP-Aufruf?
        CALL C,WRITE4
        LD   A,(RWTEST)   ;fuer Fehlertest
        POP  BC
        POP  DE
        POP  HL
        LD   SP,(SAVSTK)
        EI
        RET
;
;HST-Sector schreiben
;--------------------
;
WRITE4: LD   A,(BLTEST)   ;Kennungstest
        OR   A
        RET  Z            ;nicht schreiben
        LD   HL,CTAB2     ;Werte letzter HST-Sec.
        LD   (FDCCOM),HL
        CALL W8272        ;phys. Schreiben
        LD   HL,BLTEST
        LD   (HL),0       ;Kennung setzen
        LD   HL,CTAB      ;mit neuen Werten
        LD   (FDCCOM),HL  ;weiterarbeiten
        RET
;
;Sector lesen
;------------
;
RDFLO:  DI
        LD   (SAVSTK),SP
        LD   SP,BIOSTK
        PUSH HL
        PUSH DE
        PUSH BC
        CALL WRITE4       ;HST-Sec. schr. (A=0 bei
        LD   (RWTEST),A   ;fehlerfreiem schreiben)
        CALL CPSEC
        PUSH DE
        LD   B,0
        JR   Z,READ1
        INC  B
READ1:  LD   A,46H
        CALL CPLW
        LD   A,B
        OR   A
        CALL NZ,R8272
        POP  HL
        LD   DE,(DMA)
        LD   BC,80H
        LDIR
        LD   A,(RWTEST)
        POP  BC
        POP  DE
        POP  HL
        LD   SP,(SAVSTK)
        EI
        RET
;
;Motor einschalten
;-----------------
;
MOTON:  PUSH HL
        LD   HL,STREG
        LD   A,(BLTEST)   ;Kennungstest
        OR   A
        JR   Z,MOTON1     ;(neuen) Hostsector lesen
        LD   A,(CTAB2+1)  ;(alten) Hostsector schreiben
MOTON1: SET  0,(HL)
        LD   A,(HL)
        OUT  (DL175),A
        POP  HL
ONLOOP: CALL REGDEL
        LD   BC,0204H     ;LW-Status
        CALL WCOM
        CALL DELAY
        IN   A,(DFDC)     ;ST3
        BIT  5,A          ;LW ready?
        JR   Z,ONLOOP     ;Drehzahl zu klein
        RET
;
;Motor ausschalten
;-----------------
;
MOTOFF: PUSH HL
        LD   HL,STREG
        RES  0,(HL)
        LD   A,(HL)
        OUT  (DL175),A
        POP  HL
        RET
;
;Fehlerbehandlung
;----------------
;
DERR1:  LD   A,(STREG)
        SET  5,A
        RES  0,A          ;Motor aus
        OUT  (DL175),A
        RET
;
;Laufwerksstatus pruefen
;-----------------------
;Kommando:04H+UNIT
;
;OUT:Statusregister 3
;(FAULT,WP,RDY,TO,TS,HD,US1,US0)
;
SDS:    LD   BC,0204H
        CALL WCOM         ;Kommando schreiben
        CALL RBYTE        ;1 Byte lesen
        RET
;
;Spur einstellen
;---------------
;Kommando:0FH+UNIT+TRACK
;
SEEK:   LD   A,(TRACK)    ;>Spur 80?
        CP   80H
        JP   NC,DERROR
        LD   BC,030FH
        CALL RDY
        CALL SENSE        ;Interruptst. pruefen
SKBSY:  IN   A,(CFDC)
        AND  0FH
        JR   NZ,SKBSY
        BIT  4,B          ;>77 Spuren?
        JR   NZ,SEEK
        RET
;
;Kommando schreiben
;------------------
;
RDY:    PUSH BC           ;LW betriebsfaehig?
        CALL SDS          ;LW Status?
        POP  BC
        BIT  5,A          ;Ready-Bit in Statreg.3
        JP   Z,DERROR     ;Fehlermeldung
WCOM:   LD   HL,(FDCCOM)  ;Statustabelle
;
;IN: B - Anzahl der Kommandos
;    C - 1. Kommando
;
WCOM1:  CALL DELAY
        IN   A,(CFDC)
        AND  0C0H
        CP   80H          ;RQM, DIO=OUT
        JR   NZ,WCOM1
        LD   A,C
        OUT  (DFDC),A     ;Kommandoausgabe
        INC  HL
        LD   C,(HL)
        DJNZ WCOM1
        RET
;
;Verzoegerung fuer Statusflag 8272
;---------------------------------
;
DELAY:  PUSH BC
        LD   B,0FH
DEL1:   DJNZ DEL1
        POP  BC
        RET
;
;1 Byte lesen
;------------
;
RBYTE:  CALL DELAY
        CALL IRDY
        IN   A,(DFDC)
        RET
;
;Lese 7 Resultat-Bytes
;---------------------
;
RRSLT:  LD   B,7
        LD   C,DFDC
        LD   HL,RESLT
        PUSH HL
RRSLT1: IN   A,(CFDC)
        RLCA
        JP   NC,RRSLT1
        INI
        RLCA
        JP   NC,DERROR
        JP   NZ,RRSLT1
        POP  HL
        LD   A,(HL)
        AND  0C0H
        RET
;
;Bereit fuer Dateneingabe?
;-------------------------
;
IRDY:   IN   A,(CFDC)
        RLCA
        JR   NC,IRDY
        AND  80H
        RLCA
        RET  C            ;hier war frueher Schluss
        CALL MOTOFF
        JP   DERROR
;
;Initialisierungstabelle
;-----------------------
;
STAB:   DEFB 0E1H         ;XXXX-SRT, XXXX-HUT
        DEFB 33H          ;XXXXXXX-HLT, X-ND
;
;Schreiben oder Lesen E*256 Bytes
;--------------------------------
;wird in RAM umgeladen
;
RW:     LD   B,0
        LD   A,(N2)
        LD   E,A
        LD   A,(STREG)
        SET  1,A
        OUT  (DL175),A    ;WAIT-Freigabe
RW1:    IN   A,(CFDC)
        RLCA
        JP   NC,RW1
MODE0:  INI
        NOP
RW2:    OUT  (AKWAIT),A   ;Aktivierung Wait
        NOP
MODE1:  INI
        JP   NZ,RW2
        DEC  E
        JP   NZ,RW2
        LD   A,(STREG)
        OUT  (DL175),A    ;WAIT-Sperrung
        RET
;
;Resultatregister bis Fertigmeldung lesen
;----------------------------------------
;
REGDEL: LD   B,0
ZEIT1:  DJNZ ZEIT1
        IN   A,(CFDC)
        CP   80H
        RET  Z
        IN   A,(DFDC)
        JR   REGDEL
;
;Treiberinitialisierung U8272 + Laufwerke
;----------------------------------------
;
INITFD: CALL MOTON
        LD   HL,STAB-1    ;Parameter laden
        LD   BC,0303H     ;3 Kommandos
        CALL WCOM1        ;03H+0E1H+33H
RECAL:  LD   BC,0207H     ;Spur 0 einstellen
        CALL RDY          ;07H+UNIT --> Floppy-LW
        CALL SENSE
        BIT  4,B          ;Spur 0 erreicht?
        JR   NZ,RECAL
        CALL MOTOFF
        RET
;
;Pruefe Interruptstatus
;----------------------
;
SENSE:  LD   BC,0108H
        CALL WCOM         ;Kommando schreiben
        CALL RBYTE        ;Resultatregister 0
        LD   B,A
        CP   80H
        CALL NZ,RBYTE     ;PCN holen
        BIT  5,B          ;SEEK Ende?
        JR   Z,SENSE
        RET
;
;Sector schreiben
;----------------
;Sector in (SECTR)
;Spur in (TRACK)
;akt. LW in (UNIT)
;Ziel-/Quelladr. = FDMA
;
W8272:  DEFB 11H          ;LD DE,...
        OUTI
        LD   A,45H        ;Schreibkommando
        JR   RWIT
;
;Sector lesen
;------------
;
R8272:  DEFB 11H          ;LD DE,...
        INI
        LD   A,46H        ;Lesekommando
RWIT:   LD   (CTAB),A
        LD   (MODE0),DE
        LD   (MODE1),DE
        CALL MOTON
        CALL SEEK
        LD   B,10         ;10 Versuche
RWOP:   PUSH BC
        LD   B,9          ;9 Kommandobytes
        LD   A,(CTAB)
        LD   C,A
        CALL RDY          ;Kommandoausgabe
        LD   HL,FDMA
        LD   C,DFDC       ;Kanal
        CALL RW
;
;Ende-Impuls
;
        LD   A,(STREG)
        SET  4,A
        OUT  (DL175),A
        CALL RRSLT
        POP  BC
        JR   Z,RWEND
        DJNZ RWOP
        LD   A,1          ;ERROR
RWEND:  LD   HL,RWTEST
        OR   (HL)
        LD   (HL),A
        CALL MOTOFF
        RET
;
SECTOR: DEFB 1
DMA:    DEFW 80H
FDCCOM: DEFW CTAB
STREG:  DEFB 0            ;fuer DL175
CTAB:   DEFB 0FFH
UNIT:   DEFB 0            ;akt. LW
TRACK:  DEFB 0
HEAD:   DEFB 0
SECTR:  DEFB 1
N:      DEFB 3            ;N=3 --> 1Kbyte
EOT:    DEFB 5            ;5 Sectoren je Spur
GPL:    DEFB 32H          ;Luecke
DTL:    DEFB 0FFH
N2:     DEFB 4
RESLT:  DEFS 7,0          ;Resultattabelle f. FDC
CTAB2:  DEFS 9,0
BLTEST: DEFS 1,0          ;Host-Sector schreiben
RWTEST: DEFS 1,0          ;fuer Fehlermeldungen
FDMA:   DEFS 400H,0       ;Host-Sector
;
