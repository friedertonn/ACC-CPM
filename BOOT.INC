;       ****************
;       *              *
;       * BOOT & WBOOT *
;       *              *
;       ****************
;
;16.10.2023
;
BOOT:   DI
        LD   SP,80H
        IM   2            ;Bitmode Interrupt
        LD   A,HIGH(IV)
        LD   I,A
        LD   A,LOW(IV)    ;Interruptvektor PIO 1A
        OUT  (PIO1AC),A
        LD   A,0CFH       ;PIO - Mode 3
        OUT  (PIO1AC),A
        LD   A,0FFH       ;alles Eingaenge
        OUT  (PIO1AC),A
        LD   A,0B7H       ;Int.-st.-wort
        OUT  (PIO1AC),A
        LD   A,7FH        ;Int.-maske
        OUT  (PIO1AC),A
        XOR  A            ;Interruptvektor PIO 1B
        OUT  (PIO1BC),A
        LD   A,0CFH       ;PIO - Mode 3
        OUT  (PIO1BC),A
        LD   A,84H        ;BIT 2 & 7 = Eingang
        OUT  (PIO1BC),A
        LD   A,07H        ;Int.-st.-wort
        OUT  (PIO1BC),A
;
        LD   A,0DAH       ;SCCH-Monitor Initialisierung
        RES  3,A          ;ACC-Zeichensatz einschalten
        OUT  (PIO1BD),A
;
        LD   A,CPMMOD
        OUT  (MAP),A
;
        LD   HL,TEXT1     ;Systemueberschrift
        CALL OUTTXT
        CALL RDTEST       ;Test der RAM-Disk - Größe
        LD   HL,RDSIZE    ;RD-Size: 0=256K 2=1024K
        LD   A,(HL)
        CP   0
        JR   Z,BOOT1      ;256K RAM-Disk, nichts kopieren
;
        LD   HL,DP1024    ;DPB fuer 1024K RAM-Disk kopieren
        LD   DE,DPB0
        LD   BC,15
        LDIR
        LD   HL,TEXT1B    ;Text: "A:RAM-Disk 1024K"
        JR   BOOT2
;
BOOT1:  LD   HL,TEXT1A    ;Text: "A:RAM-Disk 256K"
BOOT2:  CALL OUTTXT
        LD   HL,TEXT1C    ;Text: "B:Floppy 780K"
        CALL OUTTXT
;
        CALL INITRD       ;RAM-Disk Initialisierung
        CALL INITFD       ;FDC initialisieren
;
        XOR  A
        LD   (3),A        ;IOBYTE
        LD   (4),A        ;akt. LW
        LD   (BLTEST),A   ;HST-Kennung ruecksetzen
;
WBOOT:  DI
        LD   SP,80H
        CALL CCPLD        ;CCP + BDOS laden
        XOR  A
        LD   (CTAB),A     ;letzter FDC-Befehl
        LD   A,0C3H
        LD   (0),A        ;JP WBOOT eintragen
        LD   HL,BIOSWB
        LD   (1),HL
        LD   (5),A        ;JP BDOS eintragen
        LD   HL,BDOS
        LD   (6),HL
        LD   (38H),A      ;JP ERROR eintragen
        LD   HL,ERROR
        LD   (39H),HL
        LD   BC,80H
        CALL SETDMA       ;DMA --> 80H
        LD   A,(4)
        LD   C,A          ;aktuelles LW
        EI
        JP   CCP          ;JP CCP
;
