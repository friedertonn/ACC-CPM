;       ***************************
;       *                         *
;       * CONOUT, CONIN und CONST *
;       *                         *
;       ***************************
;
;06.07.2022
;
CURSOR  EQU  5FH          ;Cursorsymbol "_"
;
CONOUT: DI
        LD   (SAVSTK),SP
        LD   SP,BIOSTK
        EI
        PUSH AF
        PUSH BC
        PUSH DE
        PUSH HL
        LD   A,AC1MOD
        OUT  (MAP),A
        LD   HL,(CURSP)
        LD   A,(MERKUR)
        LD   (HL),A
        XOR  A
        LD   HL,ESCCNT    ;Escapesequenz ?
        OR   (HL)
        JR   Z,VERGL      ;Nein
        RES  7,C
        DEC  (HL)
        JR   Z,DIPOS      ;C --> Spaltennummer
        DEC  HL
        LD   (HL),C       ;C --> Zeilennummer
        JP   COEND
;
DIPOS:  DEC  HL
        LD   A,(HL)       ;Zeilennummer
        AND  1FH
        LD   E,A
        LD   D,0          ;Zeilennummer --> DE
        LD   A,C          ;Spaltennummer
        AND  3FH
        LD   L,A
        LD   H,0          ;Spaltennummer --> HL
        LD   B,6
DIPOS1: SLA  E
        RL   D
        DJNZ DIPOS1
        ADD  HL,DE
        EX   DE,HL
        LD   HL,17FFH
        AND  A
        SBC  HL,DE
        LD   (CURSP),HL
        JP   COEND
;
VERGL:  LD   HL,(CURSP)
        LD   A,C
        CP   82H
        JP   Z,CUON       ;Cursor einschalten
        CP   83H
        JP   Z,CUOFF      ;Cursor abschalten
        RES  7,A
        CP   1BH
        JR   Z,ESCAPE     ;ESC
        CP   1
        JR   Z,CUHOME     ;HOME
        CP   7
        JR   Z,BEEP       ;BEEP
        CP   0AH
        JR   Z,LF         ;LINEFEED
        CP   0DH
        JR   Z,CR         ;CR
        CP   15H
        JR   Z,CURECH     ;Cursor nach rechts
        CP   16H
        JP   Z,DELEND     ;loescht Cu. -> ZE
        CP   18H
        JP   Z,DELLIN     ;loescht zeile,Cu <-
        CP   1AH
        JR   Z,CURSUP     ;Cursor eine Z. hoch
        CP   7FH
        JR   Z,DEL        ;DELETE
        CP   14H
        JR   Z,CLRTES     ;Clear to end screen
        CP   0CH
        JR   Z,CLS        ;CLS
        CP   8
        JR   Z,BS         ;BACKSPACE
        CP   20H
        JP   C,COEND
        JP   OUTZEI       ;Ausgabe auf BS
;
CR:     LD   A,L
        OR   3FH
        LD   L,A
        JP   COEND1
;
BS:     INC  HL
        JR   UP1
;
CLS:    LD   A,20H
        LD   HL,1000H
        LD   DE,1001H
        LD   BC,07FFH
        LD   (HL),A
        LDIR
        LD   (CURSP),HL
        JP   COEND
;
ESCAPE: LD   A,2          ;2 Argumente folgen
        LD   (ESCCNT),A
        JP   COEND
;
CUHOME: LD   HL,17FFH
        LD   (CURSP),HL
        JP   COEND
;
BEEP:   LD   C,0FFH
BEEP0:  LD   B,90H
BEEP1:  DJNZ BEEP1
        IN   A,(PIO1BD)
        XOR  41H          ;PB0 und PB6 ---> Piep
        OUT  (PIO1BD),A
        DEC  C
        JR   NZ,BEEP0
        JP   COEND
;
LF:     LD   DE,40H
        AND  A
        SBC  HL,DE
        JR   COEND1
;
CURECH: DEC  HL
        JR   COEND1
;
DEL:    INC  HL
        LD   A,18H
        CP   H
        JR   Z,COEND
        LD   (HL),20H
        JR   COEND1
;
CURSUP: LD   DE,40H
        ADD  HL,DE
UP1:    LD   A,18H
        CP   H
        JR   Z,COEND
        LD   (CURSP),HL
        JR   COEND
;
CLRTES: LD   A,0FH
SPACE1: LD   (HL),20H
        DEC  HL
        CP   H
        JR   NZ,SPACE1
        JR   COEND
;
DELEND: LD   A,L
        AND  3FH
        LD   B,A
        INC  B
SPACE2: LD   (HL),20H
        DEC  HL
        DJNZ SPACE2
        JR   COEND
