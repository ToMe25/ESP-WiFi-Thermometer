/*
 * webhandler.cpp
 *
 *  Created on: May 30, 2023
 *      Author: ToMe25
 */

#include "webhandler.h"
#include "main.h"
#include "prometheus.h"
#include <iomanip>
#include <sstream>
#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif

#if ENABLE_WEB_SERVER == 1
AsyncWebServer web::server(WEB_SERVER_PORT);
#endif

void web::setup() {
#if ENABLE_WEB_SERVER == 1
	std::map<String, std::function<std::string()>> index_replacements = { {
			"TEMP", getTemperature }, { "HUMID", getHumidity }, { "TIME",
			getTimeSinceMeasurement } };

	registerReplacingStaticHandler("/", "text/html", INDEX_HTML, index_replacements);
	registerReplacingStaticHandler("/index.html", "text/html", INDEX_HTML, index_replacements);

	registerCompressedStaticHandler("/main.css", "text/css", MAIN_CSS_START, MAIN_CSS_END);
	registerCompressedStaticHandler("/index.js", "text/javascript", INDEX_JS_START, INDEX_JS_END);
	registerCompressedStaticHandler("/manifest.json", "application/json", MANIFEST_JSON_START, MANIFEST_JSON_END);

	registerRequestHandler("/temperature", HTTP_GET,
			[](AsyncWebServerRequest *request) -> uint16_t {
				request->send(200, "text/plain", getTemperature().c_str());
				return 200;
			});

	registerRequestHandler("/humidity", HTTP_GET,
			[](AsyncWebServerRequest *request) -> uint16_t {
				request->send(200, "text/plain", getHumidity().c_str());
				return 200;
			});

	registerRequestHandler("/data.json", HTTP_GET, getJson);

	registerCompressedStaticHandler("/favicon.ico", "image/x-icon", FAVICON_ICO_GZ_START,
			FAVICON_ICO_GZ_END);
	registerCompressedStaticHandler("/favicon.png", "image/png", FAVICON_PNG_GZ_START,
			FAVICON_PNG_GZ_END);
	registerCompressedStaticHandler("/favicon.svg", "image/svg+xml", FAVICON_SVG_GZ_START,
			FAVICON_SVG_GZ_END);

	server.onNotFound(notFoundHandler);

	DefaultHeaders::Instance().addHeader("Server", SERVER_HEADER);
	server.begin();

	uzlib_init();

#if ENABLE_ARDUINO_OTA != 1
	MDNS.begin(HOSTNAME);
#endif

	MDNS.addService("http", "tcp", WEB_SERVER_PORT);
#endif
}

void web::loop() {

}

void web::connect() {

}

#if ENABLE_WEB_SERVER == 1
uint16_t web::getJson(AsyncWebServerRequest *request) {
	std::ostringstream json;
	json << "{\"temperature\": ";
	if (isnan(temperature)) {
		json << "\"Unknown\"";
	} else {
		json << std::setprecision(temperature > 10 ? 4 : 3) << temperature;
	}
	json << ", \"humidity\": ";
	if (isnan(humidity)) {
		json << "\"Unknown\"";
	} else {
		json << std::setprecision(humidity > 10 ? 4 : 3) << humidity;
	}
	json << ", \"time\": \"";
	json << getTimeSinceMeasurement();
	json << "\"}";
	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json.str().c_str());
	response->addHeader("Cache-Control", "no-cache");
	request->send(response);
	return 200;
}

size_t web::decompressingResponseFiller(const std::shared_ptr<uzlib_gzip_wrapper> decomp,
		uint8_t *buffer, const size_t max_len, const size_t index) {
	return decomp->decompress(buffer, max_len);
}

size_t web::replacingResponseFiller(
		const std::map<String, String> &replacements,
		std::shared_ptr<int64_t> offset, const uint8_t *start,
		const uint8_t *end, uint8_t *buffer, const size_t max_len,
		const size_t index) {
	const uint8_t *idx = start + index + *offset;
	const uint8_t *template_start = (uint8_t*) memchr(idx, TEMPLATE_CHAR,
			end - idx);
	// No replacing necessary.
	if (!template_start
			|| (size_t) (template_start - start) > max_len + index + *offset) {
		// Fill free space with null bytes, if necessary.
		if ((size_t) (end - idx) < max_len) {
			memcpy(buffer, idx, end - idx);
			memset(buffer + (end - idx), 0, max_len - (end - idx));
			return max_len;
		} else {
			memcpy(buffer, idx, max_len);
			return max_len;
		}
	} else {
		size_t written = 0;
		while (template_start && template_start - idx + written < max_len) {
			memcpy(buffer + written, idx, template_start - idx);
			written += template_start - idx;
			const uint8_t *template_end = (uint8_t*) memchr(template_start + 1,
					TEMPLATE_CHAR, end - template_start - 1);
			if (!template_end) {
				break;
			}

			uint8_t *buf = new uint8_t[template_end - template_start];
			memcpy(buf, template_start + 1, template_end - template_start - 1);
			buf[template_end - template_start - 1] = 0;
			String replacement = String((char*) buf);
			std::map<String, String>::const_iterator it = replacements.find(replacement);
			if (it != replacements.end()) {
				replacement = it->second;
			}
			if (replacement.length() > max_len - written) {
				return written > 0 ? written : RESPONSE_TRY_AGAIN;
			}

			memcpy(buffer + written, replacement.c_str(), replacement.length());
			written += replacement.length();
			*offset += template_end - template_start + 1 - replacement.length();
			delete[] buf;
			idx = template_end + 1;
			template_start = (uint8_t*) memchr(idx, TEMPLATE_CHAR, end - idx);
		}

		// Fill free space with null bytes, if necessary.
		if ((size_t) (end - idx) < max_len - written) {
			memcpy(buffer + written, idx, end - idx);
			memset(buffer + written + (end - idx), 0,
					max_len - written - (end - idx));
			return max_len;
		} else {
			memcpy(buffer + written, idx, max_len - written);
			return max_len;
		}
	}
}

