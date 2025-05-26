copy %1.h mml.h
cc fmcomp.c -o fmcomp.exe
fmcomp %1.pd2
conv2 %1.pd2 %1.ob2