;
DELLIN: LD   A,L
        OR   3FH
        LD   L,A
        LD   (CURSP),HL
        LD   B,40H
        JR   SPACE2
;
CUON:   LD   A,36H        ;LD (HL),...
        LD   HL,ON1
        LD   (HL),A
        INC  HL
        LD   (HL),CURSOR
        LD   HL,COIN1
        LD   (HL),A
        INC  HL
        LD   (HL),CURSOR
        JR   COEND
;
CUOFF:  LD   HL,0
        LD   (ON1),HL
        LD   (COIN1),HL
        JR   COEND
;
OUTZEI: LD   (HL),A
        DEC  HL
COEND1: EX   DE,HL
        LD   HL,0FFFH
        AND  A
        SBC  HL,DE
        EX   DE,HL
        LD   (CURSP),HL
        JR   C,COEND
        LD   HL,17BFH
        LD   DE,17FFH
        LD   BC,07C0H
        LDDR              ;Bild rollen
        LD   (CURSP),DE
        EX   DE,HL
        INC  L
SPACE3: DEC  L
        LD   (HL),20H
        JR   NZ,SPACE3
COEND:  LD   HL,(CURSP)
        LD   A,(HL)
        LD   (MERKUR),A
ON1:    LD   (HL),CURSOR
        LD   A,CPMMOD
        OUT  (MAP),A
        POP  HL
        POP  DE
        POP  BC
        POP  AF
        DI
        LD   SP,(SAVSTK)
        EI
        RET
;
CONIN:  DI
        LD   (SAVSTK),SP
        LD   SP,BIOSTK
        EI
        LD   A,AC1MOD
        OUT  (MAP),A
        PUSH BC
        PUSH DE
        PUSH HL
        LD   HL,(CURSP)
COIN1:  LD   (HL),CURSOR
COIN2:  LD   BC,6000H     ;Kursorblinkfrequenz
COIN3:  LD   A,(TZ)       ;Tastaturzaehler?
        OR   A
        JR   NZ,KEY       ;Taste gedrueckt
        DEC  BC
        LD   A,B
        OR   C
        JR   NZ,COIN3     ;Warteschleife
        LD   A,CURSOR
        CP   (HL)
        JR   NZ,COIN1
        LD   A,(MERKUR)
        LD   (HL),A
        JR   COIN2
;
KEY:    LD   A,(MERKUR)   ;Taste gedrueckt
        LD   (HL),A       ;Cursor weg
        LD   HL,TZ+2
        LD   D,(HL)
        DEC  HL
        LD   E,(HL)
        INC  (HL)
        RES  3,(HL)
        DEC  HL
        DEC  (HL)         ;Tastaturzaehler
        LD   A,CPMMOD
        OUT  (MAP),A
        LD   A,(DE)       ;Tastencode
        POP  HL
        POP  DE
        POP  BC
        DI
        LD   SP,(SAVSTK)
        EI
        RET
;
CONST:  DI
        LD   (SAVSTK),SP
        LD   SP,BIOSTK
        EI
        LD   A,(TZ)       ;Tastaturzaehler?
        OR   A
        JR   Z,CONEND     ;keine Taste
        LD   A,0FFH
CONEND: DI
        LD   SP,(SAVSTK)
        EI
        RET
;
INTA:   DI
        LD   (INSTK),SP
        LD   SP,INSTK1
        PUSH AF
        PUSH BC
        PUSH HL
        IN   A,(PIO1AD)
        OR   A            ;Taste gedrueckt?
        JR   Z,INTAE
        LD   HL,TZ
        BIT  3,(HL)       ;TAPU - Ueberlauf?
        JR   NZ,INTAE
        RES  7,A
        INC  (HL)
        LD   HL,(TZIN)
        LD   (HL),A
        INC  HL
        RES  3,L
        LD   (TZIN),HL
        LD   BC,1E0BH     ;Zeitschleife 100 ms
ZS1:    DEC  BC
        LD   A,B
        OR   C
        JR   NZ,ZS1
        LD   A,0B7H       ;weitere Interrupts loeschen
        OUT  (PIO1AC),A   ;---> Entprellung
        LD   A,7FH
        OUT  (PIO1AC),A
INTAE:  POP  HL
        POP  BC
        POP  AF
        LD   SP,(INSTK)
        EI
        RETI
;
INSTK:  DEFS 40,0         ;Stack fuer Int.-Entry
INSTK1:
;
CURSP:  DEFW 17FFH        ;Kursorposition
        DEFB 0
ESCCNT: DEFB 0
MERKUR: DEFS 1,0
;
