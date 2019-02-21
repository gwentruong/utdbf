#include <stdio.h>
#include <stdlib.h>

typedef struct header {
    int   record_num;
    short header_size;
    short record_size;
} Header;

typedef struct field {
    char          type;
    unsigned char len;
} Field;

Header *parse_header(FILE *fp);
Field  *parse_field(FILE *fp);
void    print_nth_record(FILE *fp, Header *header, int n);
void    parse_int32(unsigned char *buf, int *p, int n);
void    parse_int16(unsigned char *buf, short *p, int n);

int main(int argc, char **argv)
{
    char *filename = argv[1];
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    Header *header = parse_header(fp);
    int num_fields = (header->header_size - 33) / 32;
    printf("Total number of fields\t%d\n", num_fields);

    Field **array = malloc(sizeof(Field *) * num_fields);
    printf("\n>>>> Field descriptor array\n");

    for (int i = 0; i < num_fields; i++)
        array[i] = parse_field(fp);

    for (int i = 0; i < num_fields; i++)
        free(array[i]);
    free(array);
    free(header);
    fclose(fp);
    return 0;
}

Header *parse_header(FILE *fp)
{
    Header *head = malloc(sizeof(Header));
    unsigned char header[32];
    int num;
    short head_record[2];

    fread(header, sizeof(header), 1, fp);
    parse_int32(header + 4, &num, 1);           // Parse num of records
    parse_int16(header + 8, head_record, 2);    // Parse num of bytes in header & record

    head->record_num = num;
    head->header_size = head_record[0];
    head->record_size = head_record[1];

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

    return head;
}

Field *parse_field(FILE *fp)
{
    Field *fd = malloc(sizeof(Field));
    unsigned char field[32];
    fread(field, sizeof(field), 1, fp);
    if (field[0] == 0x0D)
        return NULL;

    fd->type = field[11];
    fd->len  = field[16];

    printf("\nField name       \t");
    for (int i = 0; i < 11; i++)
        printf("%c", field[i]);
    printf("\n");

    printf("Field type         \t%c\n", field[11]);
    // 4 bytes reserved
    printf("Field length       \t%u\n", field[16]);
    printf("Field decimal count\t%u\n", field[17]);
    // 2 bytes reserved
    printf("Work area ID        \t0x%02x\n", field[20]);
    // 10 bytes reserved
    printf("MDX flag            \t0x%02x ", field[31]);
    if (field[31] == 0x01)
        printf("(has index tag)\n");
    else
        printf("(no index tag)\n");

    return fd;
}

void print_nth_record(FILE *fp, Header *header, int n)
{
    int size = header->record_num * header->record_size;
    unsigned char *records = malloc(size);
    fread(records, sizeof(records), 1, fp);


    printf("Total size %d\n", size);

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
