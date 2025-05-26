/* FM MUSIC SAMPLE for PC-88VA */
/* WatcomC */
/* 参考 PC-9801マシン語サウンドプログラミング、PC-88VAテクニカルマニュアル */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#include "key.h"
#include "key98.h"
#include "tone.h"
#include "psgtone.h"

FILE *stream[2];

#define ERROR 1
#define NOERROR 0

#define EOIDATA 0x20
#define EOI_M 0x188
#define EOI_S 0x184
#define IMR_M 0x18a
#define IMR_S 0x186

#define ON 1
#define OFF 0
#define PAT 1

void __interrupt __far (*keepvector)(void);
unsigned char keepport;

#define VECT 0x14 //0x0a

#ifdef DEBUG
/* 曲 */
int mml[] = {
	KEY_D4,
	KEY_G4,
	KEY_A4,
	KEY_B4,
	KEY_G4,
	KEY_A4,
	KEY_B4,
	KEY_C5,
	KEY_B4,
	KEY_A4,
	KEY_B4,

	KEY_END,
};
#endif

#define MAX_PARTSUU 9
unsigned char PARTSUU = 9;

unsigned char COUNT[MAX_PARTSUU] = {1,1,1,1,1,1,1,1,1}; // 音長カウンタ
unsigned char STOPFRG[MAX_PARTSUU] = {0,0,0,0,0,0,0,0,0}; // WAIT&SYNC&OFF
unsigned char MAXVOL[MAX_PARTSUU] = {15,15,15,15,15,15,15,15,15}; // ボリューム
unsigned char LENGTH[MAX_PARTSUU] = {5,5,5,5,5,5,5,5,5}; // 基本音長

unsigned short OFFSET[MAX_PARTSUU] = {0,0,0,0,0,0,0,0,0}; // 演奏データ実行中アドレス
unsigned short STARTADR[MAX_PARTSUU] = {0,0,0,0,0,0,0,0,0}; // 演奏データ開始アドレス
unsigned char FEEDVOL = 0; // フェードアウトレベル
unsigned char FCOUNT = 1; //フェードアウトカウンタ
unsigned char LOOPTIME = 1; // 演奏回数（０は無限ループ）
unsigned char STOPPARTS = 0; //
unsigned char ENDFRG = 0; //
unsigned char NSAVE = 0; //

unsigned char mem[65536/4];

