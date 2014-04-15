/* 
**  mod_steg.c -- Server component of the HTTP Steganography project.
**
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "apr_hash.h"
#include "util_filter.h"
#include "http_log.h"
#include "apr_file_io.h"

#include "utils.h"
#include "config.h"
#include "mod_steg.h"
#include "packet_builder.h"


/* Table of configuration directives */
static const command_rec        steg_directives[] =
{
    AP_INIT_TAKE1("stegInputFile", steg_set_inputfile, NULL, ACCESS_CONF, "File path where we read the outgoing hidden info from"),
    AP_INIT_TAKE1("stegOutputFile", steg_set_outputfile, NULL, ACCESS_CONF, "File path where we write the incoming hidden info to"),
    AP_INIT_TAKE1("stegKnockCode", steg_set_knockcode, NULL, ACCESS_CONF, "Set the knock code to recognize steganograms"),
    AP_INIT_TAKE2("stegInputMethod", steg_set_inputmethod, NULL, ACCESS_CONF, "Configure the steganography method to be used in incoming requests"),
    AP_INIT_TAKE2("stegOutputMethod", steg_set_outputmethod, NULL, ACCESS_CONF, "Configure the steganography method to be used in outgoing responses"),
    { NULL }
};

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA steg_module = {
    STANDARD20_MODULE_STUFF, 
    create_dir_conf,
    merge_dir_conf,
    create_server_conf,
    NULL,
    steg_directives,      /* table of config file commands       */
    steg_register_hooks   /* register hooks                      */
};

static void steg_register_hooks(apr_pool_t *p)
{
    // Register the handler
    ap_hook_handler(steg_handler, NULL, NULL, APR_HOOK_LAST);

    //Register the input filter
    ap_register_input_filter("StegInput",stegInputFilter, NULL, AP_FTYPE_RESOURCE);

    //Register the output filter
    ap_register_output_filter("StegOutput",stegOutputFilter, NULL, AP_FTYPE_RESOURCE);

    // Register the child_init thread, where we initialize shared memory
    ap_hook_child_init(stegChildInit, NULL, NULL, APR_HOOK_MIDDLE);
}



// Called during initialization. Here we can setup the shared memory and mutex.
static void stegChildInit(apr_pool_t *pchild, server_rec *s){

    apr_status_t rv;
    
    /* Get the config vector. The pool and mutex fields are uninitialized */
    server_config* cfg = ap_get_module_config(s->module_config, &steg_module);
  
    /* Derive our own pool from pchild */
    rv = apr_pool_create(&cfg->pool, pchild);

    if (rv != APR_SUCCESS) {
        ap_log_perror(APLOG_MARK, APLOG_CRIT, rv, pchild, "Failed to create shared pool for steg_module");
        return;
    }

    /* Mutex to ensure thread safety */
    #if APR_HAS_THREADS
    rv = apr_thread_mutex_create(&cfg->mutex, APR_THREAD_MUTEX_DEFAULT, pchild);
    if (rv != APR_SUCCESS) {
        ap_log_perror(APLOG_MARK, APLOG_CRIT, rv, pchild, "Failed to create mutex for steg_module");
        return;
    }
    #endif
}

// Create the per-directory configuration
void *create_dir_conf(apr_pool_t *pool, char *context) {
    steg_config *cfg = apr_pcalloc(pool, sizeof(steg_config));
    if(cfg) {
        // Default configuration
        strcpy(cfg->inputfile, "/var/steg/input");
        strcpy(cfg->outputfile, "/var/steg/output");
        strcpy(cfg->knockcode, "");
        strcpy(cfg->inputmethod, "");
        strcpy(cfg->inputmethodconfig, "");
        strcpy(cfg->outputmethod, "");
        strcpy(cfg->outputmethodconfig, "");
    }
    return cfg;
}

// Create the per-server configuration
void *create_server_conf(apr_pool_t *pool, server_rec *s) {
    server_config *cfg = apr_pcalloc(pool, sizeof(server_config));

    return cfg;
}

// Merge parent directory configurations into sub-directory configurations
void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD){
    steg_config    *base = (steg_config *) BASE;
    steg_config    *add = (steg_config *) ADD;
    char *context = "merged context";
    steg_config    *cfg = (steg_config *) create_dir_conf(pool, context);

    strcpy(cfg->inputfile, strlen(add->inputfile) ? add->inputfile : base->inputfile);
    strcpy(cfg->outputfile, strlen(add->outputfile) ? add->outputfile : base->outputfile);
    strcpy(cfg->knockcode, strlen(add->knockcode) ? add->knockcode : base->knockcode);
    strcpy(cfg->inputmethod, strlen(add->inputmethod) ? add->inputmethod : base->inputmethod);
    strcpy(cfg->inputmethodconfig, strlen(add->inputmethodconfig) ? add->inputmethodconfig : base->inputmethodconfig);
    strcpy(cfg->outputmethod, strlen(add->outputmethod) ? add->outputmethod : base->outputmethod);
    strcpy(cfg->outputmethodconfig, strlen(add->outputmethodconfig) ? add->outputmethodconfig : base->outputmethodconfig);
    return cfg;
}

/* The debug content handler. For now it just prints the configuration read in httpd.conf */
static int steg_handler(request_rec *r)
{

    steg_config *config = (steg_config*) ap_get_module_config(r->per_dir_config, &steg_module);

    //if (!r->handler || strcmp(r->handler, "steg-handler")) return(DECLINED);
    ap_set_content_type(r, "text/plain");
    ap_rprintf(r, "Input File  : %s\n", config->inputfile);
    ap_rprintf(r, "Output File : %s\n", config->outputfile);
    ap_rprintf(r, "Knock code  : %s\n", config->knockcode);
    ap_rprintf(r, "Steganography Input method: %s\n", config->inputmethod);
    ap_rprintf(r, "Steganography Input configuration: %s\n", config->inputmethodconfig);
    ap_rprintf(r, "Steganography Output method: %s\n", config->outputmethod);
    ap_rprintf(r, "Steganography Output configuration: %s\n", config->outputmethodconfig);
    return OK;
}

/* Input Filter function */
static apr_status_t stegInputFilter(ap_filter_t *f,
                                      apr_bucket_brigade *bb,
                                      ap_input_mode_t mode,
                                      apr_read_type_e block,
                                      apr_off_t readbytes)
{
    // Get the request_rec object from the filter object
    request_rec *r = f->r;

    // Get the per-directory config
    steg_config *config = (steg_config*) ap_get_module_config(r->per_dir_config, &steg_module);

    // Get the server config to be able to use shared memory
    server_config *svr = (server_config*) ap_get_module_config(r->server->module_config, &steg_module);
    
    // Call the packet builder to analyze the data and decode the steganogram
    packet_decoder(r, config, svr);

    // Return the unmodified bucket brigade. This makes the filter transparent.
    return ap_get_brigade(f->next, bb, mode, block, readbytes);
}

/* Output Filter function */
static apr_status_t stegOutputFilter(ap_filter_t *f, apr_bucket_brigade *bb)
{
    // Get the request_rec object from the filter object
    request_rec *r = f->r;

    // Get the per-directory config
    steg_config *config = (steg_config*) ap_get_module_config(r->per_dir_config, &steg_module);

    // Get the server config to be able to use shared memory
    server_config *svr = (server_config*) ap_get_module_config(r->server->module_config, &steg_module);
    
    // Call the packet builder to analyze the data and decode the steganogram
    packet_encoder(r, config, svr);

    // Return the unmodified bucket brigade. This makes the filter transparent.
    return ap_pass_brigade(f->next, bb);
}






