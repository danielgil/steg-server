#define PROTOCOL_MAX_PAYLOAD_SIZE 256
#define PROTOCOL_LENGTH_SIZE 3

// Packet builder dispatcher
void packet_builder(request_rec *r, steg_config *config, server_config *svr);

// Packet builders for every method
void header_builder(request_rec *r, steg_config *config, server_config *svr);
