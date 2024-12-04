/*
    C file for requesting the (hardcoded) pseudo-stock value of Microsoft and 
    the percentage of change since the previous measurement.
    
    In case the stock value dropped more than 10% (percentage of change < 10%), 
    the attack is triggered.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

// Server URL for Microsoft stock data
#define MICROSOFT_STOCK_URL "http://172.17.0.1:8000/stock/microsoft"

// Structure to store memory for curl response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Function to write curl response to memory
static size_t WriteMemory(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Allocate memory for response
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("Memory allocation failed\n");
        return 0;
    }

    // Copy response to memory
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Function to obtain the stock value of Microsoft and the percentage of change.
// In case the stock value dropped more than 10% (percentage of change < 10%), 
// the attack is triggered.
int get_microsoft_stock() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    // Initialize memory for curl response
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Initialize curl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(curl) {
        // Set curl options for request
        curl_easy_setopt(curl, CURLOPT_URL, MICROSOFT_STOCK_URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemory);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform curl request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "Curl request failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            curl_easy_cleanup(curl);
            return -1;
        }

        // Parse JSON response
        struct json_object *parsed_json = json_tokener_parse(chunk.memory);
        
        // Extract current value and percentage of change
        struct json_object *j_current_value, *j_percentage_change;
        json_object_object_get_ex(parsed_json, "current_value", &j_current_value);
        json_object_object_get_ex(parsed_json, "percentage_change", &j_percentage_change);

        // Convert JSON values to C types
        int current_stock_value = json_object_get_int(j_current_value);
        double percentage_change = json_object_get_double(j_percentage_change);

        // Print stock information
        printf("\e[32mMicrosoft stock value:\e[0m $%d\n", current_stock_value);
        printf("\e[32mPercentage of change:\e[0m %.2f%%\n", percentage_change);

        // Check for a 10% drop to trigger the attack
        if (percentage_change < -10) {
            printf("\e[33mWARNING:\e[0m Microsoft stock value dropped more than 10%!\n");
            printf("\e[31mTriggering attack...\e[0m\n");
        }

        // Cleanup
        json_object_put(parsed_json);
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    // Curl global cleanup
    curl_global_cleanup();
    return 0;
}

// Main function
int main() {
    get_microsoft_stock();
    return 0;
}