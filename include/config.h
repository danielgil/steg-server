
/* Configuration structure */
typedef struct {
    int         enabled;      /* Enable or disable our module */
    const char *inputfile;    /* File where we read the outgoing hidden info from */
    const char *outputfile;   /* File where we write the incoming hidden info to */
    const char *knockcode;    /* Pre-shared key to identify a steganogram */
    const char *method;       /* Steganography method to be used */
    const char *methodconfig; /* Configuration of the steganography method */
} steg_config;

/* Declaration of the 'config' variable that will store the config */
extern steg_config config;

/* Configuration Directive handlers */
const char *steg_set_enabled(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg);
const char *steg_set_method(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig);

