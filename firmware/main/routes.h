#pragma once

#include <string>

#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>

#include "esp_tls_crypto.h"

extern const char *TAG;

#define HTTPD_401      "401 UNAUTHORIZED"           /*!< HTTP Response 401 */

esp_err_t root_get_handler(httpd_req_t *req);
esp_err_t echo_post_handler(httpd_req_t *req);
esp_err_t ctrl_put_handler(httpd_req_t *req);
esp_err_t admin_handler(httpd_req_t *req);

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);

char* http_auth_basic(const char *username, const char *password);
esp_err_t basic_auth_get_handler(httpd_req_t *req);

static const httpd_uri_t g_root = {
	.uri = "/",
	.method = HTTP_GET,
	.handler = root_get_handler,
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	.user_ctx = nullptr
};

static const httpd_uri_t g_echo = {
	.uri = "/echo",
	.method = HTTP_POST,
	.handler = echo_post_handler,
	.user_ctx = NULL
};

static const httpd_uri_t g_ctrl = {
	.uri = "/ctrl",
	.method = HTTP_PUT,
	.handler = ctrl_put_handler,
	.user_ctx = NULL
};

typedef struct
{
	char *username;
	char *password;
} basic_auth_info_t;

static const basic_auth_info_t g_basic_auth_info = {
	.username = CONFIG_ADMIN_USERNAME,
	.password = CONFIG_ADMIN_PASSWORD
};

static const httpd_uri_t g_admin = {
	.uri = "/admin",
	.method = HTTP_GET,
	.handler = admin_handler,
	.user_ctx = (void*)&g_basic_auth_info
};
