#pragma once

#include "routes.h"
#include "webresponse.h"

#include <string>
#include <map>

#include <esp_http_server.h>
#include <esp_event.h>


extern const char *TAG;

namespace WebConstants
{
	const std::string g_queryStart = "?";
	const std::string g_querySeparator = "&";
	const std::string g_paramSeparator = "=";
	const std::string g_colorStop = "stop";
	const std::string g_colorGo = "go";
	constexpr int g_hexStringLength = 9; // including the "%23" ascii sign (#)
}


class WebServer
{
public:
	WebServer();
	~WebServer();

public:
	httpd_handle_t m_server = nullptr;

public:
	bool start();
	void stop();

	std::map<std::string, std::string> getQueryParams(const httpd_req_t *req);
	std::map<std::string, std::string> getRequestHeaders(const httpd_req_t *req);

	esp_err_t send(WebResponse &resp) const;

	/**
	 * @brief Add URI handler in a fluid builder fashion.
	 *
	 * @param handler The struct containing URI method, handler function etc.
	 * @return WebServer& A reference to "this" to allow fluid builder pattern.
	 */
	WebServer& addHandler(const httpd_uri_t *handler);
	//WebServer &removeHandler(const);

	/**
	 * The callbacks for when we get an IP, and upon disconnect.
	 * These functions are passed to the event handler, and must have the
	 * "esp_event_handler_t" signatures.
	 */
	static void connectHandler(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *ev_data);
	static void disconnectHandler(void *arg, esp_event_base_t ev_base, int32_t ev_id, void *ev_data);
};

// A global WebServer variable, defined in the cpp file
extern WebServer g_webServer;
