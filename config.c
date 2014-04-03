#include "httpd.h"
#include "http_config.h"

#include "config.h"

/* Handler for the "stegEnabled" directive */
const char *steg_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
    steg_config    *conf = (steg_config *) cfg;

    if(conf)
    {
        if(!strcasecmp(arg, "on"))
            conf->enabled = 1;
        else
            conf->enabled = 0;
    }
    return NULL;
}

/* Handler for the "stegInputFile" directive */
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        strcpy(conf->inputfile, arg);
    }
    return NULL;
}

/* Handler for the "stegOutputFile" directive */
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        strcpy(conf->outputfile, arg);
    }
    return NULL;
}

/* Handler for the "stegKnockCode" directive */
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        strcpy(conf->knockcode, arg);
    }
    return NULL;
}

/* Handler for the "stegMethod" directive */
const char *steg_set_method(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        strcpy(conf->method, method);
        strcpy(conf->methodconfig, methodconfig);
    }
    return NULL;
}
