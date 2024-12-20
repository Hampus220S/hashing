/*
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-09-13
 */

#ifndef HASHING_H
#define HASHING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>

extern char** files_create(size_t* count, char** paths, size_t path_count, int depth);

extern void   files_free(char** files, size_t count);


extern size_t files_size_get(char** files, size_t count);

extern int    files_read(void* pointer, size_t size, char** files, size_t count);


extern size_t file_size_get(const char* filepath);

extern size_t file_read(void* pointer, size_t size, const char* filepath);

#endif // HASHING_H
