#include "httpd.h"
#include "http_config.h"
#include "apr_strings.h"

#include "config.h"
#include "utils.h"

extern module AP_MODULE_DECLARE_DATA steg_module;

/* Handler for the "stegInputFile" directive */
const char *steg_set_inputfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    server_rec* s = cmd->server;
    server_config* conf = ap_get_module_config(s->module_config, &steg_module);
    if(conf)
    {
        apr_cpystrn(conf->inputfile, arg, CONFIG_FIELD_SIZE);       
    }
    return NULL;
}

/* Handler for the "stegOutputFile" directive */
const char *steg_set_outputfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    server_rec* s = cmd->server;
    server_config* conf = ap_get_module_config(s->module_config, &steg_module);
    if(conf)
    {
        apr_cpystrn(conf->outputfile, arg, CONFIG_FIELD_SIZE);
    }
    return NULL;
}

/* Handler for the "stegKnockCode" directive */
const char *steg_set_knockcode(cmd_parms *cmd, void *cfg, const char *arg)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        apr_cpystrn(conf->knockcode, arg, CONFIG_FIELD_SIZE);
    }
    return NULL;
}

/* Handler for the "stegInputMethod" directive */
const char *steg_set_inputmethod(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        apr_cpystrn(conf->inputmethod, method, CONFIG_FIELD_SIZE);
        apr_cpystrn(conf->inputmethodconfig, methodconfig, CONFIG_FIELD_SIZE);
    }
    return NULL;
}

/* Handler for the "stegOutputMethod" directive */
const char *steg_set_outputmethod(cmd_parms *cmd, void *cfg, const char *method, const char *methodconfig)
{
    steg_config *conf = (steg_config *) cfg;
    if(conf)
    {
        apr_cpystrn(conf->outputmethod, method, CONFIG_FIELD_SIZE);
        apr_cpystrn(conf->outputmethodconfig, methodconfig, CONFIG_FIELD_SIZE);
    }
    return NULL;
}
