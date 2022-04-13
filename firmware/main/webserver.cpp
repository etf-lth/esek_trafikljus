#include "webserver.h"

#include <iostream>

#include <esp_http_server.h>
#include <esp_log.h>

using namespace std;

/* A global WebServer variable, since we need to access this from multiple places,
 * and cannot always pass it as reference, e.g., in events
 */
WebServer g_webServer;


WebServer::WebServer()
{
	std::cout << "Web server on it!" << std::endl;
}


WebServer::~WebServer()
{
	std::cout << "Web server out!" << std::endl;
}


bool WebServer::start()
{
	if (m_server) return true;

	httpd_handle_t server = nullptr;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.lru_purge_enable = true;

	// Start the httpd server
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) != ESP_OK) return false;

	m_server = server;
	return true;
}


void WebServer::stop()
{
	if (!m_server) return;

	httpd_stop(m_server);
	m_server = nullptr;
}


map<string, string> WebServer::getQueryParams(const httpd_req_t *req)
{
	map<string, string> params;

	auto query = string(req->uri);
	auto startIdx = query.find(WebConstants::g_queryStart);
	// No query found
	if (startIdx == string::npos) return params;

	auto endIdx = query.find(WebConstants::g_querySeparator);
	while (startIdx != string::npos)
	{
		const string sub = query.substr(startIdx + 1, endIdx - startIdx - 1);

		// get the parameter key and value
		const auto keyIdx = sub.find(WebConstants::g_paramSeparator);
		const string key = sub.substr(0, keyIdx);
		const string val = sub.substr(keyIdx + 1);
		if (!key.empty() && !val.empty())
		{
			params[key] = val;
		}

		startIdx = endIdx;
		endIdx = query.find(WebConstants::g_querySeparator, startIdx + 1);
	}

	return params;
}


std::map<std::string, std::string> WebServer::getRequestHeaders(const httpd_req_t *req)
{
	map<string, string> headers;

	return headers;
}


WebServer& WebServer::addResponseHeader(httpd_req_t *req, std::string field, std::string value)
{
	httpd_resp_set_hdr(req, field.c_str(), value.c_str());
	return *this;
}


esp_err_t WebServer::send(httpd_req_t *req, std::string data) const
{
	return httpd_resp_send(req, data.c_str(), HTTPD_RESP_USE_STRLEN);
}


WebServer& WebServer::addHandler(const httpd_uri_t *handler)
{
	httpd_register_uri_handler(m_server, handler);
	return *this;
}


void WebServer::connectHandler(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *ev_data)
{
	g_webServer.start();
}


void WebServer::disconnectHandler(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *ev_data)
{
	g_webServer.stop();
}
