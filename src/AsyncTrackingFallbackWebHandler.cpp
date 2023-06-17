/*
 * AsyncMultitypeFallbackWebHandler.cpp
 *
 *  Created on: Jun 17, 2023
 *      Author: ToMe25
 */

#include <AsyncTrackingFallbackWebHandler.h>
#if ENABLE_WEB_SERVER == 1
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1 || ENABLE_PROMETHEUS_PUSH == 1
#include "prometheus.h"
#endif
#include <cmath>

web::AsyncTrackingFallbackWebHandler::AsyncTrackingFallbackWebHandler(
		const String &uri, HTTPFallbackRequestHandler fallback) :
		_uri(uri), _fallbackHandler(fallback), _handlers(
				(size_t) std::log2((size_t) HTTP_ANY) + 1) {

}

web::AsyncTrackingFallbackWebHandler::~AsyncTrackingFallbackWebHandler() {

}

web::HTTPRequestHandler web::AsyncTrackingFallbackWebHandler::_getHandler(const WebRequestMethod method) const {
	WebRequestMethodComposite mtd = method;
	unsigned idx = 0;
	while (mtd >>= 1) {
	    idx++;
	}
	return _handlers[idx];
}

void web::AsyncTrackingFallbackWebHandler::setHandler(const WebRequestMethodComposite methods, HTTPRequestHandler handler) {
	for (size_t i = 0; i < _handlers.size(); i++) {
		if (methods & (1 << i)) {
			_handlers[i] = handler;
		}
	}
}

void web::AsyncTrackingFallbackWebHandler::setFallbackHandler(HTTPFallbackRequestHandler fallback) {
	_fallbackHandler = fallback;
}

WebRequestMethodComposite web::AsyncTrackingFallbackWebHandler::getHandledMethods() const {
	WebRequestMethodComposite methods = 0;
	for (size_t i = 0; i < _handlers.size(); i++) {
		if (_handlers[i]) {
			methods |= 1 << i;
		}
	}
	return methods;
}

bool web::AsyncTrackingFallbackWebHandler::canHandle(AsyncWebServerRequest *request) {
	if (!_uri.length()) {
		return false;
	}

	if (_uri == request->url() || request->url().startsWith(_uri + '/')) {
		request->addInterestingHeader("ANY");
		return true;
	}
	return false;
}

void web::AsyncTrackingFallbackWebHandler::handleRequest(AsyncWebServerRequest *request) {
	ResponseData response(request->beginResponse(500), 0, 500);
	HTTPRequestHandler handler = _getHandler((WebRequestMethod) request->method());
	if (handler) {
		delete response.response;
		response = handler(request);
	} else if (_fallbackHandler) {
		delete response.response;
		response = _fallbackHandler(getHandledMethods(), request);
	} else {
		Serial.print("The handler for uri \"");
		Serial.print(_uri);
		Serial.print("\" didn't have a handler for request type ");
		Serial.print(request->methodToString());
		Serial.println(", and didn't have a fallback handler.");
	}
#if ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
	prom::http_requests_total[std::pair<String, uint16_t>(
			request->url(), response.status_code)]++;
#endif
	request->send(response.response);
}

bool web::AsyncTrackingFallbackWebHandler::isRequestHandlerTrivial() {
	return false;
}
#endif /* ENABLE_WEB_SERVER == 1 */
