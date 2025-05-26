# FMD68.X makefile for X680x0 GCC (elf2x68k)

#É}ÉNÉç
CC = m68k-xelf-gcc

#CFLAGS =  -O -Wall -g
CFLAGS = -m68000 -O2

#LINKOPTS = -liocs -ldos
# -lsignal
LINKOPTS = -Ttext=0x6800 -s
# -specs=x68knodos.specs

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $<

#ê∂ê¨éËèá

all:FMD68.X

OBJFILES = FMD68.o

FMD68.X : $(OBJFILES)
	$(CC) $(LINKOPTS) -o FMD68.X $(OBJFILES)
# $(CFLAGS) $(LINKOPTS)

FMD68.o : FMD68.c key.h tone.h keyx68.h
