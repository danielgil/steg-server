#include "httpd.h"
#include "http_log.h"
#include "apr_file_io.h"
#include "math.h"

#include "utils.h"
#include "config.h"

extern module AP_MODULE_DECLARE_DATA steg_module;

// Write buffer to file. Comodity wrapper around various APR calls
int write_inputfile(const char *buffer, request_rec *r, const char *filename)
{
    apr_file_t *file;
    apr_status_t rv;

    rv = apr_file_open(&file, filename, APR_FOPEN_CREATE|APR_FOPEN_WRITE|APR_FOPEN_APPEND|APR_FOPEN_BINARY|APR_FOPEN_XTHREAD, APR_FPROT_OS_DEFAULT, r->pool);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to open file %s", filename) ;
        return rv;
    }

    apr_size_t size = strlen(buffer);
    rv = apr_file_write(file, buffer, &size);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to write file %s", filename) ;
        return rv;
    }
    size = 1;
    rv = apr_file_write(file, "\n", &size);

    rv = apr_file_close(file);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to close file %s", filename) ;
        return rv;
    }

    return APR_SUCCESS;
}

// Write buffer to file. Comodity wrapper around various APR calls
int readline_outputfile(char *buffer, server_rec *s)
{
    apr_status_t rv, mutex_rv;
    server_config* cfg = ap_get_module_config(s->module_config, &steg_module);

    // Start critical section
    #if APR_HAS_THREADS
        mutex_rv = apr_thread_mutex_lock(cfg->mutex);
        if (mutex_rv != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_ERR, mutex_rv, s, "Failed to acquire thread mutex");
            return mutex_rv;
        }
    #endif
    rv = apr_file_gets(buffer, 256, cfg->output_fd);
    rv = apr_file_gets(buffer, 256, cfg->output_fd);
    #if APR_HAS_THREADS
        mutex_rv = apr_thread_mutex_unlock(cfg->mutex);
        if (mutex_rv != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_ERR, mutex_rv, s, "Failed to release thread mutex");
        }
    #endif
    // End critical section
    return rv;
}



// Turn value into a string, padding it with 0 to the left, e.g. 5 -> "0005". Returns a pointer to the next position
char* int_to_string(char* dest, int value, int maxsize){
    int length;

    //Pad the field with '0'
    memset(dest, '0', maxsize);

    //Find out how many digits 'value' has, since sprintf return the number of bytes written
    char aux[100];    
    length = sprintf(aux, "%d", value);

    //Write the number in the last 'length' positions
    sprintf(dest + maxsize - length, "%d", value);

    //Return a pointer to the next position in memory
    return dest+maxsize;
}





