#include <stdio.h>
#include <stdlib.h>


void DumpMBR(u_int8_t* mbr_record) {
	printf("\"");

	for (int c=0; c < 512; c++) {
		printf("\\x%02x", mbr_record[c]);

		if ((c + 1) % 16 == 0 && (c + 1) != 512) {
			printf("\"\n\"");
		}
	}

	printf("\"\n");
}


int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: extract_mbr <image-file-containing-mbr>\n");
		exit(-1);
	}

	FILE* fh;
    u_int8_t mbr_record[512];

    fh = fopen(argv[1], "r");
    fread(mbr_record, 1, 512, fh);
	fclose(fh);

	DumpMBR(mbr_record);
}


