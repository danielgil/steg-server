
static int steg_handler(request_rec *r);

void *create_dir_conf(apr_pool_t *pool, char *context);
void *create_server_conf(apr_pool_t *pool, server_rec *s);
void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD);

static void steg_register_hooks(apr_pool_t *p);

static void stegChildInit(apr_pool_t *pchild, server_rec *s);
static int stegPostConfig(apr_pool_t *pool, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s);

static apr_status_t stegInputFilter(ap_filter_t *f,
                                      apr_bucket_brigade *bb,
                                      ap_input_mode_t mode,
                                      apr_read_type_e block,
                                      apr_off_t readbytes);
static apr_status_t stegOutputFilter(ap_filter_t *f, apr_bucket_brigade *bb);



