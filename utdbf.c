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

typedef struct record {
    unsigned char  is_deleted;
    unsigned char *content;
    struct record *next;
} Record;

Header *parse_header(FILE *fp);
Field  *parse_field(FILE *fp);
Record *parse_record(FILE *fp, Header *header);
void    parse_int32(unsigned char *buf, int *p, int n);
void    parse_int16(unsigned char *buf, short *p, int n);
int     record_prepend(Record **p_head, Record *new_record);
void    record_reverse(Record **p_head);
void    record_free(Record *head);

int main(int argc, char **argv)
{
    char *filename = argv[1];
    int   i;
    int   m;
    int len = 0;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("File not found\n");
        return -1;
    }

    Header *header = parse_header(fp);
    int     num_fields = (header->header_size - 33) / 32;
    printf("Total number of fields\t%d\n", num_fields);

    Field **array = malloc(sizeof(Field *) * num_fields);
    printf("\n>>>> Field descriptor array\n");

    for (i = 0; i < num_fields; i++)
    {
        array[i] = parse_field(fp);
        printf("XXXX array[%d] Type %c, len %i\n", i, array[i]->type, array[i]->len);
    }

    Record *record = NULL;
    while((m = record_prepend(&record, parse_record(fp, header))) == 0)
    {
        printf("XxXXXXXX %d\n", m);
        len++;
    }

    printf("$$$$$$ Done parsing\n");
    printf("###### Number of records %d\n", len);
    record_reverse(&record);
    printf("$$$$$$ Done reversing\n");

    record_free(record);
    for (i = 0; i < num_fields; i++)
        free(array[i]);
    free(array);
    free(header);
    fclose(fp);
    return 0;
}

Header *parse_header(FILE *fp)
{
    Header       *head = malloc(sizeof(Header));
    unsigned char header[32];
    int           num;
    short         head_record[2];

    fread(header, sizeof(header), 1, fp);
    parse_int32(header + 4, &num, 1);           // Parse num of records
    parse_int16(header + 8, head_record, 2);    // Parse num of bytes in header & record

    head->record_num  = num;
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
    Field        *fd = malloc(sizeof(Field));
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

Record *parse_record(FILE *fp, Header *header)
{
    int           size = header->record_size;
    Record       *rec = malloc(sizeof(size));
    unsigned char record[size];

    int nitems = fread(record, sizeof(record), 1, fp);
    printf(">>>>>>>>>>>>>>>>nitem %02x\n", nitems);
    if (nitems != 1)     // EOF
        return NULL;

    rec->is_deleted = record[0];
    rec->next       = NULL;

    rec->content    = malloc(size - 1);
    for (int i = 1, j = 0; i < size; i++, j++)
        rec->content[j] = record[i];

    return rec;
}

int record_prepend(Record **p_head, Record *new_record)
{
    if (new_record == NULL)
        return -1;

    new_record->next = *p_head;
    *p_head = new_record;

    return 0;
}

void record_reverse(Record **p_head)
{
    Record *prev = NULL;
    Record *current = *p_head;
    Record *next;
    int cnt = 0;
    printf("Start reversing\n");
    while (current != NULL)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
        cnt++;
        printf("Success %d\n", cnt);
    }
    printf("Done reversing\n");

    *p_head = prev;
}

void record_free(Record *head)
{
    for (Record *p = head; p != NULL; p = head)
    {
        head = head->next;
        free(p->content);
        free(p);
    }
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
