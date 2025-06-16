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
    char type;
    char linked_file_name[100];
    char ustar[6];
    char ustar_version[2];
    char owner_user_name[32];
    char owner_group_name[32];
    char dev_major[8];
    char dev_minor[8];
    char file_prefix[155];
    char padding[12];
};

struct tar_entry {
    struct tar_header *header;
    long offset;
};

void parse_tar_header(struct tar_header *header, const char *buffer) {
    /* copy header data from buffer */
    memcpy(header->file_path,           buffer +   0,   100);
    memcpy(header->file_mode,           buffer + 100,   8);
    memcpy(header->owner_id,            buffer + 108,   8);
    memcpy(header->group_id,            buffer + 116,   8);
    memcpy(header->file_size,           buffer + 124,   12);
    memcpy(header->last_mod_time,       buffer + 136,   12);
    memcpy(header->checksum,            buffer + 148,   8);
    memcpy(&header->type,                buffer + 156,   1);
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

    char buffer[512];
    while (true) {
        int fread_status = (fread(buffer, 1, 512, fp) == 512);
        if (!fread_status) break;

        struct tar_header header;
        struct tar_entry entry;
        entry.header = &header;

        parse_tar_header(&header, buffer);
        entry.offset = ftell(fp);

        // null terminate header.full_size
        char ascii_file_size[13];
        memcpy(ascii_file_size, header.file_size, 12);
        ascii_file_size[12] = '\0';
        
        // convert file size in ascii octal to decimal
        size_t file_size = strtol(ascii_file_size, NULL, 8);
        size_t chunks = (file_size + 511) / 512;
        size_t total_size = 512 * chunks;

        printf("file_size: %d chunks: %d total_size: %d\n", file_size, chunks, total_size);

        char *data = (char *) malloc(total_size + 1);
        data[total_size] = '\0';
        fread(data, 1, total_size, fp);

        printf("%s\n", data);

        free(data);
    }

    fclose(fp);
    return 0;
}