void web::trackingRequestHandlerWrapper(const char *uri, const HTTPRequestHandler handler, AsyncWebServerRequest *request) {
	const uint16_t status_code = handler(request);
#if ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
	prom::http_requests_total[std::pair<String, uint16_t>(uri, status_code)]++;
#endif
}

void web::notFoundHandler(AsyncWebServerRequest *request) {
	const uint16_t status_code = compressedStaticHandler(404, "text/html", NOT_FOUND_HTML_START,
			NOT_FOUND_HTML_END, request);
	Serial.print("A client tried to access the not existing file \"");
	Serial.print(request->url().c_str());
	Serial.println("\".");
#if ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
	prom::http_requests_total[std::pair<String, uint16_t>(
			request->url(), status_code)]++;
#endif
}

uint16_t web::compressedStaticHandler(const uint16_t status_code, const char *content_type, const uint8_t *start,
		const uint8_t *end, AsyncWebServerRequest *request) {
	AsyncWebServerResponse *response = NULL;
	if (request->hasHeader("Accept-Encoding")
			&& strstr(request->getHeader("Accept-Encoding")->value().c_str(),
					"gzip")) {
		response = request->beginResponse_P(200, content_type, start,
				end - start);
		response->addHeader("Content-Encoding", "gzip");
	} else {
		using namespace std::placeholders;
		std::shared_ptr<uzlib_gzip_wrapper> decomp = std::make_shared<
				uzlib_gzip_wrapper>(start, end, GZIP_DECOMP_WINDOW_SIZE);
		response = request->beginResponse(content_type,
				decomp->getDecompressedSize(),
				std::bind(decompressingResponseFiller, decomp, _1, _2, _3));
	}
	response->setCode(status_code);
	request->send(response);
	return status_code;
}

uint16_t web::replacingRequestHandler(
		const std::map<String, std::function<std::string()>> replacements,
		const uint16_t status_code, const char *content_type,
		const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request) {
	using namespace std::placeholders;
	std::map<String, String> repl;
	for (std::pair<String, std::function<std::string()>> replacement : replacements) {
		repl[replacement.first] = replacement.second().c_str();
	}

	int64_t len_diff = 0;
	const uint8_t *idx = start;
	while (idx && (idx = (uint8_t*) memchr(idx, TEMPLATE_CHAR, end - idx))) {
		const uint8_t *template_end = (uint8_t*) memchr(idx + 1, TEMPLATE_CHAR,
				end - idx - 1);
		if (!template_end) {
			break;
		}
		uint8_t *buf = new uint8_t[template_end - idx];
		memcpy(buf, idx + 1, template_end - idx - 1);
		buf[template_end - idx - 1] = 0;
		len_diff += repl[String((char*)buf)].length() - (template_end - idx + 1);
		delete[] buf;
		idx = template_end + 1;
	}

	std::shared_ptr<int64_t> offset = std::make_shared<int64_t>(0);
	AsyncWebServerResponse *response = request->beginResponse(content_type,
			(end - start) + len_diff,
			std::bind(replacingResponseFiller, repl, offset, start, end, _1, _2,
					_3));
	response->setCode(status_code);
	request->send(response);
	return status_code;
}

void web::registerRequestHandler(const char *uri, WebRequestMethodComposite method,
		HTTPRequestHandler handler) {
	using namespace std::placeholders;
	server.on(uri, method, std::bind(trackingRequestHandlerWrapper, uri, handler, _1));
}

void web::registerStaticHandler(const char *uri, const char *content_type,
		const char *page) {
	registerRequestHandler(uri, HTTP_GET,
			[content_type, page](AsyncWebServerRequest *request) -> uint16_t {
				request->send_P(200, content_type, page);
				return 200;
			});
}

void web::registerCompressedStaticHandler(const char *uri, const char *content_type,
		const uint8_t *start, const uint8_t *end) {
	using namespace std::placeholders;
	registerRequestHandler(uri, HTTP_GET,
			std::bind(compressedStaticHandler, 200, content_type, start, end, _1));
}

void web::registerReplacingStaticHandler(const char *uri,
		const char *content_type, const char *page,
		const std::map<String, std::function<std::string()>> replacements) {
	using namespace std::placeholders;
	registerRequestHandler(uri, HTTP_GET,
			std::bind(replacingRequestHandler, replacements, 200, content_type,
					(uint8_t*) page, (uint8_t*) page + strlen(page), _1));
}
#endif /* ENABLE_WEB_SERVER == 1 */
