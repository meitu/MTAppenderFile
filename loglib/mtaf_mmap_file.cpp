//
// Created by meitu on 2017/3/24.
//
#include "mtaf_mmap_file.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "fcntl.h"

int mtaf_open_mmap(mtaf_mmapped_file *mt, const char *filePath, int mapSize, bool isRead) {
    if (!mt) {
        printf("[error] try to open MMap while mtaf_mmapped_file is null");
        return -1;
    }

    if (mt && mt->isOpen) {
        mtaf_close_mmap(mt);
    }
    if (isRead) {
        mt->mmapFd = open(filePath, O_RDWR);
    } else {
        mt->mmapFd = open(filePath, O_CREAT | O_RDWR, 0666);
    }
    if (mt->mmapFd < 0) {
        int code = errno; // EINVAL
        printf("[error] mmap open failed, code: %d, filepath: %s\n", code, filePath);
        if (code == ENOENT) {
            printf("[error] please check the directory\n");
        }
        return -1;
    }
    mt->isOpen = true;
    ftruncate(mt->mmapFd, mapSize);
    mt->start = (unsigned char *)mmap(0, (size_t)mapSize, PROT_WRITE, MAP_SHARED, mt->mmapFd, 0);
    mt->offset = 0;
    mt->size = mapSize;
    return 0;
}

void mtaf_write_mmap(mtaf_mmapped_file *mt, const char *data, int len) {
    memcpy(mt->start + mt->offset, data, len);
    mt->offset += len;
}

void mtaf_read_mmap(mtaf_mmapped_file *mt, const char *data, int len) {
    memcpy((void *)data, mt->start + mt->offset, len);
    mt->offset += len;
}

void mtaf_close_mmap(mtaf_mmapped_file *mt) {
    mt->offset = 0;
    munmap(mt->start, (size_t)mt->size);
    close(mt->mmapFd);
    mt->mmapFd = -1;
    mt->isOpen = false;
}
