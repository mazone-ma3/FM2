/* FM MUSIC SAMPLE for FM TOWNS BIOS版含む */
/* WatcomC */
/* 参考 PC-9801マシン語サウンドプログラミング、FM TOWNSテクニカルデータブック */
/* Free386のドキュメント(SND.C) */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "key.h"
#include "keytow.h"
#include "tone.h"

FILE *stream[2];

#define ERROR 1
#define NOERROR 0

#define BIOS
//#define BIOSVOL


#ifdef BIOS
#define BIOSVOL
#include "snd.h"
char sndwork[SndWorkSize];
#endif

#define PARTSUU 6

unsigned char COUNT[PARTSUU] = {1,1,1,1,1,1}; // 音長カウンタ
unsigned char STOPFRG[PARTSUU] = {0,0,0,0,0,0}; // WAIT&SYNC&OFF
unsigned char MAXVOL[PARTSUU] = {15,15,15,15,15,15}; // ボリューム
unsigned char LENGTH[PARTSUU] = {5,5,5,5,5,5}; // 基本音長

unsigned short OFFSET[PARTSUU] = {0,0,0,0,0,0}; // 演奏データ実行中アドレス
unsigned short STARTADR[PARTSUU] = {0,0,0,0,0,0}; // 演奏データ開始アドレス
unsigned char FEEDVOL = 0; // フェードアウトレベル
unsigned char FCOUNT = 1; //フェードアウトカウンタ
unsigned char LOOPTIME = 1; // 演奏回数（０は無限ループ）
unsigned char STOPPARTS = 0; //
unsigned char ENDFRG = 0; //
unsigned char NSAVE = 0; //


unsigned char mem[65536];

short bload2(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned short address;
	unsigned char buffer[3];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
//		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, stream[0]);
	address = buffer[0] + buffer[1] * 256;
	fread( buffer, 1, 2, stream[0]);
	size = (unsigned short)(buffer[0] + buffer[1] * 256) - (unsigned short)address;
	address -= offset;
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address , size, (unsigned short)address + size);

	fread( &mem[address] , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

/* FMレジスタ設定 ch.1-3 */
#ifdef BIOS
void set_fm(char bank, char reg, char data)
{
	while(SND_fm_read_status() & 0x80);
	SND_fm_write_data(bank, reg, data);
//	SND_fm_write_save_data(bank, reg, data);
}
#else
void set_fm(unsigned char bank, unsigned char reg, unsigned char data)
{
	int port;
	if(bank == 1)
		port = 0x4dc;
	else if(bank == 0)
		port = 0x4d8;

	while(inp(0x4d8) & 0x80);
	outp(port,reg);
//__asm
//	ld	a,(ix+0)
//__endasm;
//	while(inp(0x4d8) & 0x80);
	outp(port+2,data);
}
#endif

/* 音色設定 */
void set_tone(unsigned char no, unsigned char ch)
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
void set_key(unsigned char no, unsigned char ch)
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

void stop(void)
{
	unsigned char i;
	for(i = 0; i < PARTSUU; ++i){
		set_fm(0, 0x28, 0x00 | key[i]);	/* off */
	}
	SND_end();
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
						set_fm(0, 0x28, 0x00 | key[i]);	/* off */
						if((data & 0x7f) != 0){
							set_key((data & 0x7f) - 1, i);	/* key */
							set_fm(0, 0x28, 0xf0 | key[i]);	/* on */
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

#ifdef BIOSVOL
	SND_init(sndwork);
#endif

	if(bload2(argv[1], 0x1000*0) == ERROR)
		return ERROR;

	for(i = 0; i < PARTSUU; ++i){
		int j = 0xdb00-0x1100 + i * 2 + (no % 256) * 12 * 2;
		COUNT[i] = 1;
		STARTADR[i] = mem[j] + mem[j+1] * 256;
		if(!STARTADR[i]){
			STOPFRG[i] = 255;
			MAXVOL[i] = 255;
			STOPPARTS++;
		}else{
			MAXVOL[i] = 0;
			STOPFRG[i] = 0;
		}
		STARTADR[i] -= 0x1000;
		OFFSET[i] = STARTADR[i];
		set_tone(noise, i);
	}
	LOOPTIME = no / 256;

	/* ミュート解除 */
#ifdef BIOSVOL
	SND_elevol_all_mute(-1);
	SND_elevol_mute(0x33);
#else
//	outp(0x4e0,0x3f);
//	outp(0x4e1, (inp(0x4e1) | 0x0c) & 0x8f);
//	outp(0x4e2,0x3f);
//	outp(0x4e3, (inp(0x4e3) | 0x0c) & 0x8f);
	outp(0x4ec, 0x40); //(inp(0x4ec) | 0x40) & 0xc0);
	outp(0x4d5, 0x02);
//	outp(0x60,0x4);
#endif

	SND_int();
	getch();

	stop();
	return 0;
}
