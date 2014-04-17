#include "httpd.h"
#include "http_config.h"
#include "apr_strings.h"
#include "apr_global_mutex.h"
#include "apr_shm.h"

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

/* Handlers for the shared memory settings. Can be usually be left to default */
const char *steg_set_shmlockfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    server_rec* s = cmd->server;
    server_config* conf = ap_get_module_config(s->module_config, &steg_module);

    conf->shm_lockfile = ap_server_root_relative(cmd->pool, arg);
    return NULL;
}

const char *steg_set_shmfile(cmd_parms *cmd, void *cfg, const char *arg)
{
    server_rec* s = cmd->server;
    server_config* conf = ap_get_module_config(s->module_config, &steg_module);

    conf->shm_file = ap_server_root_relative(cmd->pool, arg);
    return NULL;
}

