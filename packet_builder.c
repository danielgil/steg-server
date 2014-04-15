#include "httpd.h"
#include "http_config.h"

#include "config.h"
#include "utils.h"
#include "packet_builder.h"

// Dispatcher function that reads the config vector and calls the appropiate decoder for incoming requests
void packet_decoder(request_rec *r, steg_config *config, server_config *svr){
    //In the prototype, only straight head injection steganography will be used
    if(!strcasecmp(config->inputmethod, "Header")){
        header_decoder(r, config, svr);
        return;
    }
}

// Dispatcher function that reads the config vector and calls the appropiate encoder for outgoing responses
void packet_encoder(request_rec *r, steg_config *config, server_config *svr){
    //In the prototype, only straight head injection steganography will be used
    if(!strcasecmp(config->outputmethod, "Header")){
        header_encoder(r, config, svr);
        return;
    }
}


void header_decoder(request_rec *r, steg_config *config, server_config *svr){

    const char *injected_header;
    char *x;
    char length_field[PROTOCOL_LENGTH_SIZE+1]; //We need 1 extra byte for the null terminator
    long length_value;
    char payload[PROTOCOL_MAX_PAYLOAD_SIZE]; 

    // Get the value of the HTTP header
    injected_header = apr_table_get(r->headers_in, config->inputmethodconfig);

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


void header_encoder(request_rec *r, steg_config *config, server_config *svr){

    char *x;
    char payload[PROTOCOL_MAX_PAYLOAD_SIZE]; 
    char* data = "asdfasdfasdf";

    //First part of the payload is the knockcode
    safe_strcpy(payload, config->knockcode, 256);
    x = payload + strlen(config->knockcode);

    //Then we append the length field
    x = int_to_string(x, strlen(data), PROTOCOL_LENGTH_SIZE);

    // Then the data
    safe_strcpy(x, data, 256-strlen(config->knockcode)-PROTOCOL_LENGTH_SIZE);  

    // Get the value of the HTTP header
    //const char *original_header;
    //original_header = apr_table_get(r->headers_out, config->outputmethodconfig);   
    

    //string_to_inject* = apr_palloc(svr->pool, strlen(original_header) + strlen(payload) )
    apr_table_set(r->headers_out, config->outputmethodconfig, payload);

}
