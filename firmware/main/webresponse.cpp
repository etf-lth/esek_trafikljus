#include "webresponse.h"


WebResponse::WebResponse(httpd_req_t *req) : m_request(req)
{
}


httpd_req_t* WebResponse::getRequestHandle()
{
	return m_request;
}


WebResponse& WebResponse::setStatus(const std::string &status)
{
	// TODO: implement this
	return *this;
}


WebResponse& WebResponse::addHeader(const std::string &field, const std::string &value)
{
	// TODO: implement this
	return *this;
}


std::string& WebResponse::getResponse()
{
	return m_response;
}


/************* HTML RESPONSE *************/

HTMLResponse::HTMLResponse(httpd_req_t *req) : WebResponse(req)
{
	httpd_resp_set_type(m_request, "text/html");
}


HTMLResponse& HTMLResponse::addData(const std::string &data)
{
	m_response += data;
	return *this;
}
