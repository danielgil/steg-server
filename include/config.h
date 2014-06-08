#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FIELD_SIZE 256

#define SHM_FILE "logs/shm_file"
#define SHM_LOCKFILE "logs/shm_lockfile"

#define PROTOCOL_MAX_PAYLOAD_SIZE 256
#define PROTOCOL_LENGTH_SIZE 3

/* Per-dir configuration */
typedef struct {
    char  knockcode[CONFIG_FIELD_SIZE];    /* Pre-shared key to identify a steganogram */
    char  inputmethod[CONFIG_FIELD_SIZE];       /* Steganography method to be used */
    char  inputmethodconfig[CONFIG_FIELD_SIZE]; /* Configuration of the steganography method */
    char  outputmethod[CONFIG_FIELD_SIZE];       /* Steganography method to be used */
    char  outputmethodconfig[CONFIG_FIELD_SIZE]; /* Configuration of the steganography method */

} steg_config;

/* The structure that is stored in shared memory */
typedef struct {
    // 'Present' decoder
    unsigned int outputfile_offset;
    unsigned int bit_offset;
    unsigned char present_byte;
    char knockcode[CONFIG_FIELD_SIZE];
    char length[PROTOCOL_LENGTH_SIZE+1];
    char payload[PROTOCOL_MAX_PAYLOAD_SIZE];

    // 'Present' encoder
    char steganogram[CONFIG_FIELD_SIZE + PROTOCOL_MAX_PAYLOAD_SIZE + PROTOCOL_LENGTH_SIZE];
    unsigned int steganogram_offset;

} shared_mem;

/* Per-server configuration */
typedef struct {
    char  inputfile[CONFIG_FIELD_SIZE];    /* File where we read the outgoing hidden info from */
    char  outputfile[CONFIG_FIELD_SIZE];   /* File where we write the incoming hidden info to */
    apr_pool_t *pool;                      /* APR pool shared between requests */
    apr_file_t *output_fd;                 /* The output file will be read line by line on each response, so the file descriptor is shared */

    // Shared memory fields
    char *shm_file;
    char *shm_lockfile;
    apr_global_mutex_t *shm_mutex; /* the cross-thread/cross-process mutex */
    apr_shm_t *shm_steg;   /* the APR shared segment object */
    shared_mem *shm_memory;  /* the per-process address of the segment */
} server_config;


/* Configuration Directive handlers */
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_shmlockfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_shmfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_inputmethod(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);
const char *steg_set_outputmethod(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);
#endif
