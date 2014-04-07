

// Write buffer to file. Comodity wrapper around various APR calls
int write_inputfile(const char *buffer, request_rec *r, const char *filename);

// Safe string copying
int safe_strcpy(char *dest, const char *source, int maxsize);
