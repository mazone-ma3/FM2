# FMDTW.EXP makefile for FM TOWNS OpenWatcom

# Cコンパイラとオプション
CC = wcc386

#CFLAGS = -lpharlap -3r -zq -zk0 -s
CFLAGS1 = -3r -zq -zk0  -ox -fo=$*.OBJ

# リンカ
#LINK = wlink

# Cソ－スからobj作成

.SUFFIXES: .OBJ .C
.C.OBJ:
	$(CC)  $< $(CFLAGS1)

# 使用するobj

OBJFILES = SND.OBJ FMDTW.OBJ

# リンクしてEXP作成

FMDTW.EXP : $(OBJFILES)
#	wlink system pharlap runtime CALLBUFS=65536 option quiet name FMDTW file{ $(OBJFILES) }
	wlink system pharlap option quiet,STACK=160000 name FMDTW file{ $(OBJFILES) }
#	wlink system pharlap option quiet,STACK=262144 name FMDTW file{ $(OBJFILES) }
#	wlink system pharlap runtime CALLBUFS=65536 option quiet,STACK=262144 name FMDTW file{ $(OBJFILES) }


SND.OBJ : SND.C SND.H

FMDTW.OBJ : FMDTW.C SND.H
