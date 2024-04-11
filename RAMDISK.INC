;       *************************
;       *                       *
;       * Praecitronic RAM-Disk *
;       *                       *
;       *************************
;
;10.04.2024
;
;Die RAM-Disk hat eine Organisation von 256 Tracks
;und 64 Sectoren/Track (8K / Track).
;Track 0 ist fuer CCP + BDOS reserviert (8K Systemtrack).
;
RDADR   EQU  0E0H         ;Grund-Adresse der RAM-Disk
;
RDMERK: DEFS 4,0          ;4 Merkzellen fuer den RAM-Disk Test
RDSIZE: DEFB 0            ;RD-Size: 0=256K 1=512K 2=1024K 3=2048K
;
; RAM-Disk lesen
; --------------
;
RDRDSK: CALL RDARI
        INIR
        XOR  A
        RET
;
; RAM-Disk schreiben
; ------------------
;
WRRDSK: CALL RDARI
        OTIR
        XOR  A
        RET
;
; Disk-Adresse bestimmen
; ----------------------
;
RDARI:  LD   A,(TRACK)    ;TTTTTTTT
        LD   L,A
        LD   A,(SECTOR)   ;00SSSSSS
        AND  A
        JP   Z,DERROR     ;Zugriff auf Sektor 0
        LD   E,A
        DEC  E            ;CP/A beginnt mit Sektor 1
        XOR  A
        LD   H,A
        LD   D,A
        ADD  HL,HL
        ADD  HL,HL
        ADD  HL,HL
        ADD  HL,HL
        ADD  HL,HL
        ADD  HL,HL        ;00TTTTTT TT000000
        ADD  HL,DE        ;00TTTTTT TTSSSSSS
        RR   H            ;000TTTTT
        RR   L            ;TTTSSSSS
        RR   A            ;S0000000
        OUT  (RDADR+7),A  ;NWT
        LD   A,L
        OUT  (RDADR+6),A  ;HWT
        LD   A,H
        LD   C,A          ;Akku sichern
        SRL  A            ;0 --> A7 --> A0 --> CY
        SRL  A
        OUT  (RDADR+5),A  ;XWT
        LD   A,C          ;Akku zurueck
        AND  3
        OR   RDADR        ;Grundadresse der RAM-Disk
        LD   C,A
        LD   B,128
        LD   HL,(DMA)
        RET
;
; RAM-DISK formatieren
; --------------------
;
INITRD: LD   HL,RDTXT     ;RAM-Disk formatieren?
        CALL OUTTXT
        CALL CONIN
        RES  5,A          ;UPCASE
        CP   "J"
        JR   Z,INITR1
        LD   HL,NOTXT     ;nein
        CALL OUTTXT
        RET
;
INITR1: LD   HL,YESTXT    ;ja
        CALL OUTTXT
        LD   HL,0080H
        LD   (DMA),HL
        LD   (HL),0E5H    ;DMA mit 0E5H auffuellen
        LD   DE,0081H
        LD   BC,07FH
        LDIR
        LD   A,64         ;64 Sektoren
        LD   (SECTOR),A
        LD   A,(RDSIZE)   ;RD-Size: 0=256K 1=512K 2=1024K 3=2048K
        INC  A
        LD   B,A          ;B-Reg.: 1=256K 2=512K 3=1024K 4=2048K
        LD   A,16         ;16 Tracks = 128K
INITR2: SLA  A            ;A = A * 2
        DJNZ INITR2       ;1-4 Durchlaeufe, je nach RDSIZE
        LD   HL,TICK      ;Ausgabe "-"
        CALL OUTTXT
        LD   HL,TRACK
        LD   (HL),A       ;Anzahl der Tracks, bei 2048K ist A=0
INITR3: DEC  (HL)         ;Track--
        LD   A,(HL)
        AND  31           ;jeder 32. Track, also alle 256K
        JR   NZ,INITR4
        LD   HL,TICK      ;Ausgabe "-"
        CALL OUTTXT
INITR4: CALL WRRDSK
        LD   HL,SECTOR
        DEC  (HL)         ;Sektor--
        JR   NZ,INITR4
        LD   (HL),64      ;64 Sektoren
        LD   HL,TRACK
        INC  (HL)
        DEC  (HL)         ;Track = 0 ?
        JR   NZ,INITR3
        IFDEF FLOPPY
        CALL CCPWR        ;CCP + BDOS --> RAM-Disk
        ENDIF
        LD   HL,INITOK
        CALL OUTTXT
        RET
;
; CCP + BDOS in Track 0/255 schreiben (44 Sektoren)
; -----------------------------------
;
IFDEF FLOPPY
CCPWR:  XOR  A
ELSE
CCPWR:  LD   A,255
ENDIF
        LD   (TRACK),A
        LD   A,1
        LD   (SECTOR),A
        LD   HL,CCP       ;CCP-Startadresse
