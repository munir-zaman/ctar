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
    // NOTE: Should `type` be a char[] or char?
    char type[1];
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

void fread_header(struct tar_header *header, FILE *fp) {
    /* Reads header data into `header` from `fp`*/

    char buffer[512]; // 512 byte buffer
    size_t bytes_read = fread(buffer, 1, 512, fp); 
    if (bytes_read != 512) {
        // couldn't read 512 bytes
        fprintf(stderr, "fread_header error: couldn't read header data.");
        return;
    }

    memcpy(header->file_path,           buffer +   0,   100);
    memcpy(header->file_mode,           buffer + 100,   8);
    memcpy(header->owner_id,            buffer + 108,   8);
    memcpy(header->group_id,            buffer + 116,   8);
    memcpy(header->file_size,           buffer + 124,   12);
    memcpy(header->last_mod_time,       buffer + 136,   12);
    memcpy(header->checksum,            buffer + 148,   8);
    memcpy(header->type,                buffer + 156,   1);
    memcpy(header->linked_file_name,    buffer + 157,   100);
    memcpy(header->ustar,               buffer + 257,   6);
    memcpy(header->ustar_version,       buffer + 263,   2);
    memcpy(header->owner_user_name,     buffer + 265,   32);
    memcpy(header->owner_group_name,    buffer + 297,   32);
    memcpy(header->dev_major,           buffer + 329,   8);
    memcpy(header->dev_minor,           buffer + 337,   8);
    memcpy(header->file_prefix,         buffer + 345,   155);
    memcpy(header->padding,             buffer + 500,   12);

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

        // read 512 bytes into `entry->header` from `fp`
        fread_header(entry->header, fp);
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
