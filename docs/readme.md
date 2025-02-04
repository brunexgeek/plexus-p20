# Technical information and documentation of Plexus machines

Bitsavers has posted many scanned documents https://bitsavers.org/pdf/plexus/ for other Plexus systems like the P/25, P/40 and P/60. These are completely different systems from the P/20. The other systems use Multibus for all I/O and SMD storage. It seems with the P/20 and P/15, a second MC68010 was added to the motherboard to handle all I/O, SCSI was added for all disk and block storage and a new memory map was designed to handle up to 8mb of RAM. While the P/20 seems to run ports of the boot-ROM and OS, the systems are not likely compatible (from a software perspective) and from a hardware perspective, the P/20 is completely different from the P/25 and other models. 

For the P/20, please see the various files in this directory for more information. Datasheets and other scanned material has been moved to PDF.

Mainboard PCB design files are in the 'CAD' directory.

Discord user SuperSVGA is working on recreating firmware here on EasyEDA:
https://easyeda.com/editor#id=4458ae42beaf4e60baca440d338c28c5|98b98baddd544786ae3853954d24d9cd|df686bafd0404414ab79156bae6786e2|f751b3e4a1ff45f5856200f6752685d9|a056aae735f0469ea7f66c1a44c826a9|68cad4c79d3b45df94a9f1b974c85fd0

### Multibus

The Plexus P/20 has three multibus slots on an expansion card backplane. There is evidence the other Plexus models supported 10mbit ethernet with the Exeland Exos 201 ethernet card. The P/20 has some support for ethernet in the bootrom, so it's possible it is for the same card.

http://bitsavers.trailing-edge.com/communications/excelan/4200006-00_EXOS201refJun86.pdf

The other two cards with native support at the Plexus ICP (intelligent communication processor) and Plexus ACP (advanced communication processor) which are from the other Plexus models. They add 8 extra serial ports and come with an 8 port DB25 plate that screws onto the back of the P/20 and from looking at pictures of the P/25, it appears to be the same exact plate. (Based on the mounting screws and apparently visible size.) 

### Serial port map:

(Thanks to Patron Peter)

The book "Complete Guide to Rs-232 and Parallel Connections: A Step-By-Step Approach to Connecting Computers, Printers, Terminals, and Modems" actually has the serial connector pinouts for the Plexus P/15 and P/20.
```
Port 0 - console
port 1-3 no HW handshake
port 4-5 signals provided for modems
port 6-7 include timing lead support

Ports 1-3: 1-TX, 6-RX and 8-GND
Ports 4-5: 1-TX, 2-RTS, 4-DTR, 6-RX, 7-CTS, 8-GND, 9-DCD
Port 6-7: Normal DB25 RS232 with full flow control
```
### Memory map:
```
|   Bus Master | Space |  Loc   | Address Range | Function
|Job MB DMA BLK|Usr Sys|Job DMA | ------------- | -----------------    
  x      x       x       x       000000 - 7FFFFF  Main User memory
  x      x           x   x       000000 - 7FFFFF  Main Sys memory
     x               x   x       780000 - 7FFFFF  Main Sys Memory
             x       x   x       600000 - 6FFFFF  Main Sys Memory
  x      x           x       x   800000 - 80FFFF  PROM (4 - 27128s)
  x                  x   x       900000 - 901FFF  User Page Map
  x                  x   x       902000 - 903FFF  Sys Page Map
  
  x      x           x       x   A00001 - A0003F  UART 'A' (odd) 0080a314 00 a0 00 01 addr DAT_00a00001
  x      x           x       x   A00021 - A0003F  UART '#' (odd) 0080a310 00 a0 00 21 addr DAT_00a00021
  
  x      x           x       x   A10001 - A1003F  UART 'B' (odd) 0080a31c 00 a1 00 01 addr DAT_00a10001
  x      x           x       x   A10021 - A1003F  UART '#' (odd) 0080a318 00 a1 00 21 addr DAT_00a10021
  
  x      x           x       x   A20001 - A2003F  UART 'C' (odd) 0080a324 00 a2 00 01 addr DAT_00a20001
  x      x           x       x   A20021 - A2003F  UART '#' (odd) 0080a320 00 a2 00 21 addr DAT_00a20021
  
  x      x           x       x   A30001 - A3003F  UART 'D' (odd) 0080a328 00 a3 00 01 addr DAT_00a30001
  x      x           x       x   A30021 - A3003F  UART '#' (odd) 0080a32c 00 a3 00 21 addr DAT_00a30021
 
  x      x           x       x   A70000 - A70003  SCSI Data Buffers
  x                  x   x       B00000 - B7FFFF  MBus I/O Space
  x                  x   x       B80000 - BFFFFF  MBus Mem Space
  x      x   x       x       x   C00000 - C03FFF  SRAM
  x      x           x       x   D00001 - D0001B  Calendar (odd)
  x      x           x       x   D0001D - D0007F  Calendar RAM (odd)
  x      x           x       x   E00000 - E0001E  Control/Status Reg.
  x      x           x       x   F00003           MBus Int Vector
  x      x           x       x   F00005           Job Int Vector
  x      x           x       x   F00007           DMA Int Vector
  x      x           x       x   F00009           SCSI Int Vector
  x      x           x       x   F0000B           UART Int Vector
  x      x           x       x   F0000D           Clock Int Vector
  x      x           x       x   F0000F           Panic Int Vector
```
### Main Logic Board Components

