#include <stdio.h>

void parse_header(FILE *fp);
int  parse_field(FILE *fp);
void parse_int32(unsigned char *buf, int *p, int n);
void parse_int16(unsigned char *buf, short *p, int n);

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

    printf("\n>>>> Field descriptor array\n");
    while (parse_field(fp) != -1)
        ;

    fclose(fp);
    return 0;
}

void parse_header(FILE *fp)
{
    unsigned char header[32];
    int num;
    short head_record[2];

    fread(header, sizeof(header), 1, fp);
    parse_int32(header + 4, &num, 1);           // Parse num of records
    parse_int16(header + 8, head_record, 2);    // Parse num of bytes in header & record

    printf("File type\t\t0x%02x\n", header[0]); //0x03 dBase IV memo file
    printf("YYYY MM DD\t\t%i %02i %02i\n", header[1] + 1900, header[2], header[3]);
    printf("Number of records     \t%d\n", num);
    printf("Num of bytes in header\t%hu\n", head_record[0]);
    printf("Num of bytes in record\t%hu\n", head_record[1]);
    // 2 bytes reserved
    printf("Incomplete transaction flag %02x\n", header[14]);
    printf("Encryption flag        \t%02x\n", header[15]);
    // 12 bytes reserved
    printf("Production MDX flag    \t%02x ", header[28]);
    if (header[28] == 0x01)
        printf("(has MDX)\n");
    else
        printf("(no MDX)\n");
    printf("Language driver ID     \t%02x\n", header[29]);
    // 2 bytes reserved
}

int parse_field(FILE *fp)
{
    unsigned char field[32];
    int i;
    fread(field, sizeof(field), 1, fp);
    if (field[0] == 0x0D)
        return -1;

    printf("\nField name                 \t");
    for (i = 0; i < 11; i++)
        printf("%c", field[i]);
    printf("\n");

    printf("Field type                   \t%c\n", field[11]);
    // 4 bytes reserved
    printf("Field length in binary       \t%u\n", field[16]);
    printf("Field decimal count in binary\t%u\n", field[17]);
    // 2 bytes reserved
    printf("Work area ID                 \t0x%02x\n", field[20]);
    // 10 bytes reserved
    printf("MDX flag                     \t0x%02x ", field[31]);
    if (field[31] == 0x01)
        printf("(has index tag)\n");
    else
        printf("(no index tag)\n");

    return 0;
}


void parse_int32(unsigned char *buf, int *p, int n)
{
    int *q = (int *)buf;
    for (int i = 0; i < n; i++, q++)
        p[i] = *q;
}

void parse_int16(unsigned char *buf, short *p, int n)
{
    short *q = (short *)buf;
    for (int i = 0; i < n; i++, q++)
        p[i] = *q;
}
