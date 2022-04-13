#include "routes.h"
#include "webserver.h"
#include "webresponse.h"

#include <iostream>

#include <sys/param.h>

// include the html file as a C++ string
#include "html/index_html.h"
#include "html/admin_html.h"

using namespace std;

/* GET handler for / */
esp_err_t root_get_handler(httpd_req_t *req)
{
	char *buf;
	size_t buf_len;

	/* Get header value string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1)
	{
		buf = (char*)malloc(buf_len);
		/* Copy null terminated value string into buffer */
		if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found header => Host: %s", buf);
		}
		free(buf);
	}

	const auto params = g_webServer.getQueryParams(req);
	if (params.find("jarvis") != params.end())
	{
		if (params.at("jarvis") == "activate")
		{
			g_webServer.addHandler(&g_admin);
		}
	}

	cout << "Params:" << endl;
	for (const auto &p : params)
	{
		cout << p.first << ": " << p.second << endl;
	}

	HTMLResponse response(req);
	response.addHeader("Custom Header 1", "Eat shit and die")
	        .addData(index_html);
	g_webServer.send(response);

	return ESP_OK;
}


/* An HTTP POST handler */
esp_err_t echo_post_handler(httpd_req_t *req)
{
	char buf[100];
	int ret, remaining = req->content_len;

	while (remaining > 0)
	{
		/* Read the data for the request */
		if ((ret = httpd_req_recv(req, buf,
		                          MIN(remaining, sizeof(buf)))) <= 0)
		{
			if (ret == HTTPD_SOCK_ERR_TIMEOUT)
			{
				/* Retry receiving if timeout occurred */
				continue;
			}
			return ESP_FAIL;
		}

		/* Send back the same data */
		httpd_resp_send_chunk(req, buf, ret);
		remaining -= ret;

		/* Log data received */
		ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
		ESP_LOGI(TAG, "%.*s", ret, buf);
		ESP_LOGI(TAG, "====================================");
	}

	// End response
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}


/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
esp_err_t ctrl_put_handler(httpd_req_t *req)
{
	char buf;
	int ret;

	if ((ret = httpd_req_recv(req, &buf, 1)) <= 0)
	{
		if (ret == HTTPD_SOCK_ERR_TIMEOUT)
		{
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

	if (buf == '0')
	{
		/* URI handlers can be unregistered using the uri string */
		ESP_LOGI(TAG, "Unregistering / and /echo URIs");
		httpd_unregister_uri(req->handle, "/");
		httpd_unregister_uri(req->handle, "/echo");
		/* Register the custom error handler */
		httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
	}
	else
	{
		ESP_LOGI(TAG, "Registering / and /echo URIs");
		httpd_register_uri_handler(req->handle, &g_root);
		httpd_register_uri_handler(req->handle, &g_echo);
		/* Unregister custom error handler */
		httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
	}

	/* Respond with empty body */
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}


/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
	if (strcmp("/", req->uri) == 0)
	{
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/ URI is not available");
		/* Return ESP_OK to keep underlying socket open */
		return ESP_OK;
	}
	else if (strcmp("/echo", req->uri) == 0)
	{
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
		/* Return ESP_FAIL to close underlying socket */
		return ESP_FAIL;
	}
	/* For any other URI send 404 and close socket */
	httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
	return ESP_FAIL;
}


/***************************************************************************************************
 * BASIC AUTH ROUTES
 */

char* http_auth_basic(const char *username, const char *password)
{
	int out;
	char *user_info = NULL;
	char *digest = NULL;
	size_t n = 0;
	asprintf(&user_info, "%s:%s", username, password);
	if (!user_info)
	{
		ESP_LOGE(TAG, "No enough memory for user information");
		return NULL;
	}
	esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char*)user_info, strlen(user_info));

	/* 6: The length of the "Basic " string
	 * n: Number of bytes for a base64 encode format
	 * 1: Number of bytes for a reserved which be used to fill zero
	*/
	digest = (char*)calloc(1, 6 + n + 1);
	if (digest)
	{
		strcpy(digest, "Basic ");
		esp_crypto_base64_encode((unsigned char*)digest + 6, n, (size_t*)&out, (const unsigned char*)user_info,
		                         strlen(user_info));
	}
	free(user_info);
	return digest;
}


/* An HTTP GET handler */
esp_err_t admin_handler(httpd_req_t *req)
{
	char *buf = NULL;
	size_t buf_len = 0;
	basic_auth_info_t *basic_auth_info = (basic_auth_info_t*)req->user_ctx;

	buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
	if (buf_len > 1)
	{
		buf = (char*)calloc(1, buf_len);
		if (!buf)
		{
			ESP_LOGE(TAG, "No enough memory for basic authorization");
			return ESP_ERR_NO_MEM;
		}

		if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
		}
		else
		{
			ESP_LOGE(TAG, "No auth value received");
		}

		char *auth_credentials = http_auth_basic(basic_auth_info->username, basic_auth_info->password);
		if (!auth_credentials)
		{
			ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
			free(buf);
			return ESP_ERR_NO_MEM;
		}

		if (strncmp(auth_credentials, buf, buf_len))
		{
			ESP_LOGE(TAG, "Not authenticated");
			httpd_resp_set_status(req, HTTPD_401);
			//httpd_resp_set_type(req, "application/json");
			httpd_resp_set_hdr(req, "Connection", "keep-alive");
			httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Hello\"");
			httpd_resp_send(req, NULL, 0);
		}
		else
		{
			ESP_LOGI(TAG, "Authenticated!");
			HTMLResponse response(req);
			response.setStatus(HTTPD_200)
			        .addHeader("Connection", "keep-alive")
			        .addData(admin_html);

			char *basic_auth_resp = NULL;
			httpd_resp_set_status(req, HTTPD_200);
			//httpd_resp_set_type(req, "application/json");
			httpd_resp_set_hdr(req, "Connection", "keep-alive");
			asprintf(&basic_auth_resp, "{\"authenticated\": true,\"user\": \"%s\"}", basic_auth_info->username);
			if (!basic_auth_resp)
			{
				ESP_LOGE(TAG, "No enough memory for basic authorization response");
				free(auth_credentials);
				free(buf);
				return ESP_ERR_NO_MEM;
			}
			g_webServer.send(response);
			free(basic_auth_resp);
		}
		free(auth_credentials);
		free(buf);
	}
	else
	{
		ESP_LOGE(TAG, "No auth header received");
		httpd_resp_set_status(req, HTTPD_401);
		//httpd_resp_set_type(req, "application/json");
		httpd_resp_set_hdr(req, "Connection", "keep-alive");
		httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Hello\"");
		httpd_resp_send(req, NULL, 0);
	}

	return ESP_OK;
}
