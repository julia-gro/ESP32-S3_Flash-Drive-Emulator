#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}


unsigned int bit_packing(int byte_count, ...) {
    va_list bytes;
    va_start(bytes, byte_count);

    unsigned int num = 0;

    for (int i=0; i < byte_count; i++) {
        num |= (va_arg(bytes, u_int64_t) << (8 * i));
    }

    va_end(bytes);
    return num;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: mbr_reader <dd-dump-file>\n");
        exit(-1);
    }
    
    FILE* fh;
    u_int8_t mbr_record[512];

    u_int8_t records[4][16];

    fh = fopen(argv[1], "r");
    fread(mbr_record, 1, 512, fh);
    fclose(fh);

    DumpHex(mbr_record, 512);

    // for (int c=0; c < 512; c++) {
    //     printf("%02x ", mbr_record[c]);

    //     if (c % 8 == 0) {
    //         printf(" ");
    //     }

    //     if (c % 16 == 0) {
    //         printf("\n");
    //     }
    // }


    for (int r=0; r < 4; r++) {
        memcpy(records[r], mbr_record + (446 + 16*r), 16);

        printf("                   Record %i              \n", r);
        printf("=============================================\n");
        DumpHex(records[r], 16);
        printf("\n");
    }


    printf("\n");
    printf("\n");
    printf("\n");

    
    for (int r=0; r < 4; r++) {
        u_int32_t sector_count = bit_packing(4, records[r][12], records[r][13], records[r][14], records[r][15]);


        printf("                   Record %i              \n", r);
        printf("=============================================\n");

        printf("0x00: \t %02x \t\t | Status\n", records[r][0]);
        printf("0x01: \t %02x %02x %02x \t | CHS Start \n", records[r][3], records[r][2], records[r][1]);
        printf("0x04: \t %02x \t\t | Partition Type \n", records[r][4]);
        printf("0x05: \t %02x %02x %02x \t | CHS End \n", records[r][7], records[r][6], records[r][5]);
        printf("0x08: \t %02x %02x %02x %02x \t | LBA Start \n", records[r][11], records[r][10], records[r][9], records[r][8]);
        printf("0x0c: \t %02x %02x %02x %02x \t | NUmber of sectors (%i)\n", records[r][15], records[r][14], records[r][13], records[r][12], sector_count);
        printf("\n");
    }


    printf("\n\n");
}
