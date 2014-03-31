#include "httpd.h"
#include "http_config.h"

#include "config.h"

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