CCPWR1: LD   (DMA),HL
        PUSH HL
        CALL WRRDSK       ;RAM-Disk schreiben
        LD   HL,SECTOR
        INC  (HL)         ;Sektor++
        POP  HL
        LD   BC,128
        ADD  HL,BC
IFDEF FLOPPY
        LD   A,0E5H
ELSE
        LD   A,0F5H
ENDIF
        CP   H
        JR   NC,CCPWR1    ;0E600H/0F600H noch nicht erreicht
        RET
;
; CCP + BDOS von Track 0/255 lesen (44 Sektoren)
; --------------------------------
;
IFDEF FLOPPY
CCPRD:  XOR  A
ELSE
CCPRD:  LD   A,255
ENDIF
        LD   (TRACK),A
        LD   A,1
        LD   (SECTOR),A
        LD   HL,CCP       ;CCP-Startadresse
CCPRD1: LD   (DMA),HL
        PUSH HL
        CALL RDRDSK       ;RAM-Disk lesen
        LD   HL,SECTOR
        INC  (HL)         ;Sektor++
        POP  HL
        LD   BC,128
        ADD  HL,BC
IFDEF FLOPPY
        LD   A,0E5H
ELSE
        LD   A,0F5H
ENDIF
        CP   H
        JR   NC,CCPRD1    ;0E600H/0F600H noch nicht erreicht
        RET
;
; Test der RAM-Disk - Size (256 / 512 / 1024 / 2048 KByte)
; ------------------------
;
RDTEST: XOR  A            ;Kapazitaet der RAM-Disk testen
        OUT  (RDADR+6),A  ;H-Adresse ruecksetzen
        LD   C,RDADR      ;Test erfolgt in der 1. RAM-Bank
;
; Inhalte der 4 RAM-Testzellen sichern
;
        LD   HL,RDMERK    ;Merkzellen im RAM
        LD   B,4          ;4 Durchlaeufe
        LD   D,4          ;Startwert Extended-Adresse
RDTST1: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        IN   A,(C)
        LD   (HL),A       ;Daten aus der RAM-Disk sichern
        INC  HL           ;Merkzelle++
        SRL  D            ;0 --> D7 --> D0 --> CY
        DJNZ RDTST1
;
; 4 Testwerte in die RAM-Disk schreiben
;
        LD   B,4          ;4 Durchlaeufe
        LD   D,4          ;Startwert Extended-Adresse
RDTST2: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        OUT  (C),A        ;Testwert in die RAM-Disk schreiben
        SRL  D            ;0 --> D7 --> D0 --> CY
        DJNZ RDTST2
;
;Tabelle der Test-Werte (mit *...* sind die zu testenden Werte):
;
;           RAM-Disk-Size =   256K   512K  1024K  2048K
;Extended-Adresse=00000100B    0      0      0     *4*
;Extended-Adresse=00000010B    0      0     *2*     2
;Extended-Adresse=00000001B    0     *1*     1      1
;Extended-Adresse=00000000B   *0*     0      0      0
;
; Groesse der RAM-Disk bestimmen
;
        LD   B,4          ;4 Durchlaeufe
        LD   D,4          ;Startwert Extended-Adresse
RDTST3: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        IN   A,(C)
        CP   D
        JR   Z,RDTST4     ;2048(B=4),1024(B=3),512(B=2),256(B=1)
        SRL  D            ;0 --> D7 --> D0 --> CY
        DJNZ RDTST3
        JP   DERROR       ;Fehler: hier stimmt was nicht!
;
RDTST4: LD   HL,RDSIZE
        DEC  B
        LD   (HL),B       ;RD-Size: 0=256K 1=512K 2=1024K 3=2048K
;
; gesicherte Daten in die RAM-Disk zurueckschreiben
; 
        LD   HL,RDMERK+3  ;Merkzellen RAM (letzter Eintrag)
        INC  B            ;1 bis 4 Durchlaeufe, je nach RDSIZE
        LD   D,1          ;Startwert Ext.-Adr. (1 Bit n. rechts)
RDTST5: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        SRL  A            ;0 --> A7 --> A0 --> CY
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        LD   A,(HL)
        OUT  (C),A        ;urspruengliche Daten zurueckschreiben
        DEC  HL
        SLA  D            ;CY <-- D7 <-- D0 <-- 0
        DJNZ RDTST5       ;WICHTIG: nicht mehr als notwendig
        RET               ;in die RAM-Disk zurueckschreiben!
;
RDTXT:  DEFM 'RAM-Disk formatieren? (J/N): '
        DEFB 00H
YESTXT: DEFM 'J'
        DEFB 0DH,0AH,00H
NOTXT:  DEFM 'N'
        DEFB 0DH,0AH,00H
TICK:   DEFB 2DH,00H
INITOK: DEFM '> Init. O.K.'
        DEFB 0DH,0AH,00H
;
