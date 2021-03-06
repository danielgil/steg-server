#include "httpd.h"
#include "http_log.h"
#include "apr_file_io.h"
#include "math.h"
#include "apr_global_mutex.h"
#include "apr_shm.h"
#include "openssl/aes.h"

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
    mutex_rv = apr_global_mutex_lock(cfg->shm_mutex);
    if (mutex_rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, mutex_rv, s, "Failed to acquire global mutex");
        return mutex_rv;
    }

    /* Open the outputfile*/
    rv = apr_file_open(&cfg->output_fd, cfg->outputfile, APR_FOPEN_READ|APR_FOPEN_XTHREAD|APR_FOPEN_BUFFERED, APR_FPROT_OS_DEFAULT, cfg->pool);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to open file %s", cfg->outputfile) ;
        apr_global_mutex_unlock(cfg->shm_mutex);
        return rv;
    }
   
    /* Move the pointer to the first unread line */
    rv = apr_file_seek(cfg->output_fd, APR_SET, (apr_off_t *) &cfg->shm_memory->outputfile_offset);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to move file pointer to the end of %s", cfg->outputfile) ;
        apr_file_close(cfg->output_fd);
        apr_global_mutex_unlock(cfg->shm_mutex);
        return rv;
    }

    /* Read the line and advance the counter */
    rv = apr_file_gets(buffer, 256, cfg->output_fd);
    if (rv != APR_SUCCESS) {
        apr_file_close(cfg->output_fd);
        apr_global_mutex_unlock(cfg->shm_mutex);
        return rv;
    }
    cfg->shm_memory->outputfile_offset += strlen(buffer);

    /* Strip the trailing newline, if it's there */
    int offset = strlen(buffer)-1;
    if (buffer[offset] == '\n'){
        buffer[offset] = '\0';
    }

    /* Close the outputfile*/
    rv = apr_file_close(cfg->output_fd);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, "Failed to close file %s", cfg->outputfile) ;
        mutex_rv = apr_global_mutex_unlock(cfg->shm_mutex);
        return rv;
    }    
    // End critical section
    mutex_rv = apr_global_mutex_unlock(cfg->shm_mutex);
    if (mutex_rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, mutex_rv, s, "Failed to release global mutex");
    }

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

void encrypt(char* buffer, char* key, char* iv){
   
}

void decrypt(char* buffer, char* key, char* iv){
    AES_KEY aeskey;
    unsigned char dec_out[256];
    memset(dec_out, 0, 256);

    AES_set_decrypt_key((unsigned char*) key, 256, &aeskey);


    AES_cbc_encrypt((unsigned char*) buffer, dec_out, 16, &aeskey, (unsigned char*)iv, AES_DECRYPT);
    //AES_decrypt((const unsigned char*) buffer, dec_out, &dec_key);
    memset(buffer, 0, 256);
    memcpy(buffer, dec_out, 256);
}


