/* 
**  mod_steg.c -- Apache sample steg module
**
**    Compile:
**    $ apxs -c -i mod_steg.c
**
**    Configure:
**    LoadModule steg_module modules/mod_steg.so
**    <Location /steg>
**        SetHandler steg
**    </Location>
**
**    Test:
**    $ lynx -mime_header http://localhost/steg 
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


/* Configuration structure */
typedef struct {
    int         enabled;      /* Enable or disable our module */
    const char *inputfile;    /* File where we read the outgoing hidden info from */
    const char *outputfile;   /* File where we write the incoming hidden info to */
    const char *knockcode;    /* Pre-shared key to identify a steganogram */
    const char *method;       /* Steganography method to be used */
    const char *methodconfig; /* Configuration of the steganography method */
} steg_config;

/* Instance the configuration structure */
static steg_config config;

/* Handler for the "stegEnabled" directive */
const char *steg_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
    if(!strcasecmp(arg, "on")) config.enabled = 1;
    else config.enabled = 0;
    return NULL;
}

/* Handler for the "stegInputFile" directive */
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    config.inputfile = arg;
    return NULL;
}

/* Handler for the "stegOutputFile" directive */
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    config.outputfile = arg;
    return NULL;
}

/* Handler for the "stegKnockCode" directive */
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg)
{
    config.knockcode = arg;
    return NULL;
}

/* Handler for the "stegMethod" directive */
const char *steg_set_method(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig)
{
    config.method = method;
    config.methodconfig = methodconfig;
    return NULL;
}

/* Table of configuration directives */
static const command_rec        steg_directives[] =
{
    AP_INIT_TAKE1("stegEnabled", steg_set_enabled, NULL, RSRC_CONF, "Enable or disable mod_steg"),
    AP_INIT_TAKE1("stegInputFile", steg_set_inputfile, NULL, RSRC_CONF, "File path where we read the outgoing hidden info from"),
    AP_INIT_TAKE1("stegOutputFile", steg_set_outputfile, NULL, RSRC_CONF, "File path where we write the incoming hidden info to"),
    AP_INIT_TAKE1("stegKnockCode", steg_set_knockcode, NULL, RSRC_CONF, "Set the knock code to recognize steganograms"),
    AP_INIT_TAKE2("stegMethod", steg_set_method, NULL, RSRC_CONF, "Configure the steganography method to be used"),
    { NULL }
};




/* Input Filter function */
static apr_status_t stegInputFilter(ap_filter_t *f,
                                      apr_bucket_brigade *bb,
                                      ap_input_mode_t mode,
                                      apr_read_type_e block,
                                      apr_off_t readbytes)
{
    apr_status_t rv;
    const char *injected_header; //In the prototype, only straight head injection steganography will be used

    // Get the request_rec object from the filter object
    request_rec *r = f->r;
    
    // Get the value of the HTTP header
    injected_header = apr_table_get(r->headers_in, config.methodconfig);


    apr_file_t *file;
    rv = apr_file_open(&file, config.inputfile, APR_FOPEN_CREATE|APR_FOPEN_WRITE|APR_FOPEN_APPEND|APR_FOPEN_BINARY|APR_FOPEN_XTHREAD, APR_FPROT_OS_DEFAULT, r->pool);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to open file %s", config.inputfile) ;
        return rv;
    }

    apr_size_t size = strlen(injected_header);
    rv = apr_file_write(file, injected_header, &size);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to write file %s", config.inputfile) ;
        return rv;
    }

    rv = apr_file_close(file);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to close file %s", config.inputfile) ;
        return rv;
    }

    ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Header value: %s", injected_header) ;
    
    return ap_get_brigade(f->next, bb, mode, block, readbytes);
}




/* The sample content handler that will eventually be deleted */
static int steg_handler(request_rec *r)
{

    //if (!r->handler || strcmp(r->handler, "steg-handler")) return(DECLINED);
    ap_set_content_type(r, "text/plain");
    ap_rprintf(r, "Enabled     : %u\n", config.enabled);
    ap_rprintf(r, "Input File  : %s\n", config.inputfile);
    ap_rprintf(r, "Output File : %s\n", config.outputfile);
    ap_rprintf(r, "Knock code  : %s\n", config.knockcode);
    ap_rprintf(r, "Steganography method: %s\n", config.method);
    ap_rprintf(r, "Steganography configuration: %s\n", config.methodconfig);
    return OK;
}

static void steg_register_hooks(apr_pool_t *p)
{
    // Default configuration
    config.enabled    = 1;
    config.inputfile  = "/tmp/steginput";
    config.outputfile = "/tmp/stegoutput";
    config.knockcode  = "111111";
    config.method    = "header";
    config.methodconfig = "Accept-Encoding";

    // Register the handler
    ap_hook_handler(steg_handler, NULL, NULL, APR_HOOK_LAST);

    //Register the input filter
    ap_register_input_filter("StegInput",stegInputFilter, NULL, AP_FTYPE_RESOURCE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA steg_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                 /* create per-dir    config structures */
    NULL,                 /* merge  per-dir    config structures */
    NULL,                 /* create per-server config structures */
    NULL,                 /* merge  per-server config structures */
    steg_directives,      /* table of config file commands       */
    steg_register_hooks   /* register hooks                      */
};








