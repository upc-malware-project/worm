// C program representing a blockchain client.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

// Base URL of the blockchain server
#define BASE_URL "http://localhost:8000"

// Structure to dynamically store HTTP response data
struct MemoryStruct {
    char *memory;  // Pointer to response data
    size_t size;   // Size of response data
};

// Callback function for libcurl to handle response data
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Reallocate memory to fit new data
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    // Copy new data into memory
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;  // Null-terminate for string handling

    return realsize;
}

// Function to add a new transaction to the blockchain
int add_transaction(const char *item, int quantity, const char *location) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    // Initialize memory for response
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(curl) {
        char url[256];
        // Construct full transaction endpoint URL
        snprintf(url, sizeof(url), "%s/transaction", BASE_URL);

        // Prepare JSON payload with transaction details
        char payload[512];
        snprintf(payload, sizeof(payload), 
            "{\"item\":\"%s\",\"quantity\":%d,\"location\":\"%s\"}", 
            item, quantity, location);

        // Set HTTP headers for JSON content
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Configure CURL options for POST request
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform the HTTP request
        res = curl_easy_perform(curl);

        // Handle request errors
        if(res != CURLE_OK) { 
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return -1;
        }

        // Parse JSON response
        struct json_object *parsed_json;
        parsed_json = json_tokener_parse(chunk.memory);
        
        // Extract and print transaction result
        struct json_object *message, *block_index;
        json_object_object_get_ex(parsed_json, "message", &message);
        json_object_object_get_ex(parsed_json, "block_index", &block_index);
        
        printf("Transaction Result: %s (Block Index: %d)\n", 
               json_object_get_string(message),
               json_object_get_int(block_index));

        // Cleanup allocated resources
        json_object_put(parsed_json);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}

// Function to retrieve stock information from the blockchain
int get_stock_info(const char *item) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    // Initialize memory for response
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(curl) {
        char url[256];
        // Construct stock info URL with optional item filter
        snprintf(url, sizeof(url), "%s/stock_info%s%s", 
                 BASE_URL, 
                 item ? "?item=" : "", 
                 item ? item : "");

        // Configure CURL for GET request
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform the HTTP request
        res = curl_easy_perform(curl);

        // Handle request errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            curl_easy_cleanup(curl);
            return -1;
        }

        // Parse JSON response
        struct json_object *parsed_json;
        parsed_json = json_tokener_parse(chunk.memory);
        
        // Check if response is a JSON array
        if (json_object_get_type(parsed_json) == json_type_array) {
            int array_length = json_object_array_length(parsed_json);
            printf("Stock Information for %s:\n", item ? item : "All Items");

            // Iterate through and print stock information
            for (int i = 0; i < array_length; i++) {
                struct json_object *stock_item = json_object_array_get_idx(parsed_json, i);
                
                // Extract item details
                struct json_object *j_item, *j_quantity, *j_location;
                json_object_object_get_ex(stock_item, "item", &j_item);
                json_object_object_get_ex(stock_item, "quantity", &j_quantity);
                json_object_object_get_ex(stock_item, "location", &j_location);

                // Print stock item details
                printf("Item: %s, Quantity: %d, Location: %s\n", 
                       json_object_get_string(j_item),
                       json_object_get_int(j_quantity),
                       json_object_get_string(j_location));
            }
        }

        // Cleanup allocated resources
        json_object_put(parsed_json);
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}

// Function to mine a new block in the blockchain
int mine_block() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    // Initialize memory for response
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(curl) {
        char url[256];
        // Construct mine block URL
        snprintf(url, sizeof(url), "%s/mine", BASE_URL);

        // Configure CURL for GET request
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform the HTTP request
        res = curl_easy_perform(curl);

        // Handle request errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            curl_easy_cleanup(curl);
            return -1;
        }

        // Parse JSON response
        struct json_object *parsed_json;
        parsed_json = json_tokener_parse(chunk.memory);
        
        // Extract and print block mining result
        struct json_object *message, *index;
        json_object_object_get_ex(parsed_json, "message", &message);
        json_object_object_get_ex(parsed_json, "index", &index);
        
        printf("Block Mining Result: %s (Block Index: %d)\n", 
               json_object_get_string(message),
               json_object_get_int(index));

        // Cleanup allocated resources
        json_object_put(parsed_json);
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }

    curl_global_cleanup();
    return 0;
}

int main(int argc, char *argv[]) {
    // Command-line argument parsing
    if (argc > 1) {
        if (strcmp(argv[1], "add") == 0 && argc == 5) {
            // Add transaction: ./program add ItemName Quantity Location
            add_transaction(argv[2], atoi(argv[3]), argv[4]);
        } else if (strcmp(argv[1], "info") == 0) {
            // Get stock information (all or specific item): ./program info [ItemName]
            get_stock_info(argc > 2 ? argv[2] : NULL);
        } else if (strcmp(argv[1], "mine") == 0) {
            // Mine a new block: ./program mine
            mine_block();
        } else {
            // Print usage instructions
            printf("Usage:\n");
            printf("  Add transaction: %s add ItemName Quantity Location\n", argv[0]);
            printf("  Get stock info: %s info [ItemName]\n", argv[0]);
            printf("  Mine block:     %s mine\n", argv[0]);
        }
    } else {
        printf("Please provide a command. Use -h for help.\n");
    }

    return 0;
}