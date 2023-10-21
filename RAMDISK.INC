;       *************************
;       *                       *
;       * Praecitronic RAM-Disk *
;       *                       *
;       *************************
;
;27.11.2022
;
;Die RAM-Disk hat eine Organisation von 128 Tracks
;und 64 Sectoren/Track (8K / Track).
;Track 0 ist für das BIOS reserviert (Systemtrack).
;
RDADR   EQU  0E0H         ;Grund-Adresse der RAM-Disk
;
RDSIZE: DEFB 0            ;RD-Size: 0=256K 1=512K 2=1024K
RDMERK: DEFS 3,0          ;3 Merkzellen fuer den RAM-Test
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
RDARI:  LD   A,(TRACK)    ;0TTTTTTT
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
        ADD  HL,HL        ;000TTTTT TT000000
        ADD  HL,DE        ;000TTTTT TTSSSSSS
        RR   H            ;0000TTTT
        RR   L            ;TTTSSSSS
        RR   A            ;S0000000
        OUT  (RDADR+7),A  ;NWT
        LD   A,L
        OUT  (RDADR+6),A  ;HWT
        LD   A,H
        AND  1FH
        CP   10H
        JR   NC,BDERR     ;BDOS-Error
        LD   C,A          ;Akku sichern
        SRL  A
        SRL  A
        OUT  (RDADR+5),A  ;XWT
        LD   A,C          ;Akku zurueck
        AND  3
        OR   RDADR        ;Grund-Adresse der RAM-Disk
        LD   C,A
        LD   B,128
        LD   HL,(DMA)
        RET
;
; BDOS-Fehler
; -----------
;
BDERR:  LD   A,1
        RET
;
; RAM-DISK formatieren
; --------------------
;
INITRD: LD   HL,RDTXT     ;RAM-Disk formatieren?
        CALL OUTTXT
        CALL CONIN
        AND  0DFH         ;UPCASE
        CP   "J"
        JR   Z,INITR1
        LD   HL,NOTXT     ;nein
        CALL OUTTXT
        RET
;
INITR1: LD   HL,YESTXT    ;ja
        CALL OUTTXT
        LD   HL,(DMA)     ;DMA mit 0E5H auffüllen
        LD   (HL),0E5H
        LD   D,H
        LD   E,L
        INC  DE
        LD   BC,07FH      ;128 Byte
        LDIR
        LD   A,64         ;64 Sektoren
        LD   (SECTOR),A
        LD   A,(RDSIZE)   ;RD-Size: 0=256K 1=512K 2=1024K
        LD   B,A
        INC  B
INITR2: LD   A,16         ;16 Tracks = 128K
        SLA  A            ;A = A * 2
        DJNZ INITR2       ;1-3 Durchlaeufe, je nach RDSIZE
        LD   HL,TRACK
        LD   (HL),A
INITR3: DEC  (HL)         ;Track--
        PUSH HL
INITR4: CALL WRITE
        LD   HL,SECTOR
        DEC  (HL)         ;Sektor--
        JR   NZ,INITR4
        LD   (HL),64      ;64 Sektoren
        POP  HL
        DEC  (HL)
        INC  (HL)         ;Track = 0 ?
        JR   NZ,INITR3
        CALL CCPSAV       ;CCP + BDOS --> RAM-Disk
        LD   HL,0080H
        LD   (DMA),HL
        LD   HL,INITOK
        CALL OUTTXT
        RET
;
; CCP + BDOS speichern
; --------------------
;
CCPSAV: XOR  A
        LD   (TRACK),A
        INC  A
        LD   (SECTOR),A
        LD   HL,CCP       ;CCP
CCPS1:  LD   (DMA),HL
        PUSH HL
        CALL WRRDSK       ;RAM-Disk schreiben
        LD   HL,SECTOR
        INC  (HL)         ;Sektor++
        POP  HL
        LD   BC,128
        ADD  HL,BC
        LD   A,0E5H
        CP   H
        JR   NC,CCPS1     ;0E600H noch nicht erreicht
        RET
;
; CCP + BDOS laden
; ----------------
;
CCPLD:  XOR  A
        LD   (TRACK),A
        INC  A
        LD   (SECTOR),A
        LD   HL,CCP       ;CCP
CCPL1:  LD   (DMA),HL
        PUSH HL
        CALL RDRDSK       ;RAM-Disk lesen
        LD   HL,SECTOR
        INC  (HL)         ;Sektor++
        POP  HL
        LD   BC,128
        ADD  HL,BC
        LD   A,0E5H
        CP   H
        JR   NC,CCPL1     ;0E600H noch nicht erreicht
        RET
;
; Test der RAM-Disk - Größe (256 / 512 / 1024 KByte)
; -------------------------
;
RDTEST: XOR  A
        OUT  (RDADR+6),A  ;H-Adresse ruecksetzen
        LD   HL,RDMERK    ;Merkzellen RAM
        LD   B,3          ;3 Durchlaeufe
        LD   C,RDADR      ;Test erfolgt in der 1. RAM-Bank
        LD   D,0          ;Startwert Extended-Adresse
RDTST1: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        IN   A,(C)
        LD   (HL),A       ;Daten aus der RAM-Disk sichern
        XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (C),A        ;Testwert in die RAM-Disk schreiben
        INC  HL           ;Merkzelle++
        INC  D            ;Extended-Adresse++ / Testwert++
        DJNZ RDTST1
;
;Tabelle der Test-Werte (mit *...* sind die zu testenden Werte):
;
;     RAM-Disk-Size =   256K   512K   1024K
;Extended-Adresse=00B    02H    02H   *00H*
;Extended-Adresse=01B    02H   *01H*   01H
;Extended-Adresse=10B   *02H*   02H    02H
;
        LD   B,3          ;3 Durchlaeufe
        LD   D,0          ;Startwert Extended-Adresse
RDTST2: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        IN   A,(C)
        CP   D
        JR   Z,RDTST3     ;1024K (B=3), 512K (B=2), 256K (B=1)
        INC  D            ;Extended-Adresse++ / Testwert++
        DJNZ RDTST2
        JP   DERROR       ;Fehler: hier stimmt was nicht!
;
RDTST3: LD   HL,RDSIZE
        DEC  B
        LD   (HL),B       ;RD-Size: 0=256K 1=512K 2=1024K
;
        DEC  B
        JP   Z,DERROR     ;RD-SIZE=512K --> Fehlermeldung
        INC  B
;
        LD   HL,RDMERK    ;Merkzellen RAM (in die RD zurueck)
        INC  B            ;1-3 Durchlaeufe, je nach RDSIZE
        LD   C,RDADR      ;1. RAM-Bank
        LD   D,0          ;Startwert Extended-Adresse
RDTST4: XOR  A
        OUT  (RDADR+7),A  ;L-Adresse ruecksetzen
        LD   A,D
        OUT  (RDADR+5),A  ;Extended-Adresse auswaehlen
        LD   A,(HL)
        OUT  (C),A        ;urspruengliche Daten zurueckschreiben
        INC  HL
        INC  D
        DJNZ RDTST4       ;WICHTIG: nicht mehr als notwendig
        RET               ;in die RAM-Disk zurückschreiben!!
;
RDTXT:  DEFM 'RAM-Disk formatieren? (J/N): '
        DEFB 00H
YESTXT: DEFM 'J'
        DEFB 0DH,0AH,00H
NOTXT:  DEFM 'N'
        DEFB 0DH,0AH,00H
INITOK: DEFM '---> Init. O.K.'
        DEFB 0DH,0AH,00H
;
