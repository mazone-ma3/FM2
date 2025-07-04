/* FM MUSIC SAMPLE for X68K */
/* GCC(elf2x68k) */
/* 参考 PC-9801マシン語サウンドプログラミング、Inside X68000 */
/* プログラマーのためのX68000環境ハンドブック */

#include <stdio.h>
#include <stdlib.h>
#include <x68k/iocs.h>

#include "key.h"
#include "keyx68.h"
#include "tone.h"

FILE *stream[2];

#define ERROR 1
#define NOERROR 0

#define PARTSUU 8

unsigned char COUNT[PARTSUU] = {1,1,1,1,1,1,1,1}; // 音長カウンタ
unsigned char STOPFRG[PARTSUU] = {0,0,0,0,0,0,0,0}; // WAIT&SYNC&OFF
unsigned char MAXVOL[PARTSUU] = {15,15,15,15,15,15,15,15}; // ボリューム
unsigned char LENGTH[PARTSUU] = {5,5,5,5,5,5,5,5}; // 基本音長

unsigned short OFFSET[PARTSUU] = {0,0,0,0,0,0,0,0}; // 演奏データ実行中アドレス
unsigned short STARTADR[PARTSUU] = {0,0,0,0,0,0,0,0}; // 演奏データ開始アドレス
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
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, stream[0]);
	address = buffer[0] + buffer[1] * 256;
	fread( buffer, 1, 2, stream[0]);
	size = (unsigned short)(buffer[0] + buffer[1] * 256) - (unsigned short)address;
	address -= offset;
	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address , size, (unsigned short)address + size);

	fread( &mem[address] , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

#define port1 ((volatile unsigned char *)0xe90001)
#define port2 ((volatile unsigned char *)0xe90003)

/* FMレジスタ設定 */
void set_fm(unsigned char reg, unsigned char data)
{
	unsigned char a;

	while(*port2 & 0x80);
dummy:
	*port1 = reg;
	while (*port2 & 0x80);
dummy2:
	*port2 = data;
}

/* 音色設定 */
void set_tone(int no, int ch)
{
	unsigned char i, j;
	j = 0x40 + ch;
	for(i = 0; i < 24; ++i){
		set_fm(j, tone_table[no][i]);
		j+=8;
	}

	j += 0x20;
	i += 4;
	set_fm(j, tone_table[no][i] | 0xc0);
}

/* 音階設定 */
void set_key(int no, int ch)
{
	set_fm(0x28 + ch, key_table[no]);
	set_fm(0x30 + ch, 5);
}

char key[8] = {0, 1, 2, 3, 4, 5, 6, 7};


void stop(void);

unsigned char count = 0;

void  __attribute__((interrupt))int_fm(void)
{
	unsigned char i, j, no, ch;
	unsigned char data;

	set_fm(0x14, 0x2a);

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
						set_fm(ch, no);
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
						set_fm(0x08, 0x00 | key[i]);	/* off */
						if((data & 0x7f) != 0){
							set_key((data & 0x7f) - 1, i);	/* key */
							set_fm(0x08, 0xf0 | key[i]);	/* on */
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

int init_int(void)
{
	int ret = 0;
	/* 割り込み off */
	asm volatile("ori.w	#0x0700,%sr\n");
	ret = _iocs_opmintst(int_fm);
	/* 割り込み on */
	asm volatile("andi.w	#0x0f8ff,%sr\n");
	return ret;
}

void reset_int(void)
{
	asm volatile("ori.w	#0x0700,%sr\n");
	_iocs_opmintst (0);
	asm volatile("andi.w	#0x0f8ff,%sr\n");
}

void stop(void)
{
	unsigned char i;
	set_fm(0x14, 0x0);
	for(i = 0; i < PARTSUU; ++i){
		set_fm(0x08, 0x00 | key[i]);	/* off */
	}
	reset_int();
}

int	main(int argc,char **argv)
{
	unsigned short no = 0;
	unsigned char i, ch = 0;
	unsigned char noise = 0;

dum:	_iocs_b_super(0);		/* スーパーバイザモード 最適化防止にラベルを付ける */

	if (argc < 2){
		exit(1);
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
		exit(1);

	for(i = 0; i < PARTSUU; ++i){
		int j = 0xdb00-0x1100 + i * 2 + (no % 256) * 12 * 2;
		COUNT[i] = 1;
		STARTADR[i] = mem[j] + mem[j+1] * 256;
		if(!(STARTADR[i])){
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

	set_fm(0x12, 191);
	if(init_int())
		exit(1);
	set_fm(0x14, 0x2a);

	getchar();

	stop();

	exit(0);
}
