/* X1 FM呼び出し実験 By m@3 */
/* .COM版 */
/* fmx1d.objファイルと演奏ファイルを用意してください */
/* ZSDCC版 */

#include <stdio.h>
#iinclude <stdlib.h>
#include <conio.h>

FILE *stream[2];

#define ERROR 1
#define NOERROR 0

enum {
	WORK_NO,
	WORK_LOOP,
	WORK_NOISE,
	WORK_MIX,
	WORK_CTC0,
	WORK_CTC3,
	WORK_TONE,
	WORK_PART
};

void DI(void){
__asm
	DI
__endasm;
}

void EI(void){
__asm
	EI
__endasm;
}

short bload(char *loadfil)
{
	unsigned short size;
	unsigned short *address;
	unsigned char buffer[2];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, stream[0]);
	address = (unsigned short *)(buffer[0] + buffer[1] * 256);
	fread( buffer, 1, 2, stream[0]);
	size = (buffer[0] + buffer[1] * 256) - (unsigned short)address;
	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address, size, (unsigned short)address + size);

	fread( address, 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

short bload2(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned char *address;
	unsigned char buffer[2];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, stream[0]);
	address = (unsigned short *)(buffer[0] + buffer[1] * 256);
	fread( buffer, 1, 2, stream[0]);
	size = (buffer[0] + buffer[1] * 256) - (unsigned short)address;
	address -= offset;
	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address , size, (unsigned short)address + size);

	fread( address , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

short bloadx1(char *loadfil, unsigned char *buffer, unsigned short size)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

void play_bgm(int no, int noise)
{
	unsigned char *mem=(unsigned char *)0xcc09;
	mem[WORK_NO] = no % 256;
	mem[WORK_LOOP] = no / 256;
//	mem[WORK_MIX] = noise;
	mem[WORK_TONE] = noise % 256;
	if(noise / 256)
		mem[WORK_PART] = 11;
	else
		mem[WORK_PART] = 6;
__asm
	call #0xcc00
__endasm;
}

void stop_bgm(void)
{
__asm
	call #0xcc03
__endasm;
}

void fade_bgm(void)
{
__asm
	call #0xcc06
__endasm;
}



int	main(int argc,char **argv)
{
	int no = 0;
	int noise = 3; //0x38;
	if (argc < 2){
		printf("FM+PSGX1 Loader.\n");
		return ERROR;
	}
	if (argc >= 3){ //argv[2] != NULL){
		no = atoi(argv[2]);
		if((no % 256) > 9)
			no = 0;
/*		else{
			printf("no: %d\n",no);
			getch();
		}
*/	}

	if (argc >= 4){ //argv[2] != NULL){
		noise = atoi(argv[3]);
//		if(noise > 255)
//			noise = 0xc8;
/*		else{
			printf("no: %d\n",no);
			getch();
		}
*/	}

	if(bload2("keyx1.dat", 0x1000*0) == ERROR)
		return ERROR;

	if(bload2("ssg88.dat", 0x100) == ERROR)
		return ERROR;

	if(bload2("fmdx12.obj", 0) == ERROR)
		return ERROR;

	if(bload2(argv[1], 0x1000*0) == ERROR)
		return ERROR;

	EI();

	play_bgm(no, noise);
	if(getchar() != 0x1b)
		stop_bgm();
	else
		fade_bgm();

	return NOERROR;
}

