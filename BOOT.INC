;       ****************
;       *              *
;       * BOOT & WBOOT *
;       *              *
;       ****************
;
;24.01.2023
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
        CALL RDTEST       ;Kapazitaet der RAM-Disk bestimmen
        LD   A,(RDSIZE)   ;RD-Size: 0=256K 1=512K 2=1024K 3=2048K
        LD   HL,DP2048    ;DPB 2048K RAM-Disk
        LD   IX,TEXT1D    ;Text: "A:RAM-Disk 2048K"
        CP   3
        JR   Z,BOOT2
        LD   HL,DP1024    ;DPB 1024K RAM-Disk
        LD   IX,TEXT1C    ;Text: "A:RAM-Disk 1024K"
        CP   2
        JR   Z,BOOT2
        LD   HL,DP512     ;DPB 512K RAM-Disk
        LD   IX,TEXT1B    ;Text: "A:RAM-Disk 512K"
        CP   1
        JR   Z,BOOT2
        LD   IX,TEXT1A    ;Text: "A:RAM-Disk 256K"
        JR   BOOT3        ;256K RAM-Disk, kein LDIR
;        
BOOT2:  LD   DE,DPB0
        LD   BC,15
        LDIR
BOOT3:  PUSH IX           ;RAM-Disk Text --> HL
        POP  HL
        CALL OUTTXT
        LD   HL,TEXT1E    ;Text: "B:Floppy 780K"
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
        DI
        LD   SP,80H
        JR   WBOOT1       ;CCP-Read ueberspringen
;
WBOOT:  DI
        LD   SP,80H
        CALL CCPRD        ;CCP + BDOS laden
WBOOT1: XOR  A
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