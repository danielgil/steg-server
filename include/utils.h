

// Write buffer to file. Comodity wrapper around various APR calls
int write_inputfile(const char *buffer, request_rec *r, const char *filename);
int readline_outputfile(char *buffer, server_rec *s);

// Turn value into a string, padding it with 0 to the left, e.g. 5 -> "0005"
char* int_to_string(char* dest, int value, int maxsize);

// Cryptography auxiliary functions. They encrypt using AES-CBC-128
void encrypt(char* buffer, char* key, char* iv);
void decrypt(char* buffer, char* key, char* iv);
