#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FIELD_SIZE 256

/* Configuration structure */
typedef struct {
    char  inputfile[CONFIG_FIELD_SIZE];    /* File where we read the outgoing hidden info from */
    char  outputfile[CONFIG_FIELD_SIZE];   /* File where we write the incoming hidden info to */
    char  knockcode[CONFIG_FIELD_SIZE];    /* Pre-shared key to identify a steganogram */
    char  inputmethod[CONFIG_FIELD_SIZE];       /* Steganography method to be used */
    char  inputmethodconfig[CONFIG_FIELD_SIZE]; /* Configuration of the steganography method */
    char  outputmethod[CONFIG_FIELD_SIZE];       /* Steganography method to be used */
    char  outputmethodconfig[CONFIG_FIELD_SIZE]; /* Configuration of the steganography method */

} steg_config;

typedef struct {
    apr_pool_t *pool;                      /* APR pool shared between requests */
    #if APR_HAS_THREADS
    apr_thread_mutex_t *mutex;             /* Mutex to ensure thread safety on the shared pool */
    #endif
} server_config;

/* Configuration Directive handlers */
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_inputmethod(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);
const char *steg_set_outputmethod(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);
#endif
