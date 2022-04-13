#pragma once

#include <string>

#include <esp_http_server.h>

/**
 * Generic response class, not meant to be instantiated.
 * Instead, use the specific subclasses.
 */
class WebResponse
{
protected:
	httpd_req_t *m_request;
	std::string m_response;

	explicit WebResponse(httpd_req_t *req);

public:
	httpd_req_t *getRequestHandle();
	WebResponse& setStatus(const std::string &status);
	WebResponse& addHeader(const std::string &field, const std::string &value);
	std::string& getResponse();

	virtual WebResponse &addData(const std::string &data) = 0;
};


class HTMLResponse : public WebResponse
{
public:
	explicit HTMLResponse(httpd_req_t *req);

	HTMLResponse& addData(const std::string &data) override;
};

/*
class JSONResponse : public WebResponse
{
private:
	httpd_req_t *m_request;
	std::string m_response;

public:
	explicit JSONResponse(httpd_req_t *req);

	httpd_req_t *getRequestHandle() override;

	JSONResponse & setStatus(const std::string &status) override;
	JSONResponse & addHeader(const std::string &field, const std::string &value) override;
	JSONResponse & addData(const std::string &resp) override;

	std::string& getResponse() override;
};
*/
