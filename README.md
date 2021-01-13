# my6502
 My projects and libraries for the Ben Eater 6502 computer

Thanks to Ben Eater (https://www.youtube.com/c/beneater) for the base project, some scripts, and the inspiration.

![BE6502 Dino Game GIF](RESOURCE/BE6502Dino.gif)

## Hardware Requirements
This code will work on BE6502 computers that have been built up to the "Interrupt handling" video.  
Also, this code only requires a 8KiB EEPROM rather than the 32KiB one Ben uses*. The code will work on either.  
Additionally, some of this software may require a CMOS 6502 (65c02) chip to work.  
Files that require this will have "Processor: 65c02" in the description comment at the top.

*I ordered a 32KiB one for this project, but the one I got was defective/broken/softlocked. Unfortunately, you must pay shipping to return things to Jameco, so I just borrowed the 8KiB EEPROM chip from my Ben Eater 8-bit computer.
## Software Requirements
The code in this repository is compiled with VASM oldstyle. You can download VASM at http://www.ibaug.de/vasm/vasm6502.zip

Here are the files/folders in this project:
```
LIBS                : Libraries I wrote for the 6502.
PROJECTS            : Projects utilizing libraries from the LIBS folder.
RESOURCE			: Images for projects used by the README
6502AssemblyUDL.xml : A Notepad++ User Defined Language file I made for 6502 assembly.
```
Individual project files and libraries will have description comments at the top.

## Compiling Projects
To compile (assemble) a project, first copy any necessary library files into the build directory with the project source. Also copy the vasm-oldstyle binary into the folder  
For projects that do NOT require a 65c02, use the following command:
```
vasm6502-oldstyle -Fbin -dotdir ./filenamehere -o outputfile
```
For projects that DO require a 65c02, use the following command:
```
vasm6502-oldstyle -Fbin -dotdir -wdc02 ./filenamehere -o outputfile
```

## Using Libraries
If you want to use library files for yourself, open the .inc file and observe the description comment.  
There are three fields that are important.  
First, the "Libs require" field will specify any libraries it is dependent on.
Second, the "RAM base var" field will specify the name of an assembler variable that you must define for the library to store things in RAM. For example, if the base var is "EXAMPLE" then you would include the following somewhere in your code:
```
EXAMPLE = $200 ; or whatever address in RAM you want to use
```
Third is the RAM required field. This will tell you how much RAM past the RAM base var this library will use. It is your responsibility to ensure that the used RAM does not overlap with another library or the program itself.
