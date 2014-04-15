#include "httpd.h"
#include "http_log.h"
#include "apr_file_io.h"
#include "math.h"

#include "utils.h"


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

    rv = apr_file_close(file);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to close file %s", filename) ;
        return rv;
    }

    return APR_SUCCESS;
}

// Write buffer to file. Comodity wrapper around various APR calls
int readline_outputfile(const char *buffer, request_rec *r, const char *filename)
{
    apr_file_t *file;
    apr_status_t rv;

    rv = apr_file_open(&file, filename, APR_FOPEN_CREATE|APR_FOPEN_READ|APR_FOPEN_BINARY|APR_FOPEN_XTHREAD, APR_FPROT_OS_DEFAULT, r->pool);
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

    rv = apr_file_close(file);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "Failed to close file %s", filename) ;
        return rv;
    }

    return APR_SUCCESS;
}

// Safe string copy
int safe_strcpy(char *dest, const char *source, int maxsize){

    int length=strlen(source);
    if (length >= maxsize){
        strncpy(dest, source, maxsize)[maxsize-1] = '\0';
        return maxsize;
    }else{
        strcpy(dest, source);
        return length;
    }
}

// Turn value into a string, padding it with 0 to the left, e.g. 5 -> "0005". Returns a pointer to the next position
char* int_to_string(char* dest, int value, int maxsize){
    int length;
    memset(dest, '0', maxsize);
    length = floor(log10(value)) + 1;
    //length = 2;
    sprintf(dest + maxsize - length, "%d", value);
    return dest+maxsize;
}





