
/* Configuration structure */
typedef struct {
    int   enabled;      /* Enable or disable our module */
    char  inputfile[256];    /* File where we read the outgoing hidden info from */
    char  outputfile[256];   /* File where we write the incoming hidden info to */
    char  knockcode[256];    /* Pre-shared key to identify a steganogram */
    char  method[256];       /* Steganography method to be used */
    char  methodconfig[256]; /* Configuration of the steganography method */
} steg_config;

/* Configuration Directive handlers */
const char *steg_set_enabled(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_method(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);

