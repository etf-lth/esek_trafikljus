#include "routes.h"
#include "webserver.h"
#include "webresponse.h"
#include "trafficlight.h"

#include <iostream>

#include <sys/param.h>

// include the html file as a C++ string
#include "html/index_html.h"
#include "html/admin_html.h"
#include "html/about_html.h"

using namespace std;

/* GET handler for / */
esp_err_t root_get_handler(httpd_req_t *req)
{
	const auto params = g_webServer.getQueryParams(req);
	if (params.find(CONFIG_ENABLE_ADMIN_UNAME) != params.end())
	{
		if (params.at(CONFIG_ENABLE_ADMIN_UNAME) == CONFIG_ENABLE_ADMIN_PASSWD) g_webServer.addHandler(&g_admin);
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
	httpd_resp_send_chunk(req, nullptr, 0);
	return ESP_OK;
}


esp_err_t about_get_handler(httpd_req_t *req)
{
	HTMLResponse response(req);
	response.addData(about_html);
	g_webServer.send(response);
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
	size_t out;
	char *user_info = nullptr;
	char *digest = nullptr;
	size_t n = 0;
	asprintf(&user_info, "%s:%s", username, password);
	if (!user_info)
	{
		ESP_LOGE(TAG, "No enough memory for user information");
		return nullptr;
	}
	esp_crypto_base64_encode(nullptr, 0, &n, (const unsigned char*)user_info, strlen(user_info));

	/* 6: The length of the "Basic " string
	 * n: Number of bytes for a base64 encode format
	 * 1: Number of bytes for a reserved which be used to fill zero
	*/
	digest = (char*)calloc(1, 6 + n + 1);
	if (digest)
	{
		strcpy(digest, "Basic ");
		esp_crypto_base64_encode((unsigned char*)digest + 6, n, &out, (const unsigned char*)user_info,
		                         strlen(user_info));
	}
	free(user_info);
	return digest;
}


/* An HTTP GET handler */
esp_err_t admin_handler(httpd_req_t *req)
{
	const auto *basic_auth_info = static_cast<basic_auth_info_t*>(req->user_ctx);

	const auto buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
	if (buf_len == 0)
	{
		ESP_LOGE(TAG, "No auth header received");

		HTMLResponse response(req);
		response.setStatus(HTTPD_401)
		        .addHeader("Connection", "keep-alive")
		        .addHeader("WWW-Authenticate", "Basic realm=\"Hello\"");

		g_webServer.send(response);
		return ESP_OK;
	}

	const auto buf = static_cast<char*>(calloc(1, buf_len));
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
		httpd_resp_send(req, nullptr, 0);
	}
	else
	{
		ESP_LOGI(TAG, "Authenticated!");

		const auto params = g_webServer.getQueryParams(req);
		if (params.find(WebConstants::g_colorTop) != params.end())
		{
			cout << "Top color: " << params.at(WebConstants::g_colorTop) << endl;

			const auto &val = params.at(WebConstants::g_colorTop);
			if (val.length() == WebConstants::g_hexStringLength)
			{
				// skip the "%23" sign (#)
				const auto &hexVal = val.substr(3);
				const UniColor stopColor(hexVal);
				g_trafficLight.setStopColor(stopColor);
			}
		}

		if (params.find(WebConstants::g_colorBottom) != params.end())
		{
			const auto &val = params.at(WebConstants::g_colorBottom);
			if (val.length() == WebConstants::g_hexStringLength)
			{
				// skip the "%23" sign (#)
				const auto &hexVal = val.substr(3);
				const UniColor bottomColor(hexVal);
				g_trafficLight.setGoColor(bottomColor);
			}
		}

		HTMLResponse response(req);
		response.setStatus(HTTPD_200)
		        .addHeader("Connection", "keep-alive")
		        .addData(admin_html);

		g_webServer.send(response);
	}
	free(auth_credentials);
	free(buf);


	return ESP_OK;
}
