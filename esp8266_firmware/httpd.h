
// HTTPD backend!

#define MAX_RESPONSE_LENGTH 1024  // Usally headers are around 200 bytes (without cookies)

typedef unsigned (*t_httpd_callback)(char * buffer, const char * body);

typedef struct {
	const char * path;          // Path for GET/POST
	t_httpd_callback callback;  // Callback function for hit
} t_url_desc;

void ICACHE_FLASH_ATTR httpd_start(int port, const t_url_desc * descriptors);

