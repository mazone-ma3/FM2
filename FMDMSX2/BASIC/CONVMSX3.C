/* BIN->MSX BLOAD CONV. for GCC By m@3 */

#include <stdio.h>

FILE *stream[2];

int conv(char *loadfil, char*savefil)
{
	long i;
	unsigned char pattern[10];
	unsigned char pattern2[65536];
	unsigned short size, address, run;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	if ((stream[1] = fopen( savefil, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil);

		fclose(stream[1]);
		return 1;
	}

	pattern[0] = 0xfe;
	fwrite(pattern, 1, 1, stream[1]);	/* MSX先頭ヘッダ 0xFE */

	address = 0xd900;
	run = 0;

	pattern[0] = address % 256;;
	pattern[1] = address / 256;
	fwrite(pattern, 1, 2, stream[1]);	/* MSXヘッダ 開始番地 */

	size = (fread(pattern2, 1, 65536, stream[0]));
	address += (size - 1);

	pattern[0] = address % 256;
	pattern[1] = address / 256;
	fwrite(pattern, 1, 2, stream[1]);	/* MSXヘッダ 終了番地 */

	pattern[0] = run % 256;;
	pattern[1] = run / 256;
	fwrite(pattern, 1, 2, stream[1]);	/* MSXヘッダ 実行番地 */

	i = fwrite(pattern2, 1, size, stream[1]);

	fclose(stream[0]);
	fclose(stream[1]);

	return 0;
}


int	main(int argc,char **argv){

	if (argv[1] == NULL)
		return 1;
	if (argv[2] == NULL)
		return 1;

	conv(argv[1], argv[2]);

	return 0;
}
