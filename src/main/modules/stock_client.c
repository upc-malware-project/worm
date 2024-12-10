/*
    C file for requesting the (hardcoded) pseudo-stock value of Microsoft and 
    the percentage of change since the previous measurement.
    
    In case the stock value dropped more than 10% (percentage of change < 10%), 
    the attack is triggered.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Server URL for Microsoft stock data
#define MICROSOFT_STOCK_URL "http://172.17.0.1:8000/stock/microsoft"

// Function to check if wget is installed on the system
int is_wget_installed() {
    FILE *fp;
    char buffer[256];
    // Try running 'which wget' to check if wget is installed
    fp = popen("which wget", "r");
    if (fp == NULL) {
        // If 'which wget' fails, wget is not installed
        return 0;
    }
    // Read output of 'which wget'
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // If wget's path is found, it's installed
        pclose(fp);
        return 1;
    }
    // wget is not installed
    pclose(fp);
    return 0;
}

// Function to obtain the stock value of Microsoft and the percentage of change.
// In case the stock value dropped more than 10% (percentage of change < 10%),
// the attack is triggered.
int get_microsoft_stock() {
    char command[512];
    FILE *fp;
    char buffer[256];
    char symbol[10];
    int current_stock_value;
    float percentage_change;
    int parse_result;

    // Check if wget is installed
    if (!is_wget_installed()) {
        printf("\e[33mWARNING:\e[0m Wget is not installed\n");
        printf("\e[31mTriggering attack...\e[0m\n");
        return 0;
    }

    // Construct the wget command
    // -q for quiet mode, -O - to output to stdout
    snprintf(command, sizeof(command), "wget -q -O - %s", MICROSOFT_STOCK_URL);

    // Open a pipe to the wget command
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        return -1;
    }

    // Read the response from the wget command
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Parse the response (format: symbol,current_stock_value,percentage_change)
        parse_result = sscanf(buffer, "%9[^,],%d,%f", symbol, &current_stock_value, &percentage_change);
        if (parse_result == 3) { // Upon obtaining 3 values...
            // Print stock value and percentage of change
            printf("\e[32mMicrosoft stock value:\e[0m $%d\n", current_stock_value);
            printf("\e[32mPercentage of change:\e[0m %.2f%%\n", percentage_change);

            // Check for a 10% drop to trigger the attack
            if (percentage_change < -10) {
                printf("\e[33mWARNING:\e[0m Microsoft stock value dropped more than 10%%!\n");
                printf("\e[31mTriggering attack...\e[0m\n");
            }
        } else {
            printf("Failed to parse response. Parse result: %d\n", parse_result);
        }
    } else {
        printf("Failed to retrieve data.\n");
    }

    // Close the pipe
    pclose(fp);
    return 0;
}

// Main function
int main() {
    get_microsoft_stock();
    return 0;
}