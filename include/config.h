
#define CONFIG_FIELD_SIZE 256

/* Configuration structure */
typedef struct {
    int   enabled;      /* Enable or disable our module */
    char  inputfile[CONFIG_FIELD_SIZE];    /* File where we read the outgoing hidden info from */
    char  outputfile[CONFIG_FIELD_SIZE];   /* File where we write the incoming hidden info to */
    char  knockcode[CONFIG_FIELD_SIZE];    /* Pre-shared key to identify a steganogram */
    char  method[CONFIG_FIELD_SIZE];       /* Steganography method to be used */
    char  methodconfig[CONFIG_FIELD_SIZE]; /* Configuration of the steganography method */
} steg_config;

/* Configuration Directive handlers */
const char *steg_set_enabled(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_method(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);
