// FM COMPILER(MSX-BASIC版から移植) By m@3 2024/8/28-
// mml.hにMMLを記述してコンパイルして実行するとMSX形式のdummy.pd2を出力する。
// (ファイル名変更指定可能)
// 必要に応じてconv2.exeでPC-88形式に変換する。

//-------------------------------------------
// ＜データ型式＞
// 0DAFFH番地から0番地の方向に入っていく
// 各演奏コマンドは数値で指示(可変長)
// 
// ＜コマンド一覧＞
// 0-96,n     音(音長n、nは60/n秒間)、0は休符
// 128-224    音(音長指定無し)、128は休符
// 225,m,n    SOUND m,n Y
// 226,n      VOLUME=n、n=16ならエンベローブ
// 227,n      LENGTH=n、128-224のコマンドの音長
// 228.n      音色変更@
// 254        全てのパートの同期を取る
// 255        ダ・カーポ(全パート)
//-------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ADDRESS = 0xd9ff;
int address = 0xd9ff;

#define PARTSUU 12

int TEMPO = 120;
int LENGTH = 4;
int OCT = 4;
int VOL = 9;
long double dZE = 0, dZR = 0, dMA = 0, dME = 0, dMA2 = 0;

int ZB;
int PE[3];

int F;
int FM;

int SA;

int BT;

char *pStr;

int H;

int VALUE, MAXVALUE, MINVALUE;
int NUM;
int LN;

char outputdata[65536];

char chr;

int tmp;
int j;

int data_num = 0;

#include "mml.h"

// 警告音
void beep(void)
{
}

// エラー表示して強制終了
void error(void)
{
	printf("DATA ERROR.%d \n", H);
	beep();
	exit(1);
}

// サウンドデータ書き込み
void poke(int data)
{
	if(data >= 256)
		error();
	outputdata[address--] = data % 256;
//	printf("%.2x ",data & 255);
}


//	文字列を数値に変換する
void sub(void)
{
//_460:
	H = H + 1;
	chr = pStr[H];

	if(((chr < '0') || (chr > '9')) &&
		(chr != 'O')){
		H = H - 1;
		return;
	}else{
//		printf("%c", chr);
		if((chr >= '0') && (chr <= '9'))
			VALUE = chr - '0';
	}

//_480:
	while(1){
		chr = pStr[H+1];
		if((chr >= '0') && (chr <= '9')){
//			printf("%c", chr);
			VALUE = VALUE * 10 + (chr - '0');
			H = H + 1;
//			goto _480;
		}else if((VALUE > MAXVALUE) || (VALUE < MINVALUE)){
			error();
		}else{
			printf("[%d]",VALUE);
			return;
		}
	}
//	printf("(%d)",VALUE);
//	return;
}

//	音指定の処理
void oto(int data)
{
	if(data == 1)	// 休符
		NUM = 0;
	else
		NUM = (OCT - 1) * 12 + data - 1;

	while(1){
		chr = pStr[H + 1];
		if((chr == '+') || (chr == '#')){
			NUM = NUM + 1;
			if(NUM > 96)
				error();
			else{
				H=H+1;
				continue;
			}
		}
		if(chr == '-'){
			NUM=NUM-1;
			if(NUM < 1)
				error();
			else{
				H=H+1;
				continue;
			}
		}
		break;
	}

	if((chr >= '1') && (chr <= '9')){
		VALUE = -1;
		MAXVALUE = 64;
		MINVALUE = 1;
		sub();
		if(VALUE != -1){
			LN = VALUE;
			chr = pStr[H+1];
		}else{
//			LN=LENGTH;
//			H=H-1;
			error();
		}
	}else{
		LN = 0;
	}
//	printf("<%d>",LN);

	if(LN == 0){	// 音長なし
//		printf("<%d>",LN);
		if((NUM == 0) || (chr == '.') || (chr == '&')){
			if(NUM == 0)
				LN = 4;
			else
				LN = LENGTH;
			printf("(%d)",LN);
			goto _210;
		}
		LN = LENGTH;

		dME = 14400 / (LENGTH * TEMPO);
		dMA = dME;// + dMA;
		dMA += dMA2;
		tmp = (int)dMA;
		dZE = dMA - tmp;
		if((dZE < 1) && (dMA2 == 0)){
			printf("%d/",LENGTH);
			dZR = dZE;
			poke(NUM+128);	// 音長指定しない場合
			return;
		}
		printf("%d ",LENGTH);
//		dMA -= dMA2;
		goto _230;
	}

_210:
	if(!LN){
		printf("(lengh 0)");
		error();
	}
//	printf("<%d>",LN);
	dMA = 14400 / (LN * TEMPO);
//	dME = 14400 / (LENGTH * TEMPO);
//	dMA = dME;// + dMA;
_220:
	while(1){
		chr = pStr[H + 1];
		if(chr == '.'){
			printf(".");
			dMA = dMA * 1.5;
			H = H + 1;
		}else if(chr == '&'){
			printf("&"); //,LN);
			dMA2 += dMA;
			H = H + 1;
			return;
		}else{
//			dMA += dME;
			dMA += dMA2;
			break;
		}
	}
_230:
//	dMA += dMA2;
	tmp = (int)dMA;
	dZR = dZR + dMA - tmp; // 誤差
	if(dZR >= 1){
		tmp = (int)dZR;
		dMA = dMA + tmp;
		dZR = dZR - tmp;
	}
	poke(NUM);	// コマンド
//	if(!NUM)
//		printf("%x/", dMA);
	poke(dMA);	// 音長指定
	dMA2 = 0;
}