short bload2(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned short address;
	unsigned char buffer[3];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, stream[0]);
	address = buffer[0] + buffer[1] * 256;
	fread( buffer, 1, 2, stream[0]);
	size = (unsigned short)(buffer[0] + buffer[1] * 256) - (unsigned short)address;
	address -= offset;
	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address , size, (unsigned short)address + size);

	fread( &mem[address-0xc000] , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

/* FMレジスタ設定 ch.1-3 */
void set_fm(char bank, char reg, char data)
{
	int port;
	if(bank == 1)
		port = 0x44+2;
	else if(bank == 0)
		port = 0x44;

	while(inp(0x44) & 0x80);
	outp(port,reg);
//__asm
//	ld	a,(ix+0)
//__endasm;
	while(inp(0x44) & 0x80);

/*	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);

	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);

	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);

	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
	outp(0x5f,0);
*/
	outp(port+1,data);
}

/* 音色設定 */
void set_tone(char no, char ch)
{
	char i, j, bank = 0;
	if(ch >= 3){
		ch -= 3;
		bank = 1;
	}

	j = 0x30 + ch;
	for(i = 0; i < 28; ++i){
		set_fm(bank, j, tone_table[no][i]);
		j += 4;
	}
	j += 0x10;
		set_fm(bank, j, tone_table[no][i]);
}

/* 音程設定 */
void set_key(char no, char ch)
{
	char i, j, bank = 0;
	if(ch >= 3){
		ch -= 3;
		bank = 1;
	}
	set_fm(bank, 0xa4 + ch, key_table[no][0]);
	set_fm(bank, 0xa0 + ch, key_table[no][1]);
}


char key[6] = {0, 1, 2, 4, 5, 6};

void int_end(void);

void stop(void)
{
	unsigned char i;
	for(i = 0; i < PARTSUU - 3; ++i){
		set_fm(0, 0x28, 0x00 | key[i]);	/* off */
	}
	for(i = 0; i < 3; ++i){
		set_fm(0, 8 + i, 0);	/* PSG */
	}
	int_end();
}

unsigned char count = 0;

void int_fm(void)
{
	unsigned char i, j, no, ch;
	unsigned char data;

playloop:
playloop2:
//	if(FEEDVOL == 15)
//		stop();
//	if(FEEDVOL)
//		feedsub();
	for(i = 0; i < PARTSUU; ++i){
		if(STOPFRG[i] >= 254){
			/* 同期待ち・演奏終了 演奏スキップ */
			continue;
		}
		--COUNT[i];
		if(!(COUNT[i])){
			/* 演奏処理 */
			for(;;){
				data = mem[OFFSET[i]];
				switch(data){
					case 226:	/* 音量変更 V */
						data = mem[--OFFSET[i]];
						MAXVOL[i] = data;
						break;
					case 227:	/* 標準音調変更 T */
						data = mem[--OFFSET[i]];
						LENGTH[i] = data;
						break;
					case 228:	/* 音色変更 */
						no = mem[--OFFSET[i]];
						set_tone(no, i);
						break;
					case 225:	/* 直接出力 Y */
						ch = mem[--OFFSET[i]];
						no = mem[--OFFSET[i]];
						set_fm(0, ch, no);
						break;
					case 255:	/* ループ */
						OFFSET[i] = STARTADR[i] + 1;
						ENDFRG = 1;
						break;
					case 254:	/* 同期 */
						STOPFRG[i] = 254;
						--OFFSET[i];
						++STOPPARTS;
						goto playend;
						break;
					default:
						/* 演奏 */
						STOPFRG[i] = data & 0x7f;
						if(i < PARTSUU - 3){	/* FM */
							set_fm(0, 0x28, 0x00 | key[i]);	/* off */
							if((data & 0x7f) != 0){
								set_key((data & 0x7f) - 1, i);	/* key */
								set_fm(0, 0x28, 0xf0 | key[i]);	/* on */
							}
						}else{
							if(data & 0x7f){
							/* PSG */
								set_fm(0, (i - 3) * 2 + 0, psg_tone[((data & 0x7f) - 1) * 2 ]);
								set_fm(0, (i - 3) * 2 + 1, psg_tone[((data & 0x7f) - 1) * 2 + 1]);
								set_fm(0, 8 + i - 3, MAXVOL[i - 3]);
							}else{
								set_fm(0, 8 + i - 3, 0);
							}
						}
						if(data & 0x80){	/* 音長が設定されている */
							data = LENGTH[i];
						}else{
							data = mem[--OFFSET[i]];
						}
						COUNT[i] = data;
						--OFFSET[i];
						goto playend;
						break;
				}
				--OFFSET[i];
			}
		}
playend:
		continue;
	}
	if(PARTSUU == STOPPARTS){	/* 演奏パート数=停止パート数 */
		j = PARTSUU;
		for(i = 0; i < PARTSUU; ++i){
			if(STOPFRG[i] != 255){	/* 演奏停止でない */
				STOPFRG[i] = 0;	/* 演奏中 */
				COUNT[i] = 1;	/* 音長カウンタを1にする */

				--j;		/* 演奏停止パート数の計算 */
			}
		}
		STOPPARTS = j;	/* 演奏停止パート数 */
		goto playloop2;
	}

	/* 割り込み終了 */
	if((--ENDFRG) == 0){	/* 終了カウンタが0なら終了 */

		ENDFRG = 0;
		if(!LOOPTIME)
			return;			/* 無限ループ */
		if(--LOOPTIME){
			goto playloop;	/* ループ回数が0以外ならループ */
		}
		/* 演奏自己停止 */
		stop();
	}
	ENDFRG = 0;
}

void s_eoi(void)
{
	unsigned char a;

	outp(EOI_S,	EOIDATA);
	outp(EOI_S, 0x0b);
	a = inp(EOI_S);
	a &= a;
	if(!a)
		outp(EOI_M, EOIDATA);
}

void __interrupt far int_timer(void)
{
	outp(0x32, inp(0x32) | 0x80);
	if(inp(0x44) & 0x02){	/* Timer-B */
		set_fm(0, 0x27, 0x28);
		set_fm(0, 0x27, 0x2a);
		int_fm();
	}
//	outp(EOI_M, EOIDATA);

/*	asm{
		movb $0x20,al
		outb al,$0x00
	};*/
	outp(0x32, inp(0x32) & 0x7f);
	s_eoi();
}

void init_int(void)
{
	_disable();
	outp(0x158, 0);

	keepport = inp(IMR_S);
	keepvector = _dos_getvect(VECT);
	outp(IMR_S, inp(IMR_S) & 0xef);	/* INT14(IR12) */

	outp(0x32, inp(0x32) & 0x7f);
	set_fm(0, 0x26, 199);

	set_fm(0, 0x27, 0x28);
	set_fm(0, 0x27, 0x2a);
	_dos_setvect(VECT, int_timer);

//	outp(EOI_M, EOIDATA);
	s_eoi();
	_enable();
}

void int_end(void)
{
	_disable();
	_dos_setvect(VECT, keepvector);
	outp(IMR_S, keepport);

	set_fm(0, 0x27, 0);
	_enable();
}

int	main(int argc,char **argv)
{
	unsigned short no = 0;
	unsigned char i, ch = 0;
	unsigned char noise = 0;

	if (argc < 2){
		return ERROR;
	}

	if (argc >= 3){
		no = atoi(argv[2]);
		if((no % 256) > 9)
			no = 0;
	}

	if (argc >= 4){
		noise = atoi(argv[3]);
	}

	if(bload2(argv[1], 0x1000*0) == ERROR)
		return ERROR;

	outp(0x44, 0x00);
	outp(0x45, 0x00);
	outp(0x44, 0x00);
	outp(0x45, 0x01);
	outp(0x44, 0x00);
	if(inp(0x45) != 1){
		printf("0x44 not found ");
		return ERROR;
	}else{
		outp(0x44 ,0xff);
		if(inp(0x45) == 0x01){
			printf("0x44 = OPNA ");
			set_fm(0, 0x29, 0x82);
//			mem[WORK_PART] = 9;
			PARTSUU = 9;
		}else{
			printf("0x44 = OPN ");
//			mem[WORK_PART] = 6;
			PARTSUU = 6;
		}
	}
	set_fm(0, 0x7, 0x38);
	printf("\n");

	for(i = 0; i < PARTSUU; ++i){
		int j = 0xdb00-0x1100 + i * 2 + (no % 256) * 12 * 2;
		COUNT[i] = 1;
		STARTADR[i] = mem[j - 0xc000] + mem[j+1 - 0xc000] * 256;
		if(!STARTADR[i]){
			STOPFRG[i] = 255;
			MAXVOL[i] = 255;
			STOPPARTS++;
		}else{
			MAXVOL[i] = 0;
			STOPFRG[i] = 0;
		}
		STARTADR[i] -= 0x1000;
		STARTADR[i] -= 0xc000;
		OFFSET[i] = STARTADR[i];
		if(i < PARTSUU - 3)
			set_tone(noise, i);
	}
	LOOPTIME = no / 256;


	init_int();
//	getch();

__asm{
	mov	ah,0
	int	82h
};

	stop();



#ifdef DEBUG
	i = 0;
	while(1){
		if(mml[i] == KEY_END)
			break;

		set_key(mml[i], ch);	/* key */
//		set_fm(0xc0, 0xb4 + ch);

		set_fm(0, 0x28, 0xf0 | key[ch]);	/* on */
		getchar();
		set_fm(0, 0x28, 0x00 | key[ch]);	/* off */

		++i;
	}
#endif

//	exit(0);
	return 0;
}



