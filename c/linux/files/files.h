#ifndef FILES_H
#define FILES_H

int file_save(const unsigned char *data, int len, const char *recipient,
              const char *sender, char *file_id);
int file_load(const char *file_id, unsigned char **data, int *len);

#endif
