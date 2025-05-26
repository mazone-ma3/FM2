#include <stdio.h>

enum {
	REG_0x20,
	REG_0x23,
	REG_0x40,
	REG_0x43,
	REG_0x60,
	REG_0x63,
	REG_0x80,
	REG_0x83,

	REG_0xc0,

	REG_0xe0,
	REG_0xe3,

	REG_MAX
};

enum {
	FB,
	CON,
	WAV1,
	WAV2,
	DUM0,
	DUM1,
	DUM2,
	DUM3,

	KSR1,
	EGT1,
	VIB1,
	AM1,
	MUL1,
	KBL1,
	TL1,
	AR1,
	DR1,
	SL1,
	RR1,

	KSR2,
	EGT2,
	VIB2,
	AM2,
	MUL2,
	KBL2,
	TL2,
	AR2,
	DR2,
	SL2,
	RR2,

	MAX
};

#define MAX_TONE 4

unsigned char tone_org[MAX_TONE][MAX] = {
	{	/* Trumpet */
		6, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 0, 1, 0,38, 3, 0, 2,13,
		0, 1, 1, 0, 1, 2,10, 4, 2, 3,11,
	},
	{	/* Flute */
		2, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 0, 5, 3,37, 8, 0, 0, 7,
		1, 1, 1, 0, 1, 1,10, 6, 0, 0,14,
	},
	{	/* Marinba */
		5, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 5, 1,24,10,10, 2, 7,
		0, 0, 0, 0, 1, 0,13,15,10, 1, 5,
	},
	{	/* Base1 */
		3, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 0,22,13, 1, 2, 7,
		0, 1, 0, 0, 1, 0,24,15, 1, 1, 8,
	},
};

unsigned char tone_dst[REG_MAX];

void main(void)
{
	printf("unsigned char tone_table[%d][%d] = {\n", MAX_TONE, REG_MAX);

	for(int j = 0; j < MAX_TONE; ++j){
		unsigned char *tone_src = tone_org[j];
		tone_dst[REG_0x20] = tone_src[AM1] * 128 | tone_src[VIB1] * 64 | tone_src[EGT1] * 32 | tone_src[KSR1] * 16 | tone_src[MUL1];
		tone_dst[REG_0x23] = tone_src[AM2] * 128 | tone_src[VIB2] * 64 | tone_src[EGT2] * 32 | tone_src[KSR2] * 16 | tone_src[MUL2];
		tone_dst[REG_0x40] = tone_src[KSR1] * 64 | tone_src[TL1];
		tone_dst[REG_0x43] = tone_src[KSR2] * 64 | tone_src[TL2];
		tone_dst[REG_0x60] = tone_src[AR1] * 16 | tone_src[DR1];
		tone_dst[REG_0x63] = tone_src[AR2] * 16 | tone_src[DR2];
		tone_dst[REG_0x80] = tone_src[SL1] * 16 | tone_src[RR1];
		tone_dst[REG_0x83] = tone_src[SL2] * 16 | tone_src[RR2];
		tone_dst[REG_0xc0] = tone_src[FB] * 2 | tone_src[CON];
		tone_dst[REG_0xe0] = tone_src[WAV1];
		tone_dst[REG_0xe3] = tone_src[WAV2];

		printf("\t{");
		for(int i = 0; i < REG_MAX; ++i){
			printf(" %3d,", tone_dst[i]);
		}
		printf(" },\n");
	}
	printf("};\n");
}
