//
// Created by meitu on 2017/3/24.
//

#ifndef MTAF_MMAPPED_FILE_H
#define MTAF_MMAPPED_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mtaf_mmapped_file {
    int mmapFd;
    int offset;
    int size;
    bool isOpen;
    unsigned char *start;
} mtaf_mmapped_file;

int mtaf_open_mmap(struct mtaf_mmapped_file *mt, const char *filePath, int mapSize, bool isRead);

void mtaf_write_mmap(struct mtaf_mmapped_file *mt, const char *data, int len);

void mtaf_read_mmap(struct mtaf_mmapped_file *mt, const char *data, int len);

void mtaf_close_mmap(struct mtaf_mmapped_file *mt);


#ifdef __cplusplus
}
#endif

#endif // MTAF_MMAPPED_FILE_H
