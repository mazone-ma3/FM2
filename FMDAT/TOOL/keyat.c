#include <stdio.h>

#define MAX_STR 12

char *str[MAX_STR] = {"C","CP", "D", "DP", "E", "F", "FP", "G", "GP", "A", "AP", "B"};


int key[MAX_STR] = {
	343, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647
};

void main(void)
{
	int i,j;
//	printf("key_table:\n");
	printf("unsigned char key_table[%d][2] = {\n", MAX_STR * 8);

	for(i = 1; i <= 8; ++i){
		for(j = 0; j < MAX_STR; ++j){
//			printf("db\t%.3xh, %.3xh\t; %s%d\n", (i - 1) * 2 | key[i] / 256, key[i] % 256, str[j], i);
			printf("\t{\t0x%.3x,\t0x%.3x\t},\t// %s%d\n", (i - 1) * 4 | key[j] / 256, key[j] % 256, str[j], i);
		}
	}
	printf("};\n");
}
