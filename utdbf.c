#include <stdio.h>

void parse_header(FILE *fp);
void parse_int32(unsigned char *buf, int *p, int n, int big_endian);
void parse_int16(unsigned char *buf, short *p, int n, int big_endian);

int main(int argc, char **argv)
{
    char *filename = argv[1];
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    parse_header(fp);

    fclose(fp);
    return 0;
}

void parse_header(FILE *fp)
{
    unsigned char header[68];
    int i;
    int num;
    short head_record[2];

    fread(header, sizeof(header), 1, fp);
    parse_int32(header + 4, &num, 1, 0);
    parse_int16(header + 8, head_record, 2, 0);

    printf(" File type 0x%02x\n", header[0]);   //0x03 FoxBase+/DBase III+, no memo

    printf("YYMMDD\n");                         // Print date
    for (i = 1; i < 4; i++)
        printf("%02x", header[i]);
    printf("\n");

    printf("Number of records: %d\n", num);
    printf("Num of bytes in header %hu\n", head_record[0]);
    printf("Num of bytes in record %hu\n", head_record[1]);

    // 2 bytes reserved (12-13)
    printf("Flag #14 %02x\tFlag #15 %02x\n", header[14], header[15]);
    // 12 bytes reserved
    printf("Flag #28 %02x\tDriverID  %02x\n", header[28], header[29]);
    // 2 bytes reserved

    printf("Language driver name\n");
    for (i = 32; i < 64; i++)
        printf("0x%02x ", header[i]);
    printf("\n");
    // 4 bytes reserved
}

void parse_int32(unsigned char *buf, int *p, int n, int big_endian)
{
    if (big_endian == 0)            // Little endian
    {
        int *q = (int *)buf;
        for (int i = 0; i < n; i++, q++)
            p[i] = *q;
    }
    else                            // Big endian
    {
        for (int k = 0; k < n; k++, buf += 4)
        {
            unsigned char b[4];
            for (int i = 0, j = 3; i < 4; i++, j--)
                b[j] = buf[i];
            int *q = (int *)b;
            p[k] = *q;
        }
    }
}

void parse_int16(unsigned char *buf, short *p, int n, int big_endian)
{
    if (big_endian == 0)
    {
        short *q = (short *)buf;
        for (int i = 0; i < n; i++, q++)
            p[i] = *q;
    }
    else
    {
        for (int i = 0; i < n; i++, buf += 2)
        {
            unsigned char b[2];
            b[1] = buf[0];
            b[0] = buf[1];
            short *q = (short *)b;
            p[i] = *q;
        }
    }
}
