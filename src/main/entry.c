#include "scanner.h"
#include "propagate.h"


void * start_propagate(void *varg){
    propagate((Globals *) varg);
}
void * start_network_scanner(void *varg){
    scan_net((Globals *) varg);
}

void entry(Globals *global) {
    // start propagate
    pthread_t thread_id_propagate;
    global->pthread_create(&thread_id_propagate, NULL, start_propagate, global);

    // start network scanner
    pthread_t thread_id_scanner;
    global->pthread_create(&thread_id_scanner, NULL, start_network_scanner, global);

    global->fprintf(global->stdout, "Started all modules!\n");
    // keep running
    while(1){
        //global->fprintf(global->stdout, "Sleeping with my eyes open *.*\n");
        global->sleep(1);
    }
}