| Desig. | Part Number | Description |
| ------ | ----------- | ----------- |
| U16F | MC68010L12 | Motorola 68010 Microprocessor |
| U27H | MC68010L12 | Motorola 68010 Microprocessor |
| U15K | 27128-30 | 32Kx8 EEPROM |
| U15L | 27128-30 | 32Kx8 EEPROM |
| U17K | 27128-30 | 32Kx8 EEPROM |
| U17L | 27128-30 | 32Kx8 EEPROM |
| U19K | M5M5165P-10 | 8Kx8 SRAM |
| U19L | M5M5165P-10 | 8Kx8 SRAM |
| U21E | TMM2018D-45 | 2Kx8 SRAM |
| U21H | TMM2018D-45 | 2Kx8 SRAM |
| U22E | TMM2018D-45 | 2Kx8 SRAM |
| U22H | TMM2018D-45 | 2Kx8 SRAM |
| U23E | TMM2018D-45 | 2Kx8 SRAM |
| U23H | TMM2018D-45 | 2Kx8 SRAM |
| U3M | | PAL? |
| U6F | | PAL |
| U13C | | PAL? |
| U23C | | PAL? |
| U24C | | PAL? |
| U25C | | PAL? |
| U24L | PAL16L8ACN | PAL |
| U19N | | PAL? |
| U9P | MK68564N-3A | MK68564N Serial Input/Output Controller |
| U15P | MK68564N-3A | MK68564N Serial Input/Output Controller |
| U21P | MK68564N-3A | MK68564N Serial Input/Output Controller |
| U26P | MK68564N-3A | MK68564N Serial Input/Output Controller |
| U2H | MC146818P | [Motorola MC146818 Real-Time Clock plus RAM](https://www.nxp.com/docs/en/data-sheet/MC146818.pdf) |

### General and random technical information:

- Dual MC68010 CPUs. One is the "DMA" processor which handles I/O and all boot diagnostics. The other is "JOB" which likely handles UNIX and user processes.
- MC1488 / MC1489 used at RS232 drivers/buffers
- MOSTEK MK68564 used as the SIO chip to drive the 8 serial ports (4 of them)
- The Motorola MC146818 is used for the battery backed up RAM and clock, just like on the PC and countless other systems. There is a NiCD battery attached to the motherboard to power this circuit when the system is turned off.  
- The reset circuit on the Plexus is inside the primary power supply. Pin 4 on the motherboard power connector is the input from the PSU to the motherboard, so you cannot run the system on the bench with a different power supply without managing this input yourself. (Logic low = held in reset, logic high 5V = out of reset.) 
- The PSU remote power on pin is grounded to turn on the power supply. It has relays inside to turn itself on and also power up the secondary PSU and external fan.
- The PSU reset input is grounded by the front keyswitch, which which is input into the reset circuit inside the PSU which supplies a reset (Logic low) to the motherboard via pin 4.
- On my system the primary PSU powers up the motherboard, hard drive, floppy drive and tape drive. The secondary PSU powers up the multibus backplane and the SCSI / MFM interposer. These are powered by normal Molex style 5/12v connectors, so you can use a normal PC splitter to eliminate the secondary PSU if you aren't using any multibus cards. Just unplug the cable that feeds from the primary to the secondary power supply.
- Serial port 0 is the console port. See below for the pinout. (It is non standard) It runs at 9600bps, 8N2 as the default baud rate.
- The top processor (MC68010, named the DMA processor) runs the boot diagnostics. Without it installed or working, the system will not do anything at power up. This CPU likely handles all IO operations while the system is running.
- The bottom processor (named JOB) likely runs UNIX and user tasks. 
- My system has 2048k of 41256 DRAM installed (8 chips + 1 parity per 256k) on a RAM board on the backside of the motherboard. All of the RAM chips are in sockets and can be tested in another system. 

