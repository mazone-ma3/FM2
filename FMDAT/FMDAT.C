/* FM MUSIC SAMPLE for PC AT */
/* OpenWatcom */
/* 参考 PC-9801マシン語サウンドプログラミング / DOS/Vプログラミングリファレンス */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#include "tone.h"
#include "Keyat.h"
#include "Key.h"

#define BASEIO 0x220
#define EOIDATA 0x20
#define EOI_M 0x20

FILE *stream[2];

#define ERROR 1
#define NOERROR 0

void __interrupt __far (*keepvector)(void);
unsigned char keepport;

unsigned char fm_flag = 0;
unsigned short fm_timer = 20000;
unsigned short dos_timer = 0;

#define VECT 0x8 //0x0a


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

void wait(int loop)
{
	while(loop--){
		__asm {
			CLD
			CLD
			CLD
			CLD
			CLD
			CLD
		};
	}
}

/* FMレジスタ設定 */
void set_fm(unsigned char reg, unsigned char data)
{
//	printf("%x = %x\n",reg, data);
	outp(BASEIO + 8,reg);
	wait(12);
	outp(BASEIO + 9,data);
	wait(84);
}

/* 音程設定 */
void set_key(int no, int ch)
{
	set_fm(0xa0 + ch, key_table[no][1]);
	set_fm(0xb0 + ch, 0x20 | key_table[no][0]);
}

unsigned char ch_table[9] = {0, 1, 2, 6, 9, 0xa, 0x10, 0x11, 0x12};

/* 音色設定 */
void set_tone(int no, int ch)
{
	int i = 0, j = 0x20 + ch_table[ch], k;

	for(k = 0 ; k < 4; ++k){
//		printf("%x ",j);
		set_fm(j , tone_table[no][i++]);
		j+=3;
		set_fm(j , tone_table[no][i++]);
		j+=(0x20-3);
	}

	j+=(0x0c0-0x80-0x20);
//	printf("%x ",j);
	set_fm(j , tone_table[no][i++]);

	j+=(0x0e0-0xc0);
//	printf("%x ",j);
	set_fm(j , tone_table[no][i++]);
	j+=3;
	set_fm(j , tone_table[no][i++]);
}

void int_end(void);

void stop(void)
{
	unsigned char i;
	for(i = 0; i < PARTSUU; ++i){
		set_fm(0xb0 + i, 0x0); /* key off */
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

						set_fm(0xb0 + i, 0x0); /* key off */

						if((data & 0x7f) != 0){
							set_key((data & 0x7f) - 1, i);	/* key */
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

void __interrupt far int_timer(void)
{
	int_fm();

	if(dos_timer >= fm_timer){
		dos_timer -= fm_timer;
		outp(EOI_M, EOIDATA);
	}else{
		dos_timer -= fm_timer;
/*	__asm{
		cli
		hlt
;		PUSHF
	};*/
		keepvector();
	}
}

void init_int(void)
{
	_disable();
	keepvector = _dos_getvect(VECT);
	_dos_setvect(VECT, int_timer);

	outp(0x43, 0x36);
	outp(0x40, fm_timer % 256);
	outp(0x40, fm_timer / 256);

	outp(EOI_M, EOIDATA);
	_enable();
}

void int_end(void)
{
	_disable();
	_dos_setvect(VECT, keepvector);

	outp(0x43, 0x36);
	outp(0x40, 0xff);
	outp(0x40, 0xff);

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

	set_fm(1, 0x20);
	set_fm(2, 0x00);
	set_fm(3, 0x00);
	set_fm(4, 0xe0);
	set_fm(0xb, 0x00);

	outp(BASEIO + 4,0x22);
	outp(BASEIO + 5,0xff);
	outp(BASEIO + 4,0x26);
	outp(BASEIO + 5,0xff);

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
		set_tone(noise, i);
	}
	LOOPTIME = no / 256;


	init_int();
	getchar();

	stop();


#ifdef DEBUG
	i = 0;
	while(1){
		if(mml[i] == KEY_END)
			break;

		set_key(mml[i], ch);	/* key on */

		getchar();
		set_fm(0xb0 + ch, 0x0); /* key off */

		++i;
	}
#endif
	return 0;
}
