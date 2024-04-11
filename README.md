# ACC-CPM
Assemblercode von ACC-CPM für den AC1-2010

ACC-CPM ist ein CP/M V2.2 kompatibles Betriebssystem und wurde vom **A**mateur-**C**omputer-**C**lub Berlin für den AC1 entwickelt.
Es besteht aus CCP, BDOS und BIOS.
Das BIOS ist modular aufgebaut und in diesem Repository an die Hardware des AC1-2010 angepasst.
Es benötigt eine Präzitronic RAM-Disk und eine FDC-Karte mit *double sided, double density* Diskettenlaufwerk.
Der CCP beginnt auf Adresse 0D000h, die Startadresse ist 0E600h.

Für den CCP und das BDOS wurden CP/A-Assemblerdateien von Volker Pohlers verwendet:
https://hc-ddr.hucki.net/wiki/doku.php/cpm/cpa

Im BIOS wurden Änderungen vorgenommen:
- Automatische Erkennung der RAM-Disk-Größe: 256 / 512 / 1024 / 2048 KByte
- Umschalten des Diskettenlaufwerks auf 780 / 800 KByte mit "DISK"
- Der Floppy-Treiber kann im BIOS auskommentiert werden, der CCP beginnt dann bei 0E000h

Das Assemblieren des Quellcodes erfolgt mit SUBMIT.COM:
```
SUBMIT BIOS
```

Nachfolgend ein Bildschirmcopy von ACC-CPM im JKCEMU:

![Bildschirmcopy von ACC-CPM](https://github.com/friedertonn/ACC-CPM/blob/main/Fotos/ACC-CPM.png?raw=true)
