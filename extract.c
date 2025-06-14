#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// NOTE: strings inside this sturct are not null terminated except `ustar`
struct tar_header {
    char file_path[100];
    char file_mode[8];
    char owner_id[8];
    char group_id[8];
    char file_size[12];
    char last_mod_time[12];
    char checksum[8];
    char type[1];
    // NOTE: Should type be a char[] or char
    char linked_file_name[100];
    char ustar[6];                  // "ustar\0"
    char ustar_version[2];          // "00"
    char owner_user_name[32];
    char owner_group_name[32];
    char dev_major[8];
    char dev_minor[8];
    char file_prefix[155];
    char padding[12];
};

struct tar_entry {
    struct tar_header *header;
    long offset; // file data offset
};

void fill_buff(FILE *fp, char *buff, size_t size)
{
    fread(buff, sizeof(char), size, fp);
}

// NOTE: imporve this maybe?
#define FILL_FIELD(field) fill_buff(fp, header->field, sizeof(header->field))
void fill_header(FILE *fp, struct tar_header *header)
{
    FILL_FIELD(file_path);
    FILL_FIELD(file_mode);
    FILL_FIELD(owner_id);
    FILL_FIELD(group_id);
    FILL_FIELD(file_size);
    FILL_FIELD(last_mod_time);
    FILL_FIELD(checksum);
    FILL_FIELD(type);
    FILL_FIELD(linked_file_name);
    FILL_FIELD(ustar);
    FILL_FIELD(ustar_version);
    FILL_FIELD(owner_user_name);
    FILL_FIELD(owner_group_name);
    FILL_FIELD(dev_major);
    FILL_FIELD(dev_minor);
    FILL_FIELD(file_prefix);
    FILL_FIELD(padding);
}

int main(int argc, char* argv[])
{
    FILE *fp;
    if (argc > 1) {
        fp = fopen(argv[1], "r");
    } else {
        return 1;
    }

    while (!feof(fp)) {
        struct tar_entry *entry;
        entry = (struct tar_entry *)malloc(sizeof(*entry));
        entry->header = (struct tar_header *)malloc(sizeof(*(entry->header)));

        fill_header(fp, (entry->header));
        entry->offset = ftell(fp);
        
        // entry->header->file_size is not null terminated
        // which may cause strtol to not work properly
        // so we copy it into a null terminated buffer
        char ascii_file_size_buff[13];
        memcpy(ascii_file_size_buff, (entry->header)->file_size, sizeof((entry->header)->file_size));
        ascii_file_size_buff[13] = '\0';

        long file_size = strtol(ascii_file_size_buff, NULL, 8);
        size_t chunks = (file_size + 511) / 512;
        size_t padded_size = chunks * 512;
        printf("file_size: %d chunks:  %d padded_size: %d \n", file_size, chunks, padded_size);
        
        // skip file data
        // fseek(fp, SEEK_CUR, padded_size-1);
        
        // read into buffer and print buffer
        char buffer[padded_size+1];
        fread(&buffer, sizeof(char), padded_size, fp);
        buffer[padded_size] = '\0';
        printf("file data: \n%s\n", buffer);

        free(entry->header);
        free(entry);
    }
}
