

// Packet builder dispatcher
void packet_decoder(request_rec *r, steg_config *config, server_config *svr);
void packet_encoder(request_rec *r, steg_config *config, server_config *svr);

// Packet builders for every method
void header_decoder(request_rec *r, steg_config *config, server_config *svr);
void header_encoder(request_rec *r, steg_config *config, server_config *svr, char* data);

void present_decoder(request_rec *r, steg_config *config, server_config *svr);