// 本体
int	main(int argc,char **argv)
{
	ZB = -1;
	PE[0] = 0;
	PE[1] = 0;
	PE[2] = 0;
	F = 0;
	FM = 0;

	while(1){
		for(j = 0; j < PARTSUU; ++j){
			SA = address;
			printf("\n%d %d ", FM, j);

			TEMPO = 120;
			LENGTH = 4;
			OCT = 4;
			VOL = 9;
			dZR = 0;

			while(1){
				pStr =data[data_num];
				data_num++;
//				printf("\n%s \n", pStr);
				if(!strcmp(pStr,"P")){
					poke(254);	/* パート終了 */
					goto _110;
				}else if (!strcmp(pStr, "!")){
					beep();
					SA = 0;
//					--datanum;
//					++j;
					j = PARTSUU;
					goto _110;
				}
				else if(!strcmp(pStr, "END") || !strcmp(pStr, "*"))
					goto comp_end;

				for(H = 0; H < strlen(pStr); ++H){
					chr = pStr[H];
					printf("%c", chr);
					if(chr >= 'a' &&  chr <= 'z'){
						chr -= 32;
					}else if (chr == ' '){
						continue;
//						goto _120;
					}

					switch(chr){
						case 'R':
							oto(1);
							break;
						case 'C':
							oto(2);
							break;
						case 'D':
							oto(4);
							break;
						case 'E':
							oto(6);
							break;
						case 'F':
							oto(7);
							break;
						case 'G':
							oto(9);
							break;
						case 'A':
							oto(11);
							break;
						case 'B':
							oto(13);
							break;

						case '>':
							if(OCT < 8)
								OCT = ++OCT;
							break;

						case '<':
							if(OCT > 1)
								OCT = --OCT;
							break;

						case 'O':
							VALUE = 4;
							MAXVALUE = 8;
							MINVALUE = 1;
							sub();
							OCT = VALUE;
							break;

						case 'L':
							VALUE = 4;
							MAXVALUE = 64;
							MINVALUE = 1;
							sub();
							LENGTH = VALUE;
							poke(227);
							dME = 14400 / (LENGTH * TEMPO);
							tmp = (int)dME;
							poke(tmp);
							break;

						case 'T':
							VALUE = 120;
							MAXVALUE = 255;
							MINVALUE = 80;
							sub();
							TEMPO = VALUE;
							poke(227);
							dMA = 14400 / (LENGTH * TEMPO);
							poke(dMA);
							break;

						case 'V':
							VALUE = 4;
							MAXVALUE = 16;
							MINVALUE = 0;
							sub();
							VOL = VALUE;
							poke(226);
							poke(VOL);
							break;

						case 'S':
							VALUE = 0;
							MAXVALUE = 16;
							MINVALUE = 0;
							sub();
							poke(226);
							poke(16+VALUE);
							break;

						case 'M':
							VALUE = 8000;
							MAXVALUE = 32767;
							MINVALUE = 0;
							sub();
							poke(225);
							poke(11);
							poke(VALUE & 255);
							poke(225);
							poke(12);
							poke(VALUE / 256);
							break;

						case 'N':
							break;

						case 'Y':
							VALUE = 0;
							MAXVALUE = 255;
							MINVALUE = 0;
							sub();
							poke(225);
							poke(VALUE);
							break;

						case ',':
							VALUE = 0;
							MAXVALUE = 255;
							MINVALUE = 0;
							sub();
							poke(VALUE);
							break;

						case '@':
							VALUE = 0;
							MAXVALUE = 255;
							MINVALUE = 0;
							sub();
							poke(228);
							poke(VALUE);
							break;

						default:
							printf("\nIllegal COMMAND = %c", chr);
							error();
							break;
					}
				}

				poke(254);	/* パート終了 */
			}

// HEADER ADRESS WRITE;
_110:
			poke(255);	/* 曲終了 */

_120:
//			BT = 0xda00 + ( PARTSUU - 1 - j) * 2 + FM * 2 * 8;
			BT = 0xda00 + j * 2 + FM * 2 * PARTSUU;
			outputdata[BT] = SA % 256;
			outputdata[BT+1] = SA / 256;
			printf("[%x]",SA);

		}
		FM = FM+1;
	}
// SAVE;
comp_end:
	printf("\nSTART %x END %x\n", address, BT + 2 * PARTSUU - 1);

//	for(j = 65535; j > address; --j){
/*	for(j = address+1; j < (BT+7); ++j){
		printf("%.2x ",outputdata[j] & 255);
	}*/

	char *savefil;
	if (argv[1] == NULL)
		savefil = "dummy.pd2";
//		return 1;
	else
		savefil = argv[1];

	FILE *stream[2];
	unsigned char dummy[2] = {0, 0};

	if ((stream[0] = fopen( savefil, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil);

		fclose(stream[0]);
		return 1;
	}

	unsigned char pattern[10];

	pattern[0] = 0xfe;
	fwrite(pattern, 1, 1, stream[0]);	/* MSX先頭ヘッダ 0xFE */

	int run = 0;

	pattern[0] = address % 256;;
	pattern[1] = address / 256;
	fwrite(pattern, 1, 2, stream[0]);	/* MSXヘッダ 開始番地 */

	pattern[0] = (BT + 2 * PARTSUU - 1) % 256;
	pattern[1] = (BT + 2 * PARTSUU - 1) / 256;
	fwrite(pattern, 1, 2, stream[0]);	/* MSXヘッダ 終了番地 */

	run = address;

	pattern[0] = run % 256;;
	pattern[1] = run / 256;
	fwrite(pattern, 1, 2, stream[0]);	/* MSXヘッダ 実行番地 */

	fwrite(&outputdata[address], 1, (BT + 2 * PARTSUU - 1 - address - 1), stream[0]);
	fclose(stream[0]);

	return 0;
}
