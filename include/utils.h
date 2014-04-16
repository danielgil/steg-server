

// Write buffer to file. Comodity wrapper around various APR calls
int write_inputfile(const char *buffer, request_rec *r, const char *filename);
int readline_outputfile(char *buffer, server_rec *s);


// Turn value into a string, padding it with 0 to the left, e.g. 5 -> "0005"
char* int_to_string(char* dest, int value, int maxsize);

