#include "files.h"
#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

static int file_counter = 0;

int file_save(const unsigned char *data, int len, const char *recipient,
              const char *sender, char *file_id)
{
    char dir[MAX_LINE];
    snprintf(dir, sizeof(dir), "%s/files/%s", STORAGE_DIR, recipient);
    mkdir(dir, 0700);

    file_counter++;
    snprintf(file_id, 64, "f%d_%s", file_counter, sender);

    char fname[MAX_LINE];
    snprintf(fname, sizeof(fname), "%s/%s", dir, file_id);

    FILE *fp = fopen(fname, "wb");
    if (!fp) return -1;
    fwrite(data, 1, len, fp);
    fclose(fp);
    return 0;
}

int file_load(const char *file_id, unsigned char **data, int *len)
{
    // Search all recipient directories
    char dir[MAX_LINE];
    snprintf(dir, sizeof(dir), "%s/files", STORAGE_DIR);

    char fname[MAX_LINE];
    snprintf(fname, sizeof(fname), "%s/*/%s", dir, file_id);

    // Simple approach: just search common storage
    char path[MAX_LINE];
    snprintf(path, sizeof(path), "%s/files/%s", STORAGE_DIR, file_id);

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        // Try searching in subdirectories
        DIR *dp = opendir(dir);
        if (!dp) return -1;
        struct dirent *e;
        while ((e = readdir(dp)) != NULL) {
            if (e->d_type != DT_DIR) continue;
            snprintf(path, sizeof(path), "%s/%s/%s", dir, e->d_name, file_id);
            fp = fopen(path, "rb");
            if (fp) break;
        }
        closedir(dp);
        if (!fp) return -1;
    }

    fseek(fp, 0, SEEK_END);
    *len = ftell(fp);
    rewind(fp);
    *data = malloc(*len);
    fread(*data, 1, *len, fp);
    fclose(fp);
    return 0;
}
