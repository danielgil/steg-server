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
#include "apr_strings.h"
#include "apr_global_mutex.h"
#include "apr_shm.h"
#include "unixd.h"

#include "config.h"
#include "utils.h"
#include "mod_steg.h"
#include "packet_builder.h"


/* Table of configuration directives */
static const command_rec        steg_directives[] =
{
    AP_INIT_TAKE1("stegInputFile", steg_set_inputfile, NULL, RSRC_CONF, "File path where we read the outgoing hidden info from"),
    AP_INIT_TAKE1("stegOutputFile", steg_set_outputfile, NULL, RSRC_CONF, "File path where we write the incoming hidden info to"),
    AP_INIT_TAKE1("stegShmLockfile", steg_set_shmlockfile, NULL, RSRC_CONF, "Filename of global mutex"),
    AP_INIT_TAKE1("stegShmFile", steg_set_shmfile, NULL, RSRC_CONF, "Filename of shared segment, or NULL for anonymous shared memory"),
    AP_INIT_TAKE1("stegKnockCode", steg_set_knockcode, NULL, ACCESS_CONF, "Set the knock code to recognize steganograms"),
    AP_INIT_TAKE2("stegInputMethod", steg_set_inputmethod, NULL, ACCESS_CONF, "Configure the steganography method to be used in incoming requests"),
    AP_INIT_TAKE2("stegOutputMethod", steg_set_outputmethod, NULL, ACCESS_CONF, "Configure the steganography method to be used in outgoing responses"),
    AP_INIT_TAKE1("stegCryptKey", steg_set_cryptkey, NULL, RSRC_CONF, "AES Key to encrypt the payloads"),
    AP_INIT_TAKE1("stegCryptIV", steg_set_cryptiv, NULL, RSRC_CONF, "AES Inizialization vector to encrypt the payloads"),
    AP_INIT_TAKE1("stegCryptEnabled", steg_set_cryptenabled, NULL, RSRC_CONF, "Whether to enable cryptography"),
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

    // Register the child_init thread
    ap_hook_child_init(stegChildInit, NULL, NULL, APR_HOOK_MIDDLE);

    // Register the post_config hook for the shared memory
    ap_hook_post_config(stegPostConfig, NULL, NULL, APR_HOOK_REALLY_FIRST);
}

// Post-config hook to setup the shared memory and global mutex
static int stegPostConfig(apr_pool_t *pool, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
    apr_status_t rv;
    server_config *cfg;
    void *data = NULL;
    const char *userdata_key = "shm_counter_post_config";

    apr_pool_userdata_get(&data, userdata_key, s->process->pool);
    if (data == NULL) {
        apr_pool_userdata_set((const void *)1, userdata_key, apr_pool_cleanup_null, s->process->pool);
        return OK;
    }

    /* If we made it this far, we can safely initialize the module */

    cfg = ap_get_module_config(s->module_config, &steg_module);

    rv = apr_global_mutex_create(&cfg->shm_mutex, cfg->shm_lockfile, APR_LOCK_DEFAULT, pool);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to create mod_steg global mutex file '%s'", cfg->shm_lockfile);
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    rv = ap_unixd_set_global_mutex_perms(cfg->shm_mutex);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to set permissions on mutex lockfile '%s'", cfg->shm_lockfile);
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    rv = apr_shm_create(&cfg->shm_steg, sizeof(*cfg->shm_memory), cfg->shm_file, pool);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to create mod_shm_counter shared segment file '%s'", cfg->shm_file ? cfg->shm_file : "NULL");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    cfg->shm_memory = apr_shm_baseaddr_get(cfg->shm_steg);   
    memset(cfg->shm_memory, 0, sizeof(*cfg->shm_memory));

    return OK;
}


// Called during initialization. Here we can setup the shared memory and mutex.
static void stegChildInit(apr_pool_t *pchild, server_rec *s){

    apr_status_t rv;
    
    /* Get the config vector. The pool and mutex fields are uninitialized */
    server_config* cfg = ap_get_module_config(s->module_config, &steg_module);
  
    /* Derive our own pool from pchild */
    rv = apr_pool_create(&cfg->pool, pchild);

    if (rv != APR_SUCCESS) {
        ap_log_perror(APLOG_MARK, APLOG_CRIT, rv, pchild, "Failed to create shared pool for steg_module: %d", rv);
        return;
    }

    rv = apr_global_mutex_child_init(&cfg->shm_mutex, cfg->shm_lockfile, pchild); 
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to attach to mod_steg global mutex file '%s'", cfg->shm_lockfile);
        return;
    }

    if (!cfg->shm_steg) {
        rv = apr_shm_attach(&cfg->shm_steg, cfg->shm_file, pchild);
        if (rv != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to attach to mod_steg shared memory file '%s'", cfg->shm_file ? cfg->shm_file : "NULL");
            return;
        }
    }

    cfg->shm_memory = apr_shm_baseaddr_get(cfg->shm_steg);
}

// Create the per-directory configuration
void *create_dir_conf(apr_pool_t *pool, char *context) {
    steg_config *cfg = apr_pcalloc(pool, sizeof(steg_config));
    if(cfg) {
        // Default configuration
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
    apr_cpystrn(cfg->inputfile, "/var/steg/input", CONFIG_FIELD_SIZE);
    apr_cpystrn(cfg->outputfile, "/var/steg/output", CONFIG_FIELD_SIZE);
    cfg->cryptenabled = 0;
    memset(cfg->key, 0, AES_KEY_SIZE);
    memset(cfg->iv, 0, AES_IV_SIZE);

    // Initialize shared memory with default values
    cfg->shm_file = ap_server_root_relative(pool, SHM_FILE);
    cfg->shm_lockfile = ap_server_root_relative(pool, SHM_LOCKFILE);
    cfg->shm_mutex = NULL;
    cfg->shm_steg = NULL;
    cfg->shm_memory = NULL;

    return cfg;
}

// Merge parent directory configurations into sub-directory configurations
void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD){
    steg_config    *base = (steg_config *) BASE;
    steg_config    *add = (steg_config *) ADD;
    char *context = "merged context";
    steg_config    *cfg = (steg_config *) create_dir_conf(pool, context);

    apr_cpystrn(cfg->knockcode, strlen(add->knockcode) ? add->knockcode : base->knockcode, CONFIG_FIELD_SIZE);
    apr_cpystrn(cfg->inputmethod, strlen(add->inputmethod) ? add->inputmethod : base->inputmethod, CONFIG_FIELD_SIZE);
    apr_cpystrn(cfg->inputmethodconfig, strlen(add->inputmethodconfig) ? add->inputmethodconfig : base->inputmethodconfig, CONFIG_FIELD_SIZE);
    apr_cpystrn(cfg->outputmethod, strlen(add->outputmethod) ? add->outputmethod : base->outputmethod, CONFIG_FIELD_SIZE);
    apr_cpystrn(cfg->outputmethodconfig, strlen(add->outputmethodconfig) ? add->outputmethodconfig : base->outputmethodconfig, CONFIG_FIELD_SIZE);
    return cfg;
}

/* The debug content handler. For now it just prints the configuration read in httpd.conf */
static int steg_handler(request_rec *r)
{

    steg_config *config = (steg_config*) ap_get_module_config(r->per_dir_config, &steg_module);

    //if (!r->handler || strcmp(r->handler, "steg-handler")) return(DECLINED);
    ap_set_content_type(r, "text/plain");
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






