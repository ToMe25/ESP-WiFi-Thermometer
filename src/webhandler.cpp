/*
 * webhandler.cpp
 *
 *  Created on: May 30, 2023
 *      Author: ToMe25
 */

#include "webhandler.h"
#include "main.h"
#if ENABLE_WEB_SERVER == 1
#if ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1 || ENABLE_PROMETHEUS_PUSH == 1
#include "prometheus.h"
#endif
#include "AsyncHeadOnlyResponse.h"
#include <sstream>
#endif
#include <iomanip>
#ifdef ESP32
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266mDNS.h>
#endif
#include "fallback_log.h"

#if ENABLE_WEB_SERVER == 1
AsyncWebServer web::server(WEB_SERVER_PORT);
std::map<String, web::AsyncTrackingFallbackWebHandler*> web::handlers;
#endif

void web::setup() {
#if ENABLE_WEB_SERVER == 1
	std::map<String, std::function<std::string()>> index_replacements = { {
			"TEMP", getTemperature }, { "HUMID", getHumidity }, { "TIME",
			getTimeSinceMeasurement } };

	registerRedirect("/", "/index.html");
	registerReplacingStaticHandler("/index.html", "text/html", INDEX_HTML,
			index_replacements);

	registerCompressedStaticHandler("/main.css", "text/css", MAIN_CSS_START,
			MAIN_CSS_END);
	registerCompressedStaticHandler("/index.js", "text/javascript",
			INDEX_JS_START, INDEX_JS_END);
	registerCompressedStaticHandler("/manifest.json", "application/json",
			MANIFEST_JSON_START, MANIFEST_JSON_END);

	registerRequestHandler("/temperature", HTTP_GET,
			[](AsyncWebServerRequest *request) -> ResponseData {
				const std::string temp = getTemperature();
				return ResponseData(
						request->beginResponse(200, "text/plain", temp.c_str()),
						temp.length(), 200);
			});

	registerRequestHandler("/humidity", HTTP_GET,
			[](AsyncWebServerRequest *request) -> ResponseData {
				const std::string humidity = getHumidity();
				return ResponseData(
						request->beginResponse(200, "text/plain",
								humidity.c_str()), humidity.length(), 200);
			});

	registerRequestHandler("/data.json", HTTP_GET, getJson);

	registerCompressedStaticHandler("/favicon.ico", "image/x-icon",
			FAVICON_ICO_GZ_START, FAVICON_ICO_GZ_END);
	registerCompressedStaticHandler("/favicon.png", "image/png",
			FAVICON_PNG_GZ_START, FAVICON_PNG_GZ_END);
	registerCompressedStaticHandler("/favicon.svg", "image/svg+xml",
			FAVICON_SVG_GZ_START, FAVICON_SVG_GZ_END);

	// An OPTIONS request to * is supposed to return server-wide support.
	registerRequestHandler("*", HTTP_OPTIONS,
			std::bind(optionsHandler, HTTP_GET | HTTP_HEAD | HTTP_OPTIONS,
					std::placeholders::_1));

	server.onNotFound(notFoundHandler);

	DefaultHeaders::Instance().addHeader("Server", SERVER_HEADER);
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
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
web::ResponseData::ResponseData(AsyncWebServerResponse *response,
		size_t content_len, uint16_t status_code) :
		response(response), content_length(content_len), status_code(
				status_code) {

}

web::ResponseData web::getJson(AsyncWebServerRequest *request) {
	const std::string time_string = getTimeSinceMeasurement();
	// Valid values will never be longer than "Unknown".
	const size_t max_len = 61 + time_string.length();
	char *buffer = new char[max_len + 1];
	buffer[0] = 0;

	strcpy(buffer, "{\"temperature\": ");
	size_t len = 16;
	if (isnan(temperature)) {
		strcpy(buffer + len, "\"Unknown\"");
		len += 9;
	} else {
		len += snprintf(buffer + len, max_len - len, "%.2f", temperature);
	}
	strcpy(buffer + len, ", \"humidity\": ");
	len += 14;
	if (isnan(humidity)) {
		strcpy(buffer + len, "\"Unknown\"");
		len += 9;
	} else {
		len += snprintf(buffer + len, max_len - len, "%.2f", humidity);
	}
	len += snprintf(buffer + len, max_len - len, ", \"time\": \"%s\"}",
			time_string.c_str());

	AsyncWebServerResponse *response = request->beginResponse(200,
			"application/json", buffer);
	delete[] buffer;
	response->addHeader("Cache-Control", "no-cache");
	return ResponseData(response, len, 200);
}

size_t web::decompressingResponseFiller(
		const std::shared_ptr<uzlib_gzip_wrapper> decomp, uint8_t *buffer,
		const size_t max_len, const size_t index) {
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
			std::map<String, String>::const_iterator it = replacements.find(
					replacement);
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

web::ResponseData web::defaultHeadRequestHandlerWrapper(
		const HTTPRequestHandler &handler, AsyncWebServerRequest *request) {
	ResponseData response = handler(request);
	response.response = new AsyncHeadOnlyResponse(response.response,
			response.status_code);
	return response;
}

void web::notFoundHandler(AsyncWebServerRequest *request) {
	const size_t start = micros();
	const std::map<String, String> replacements { { "TITLE", "Error 404 Not Found" },
			{ "ERROR", "The requested file can not be found on this server!" },
			{ "DETAILS", "The page \"" + request->url()
					+ "\" couldn't be found." } };
	ResponseData response = replacingRequestHandler(replacements, 404,
			"text/html", (uint8_t*) ERROR_HTML,
			(uint8_t*) ERROR_HTML + strlen(ERROR_HTML), request);
	if (request->method() == HTTP_HEAD) {
		response.response = new AsyncHeadOnlyResponse(response.response,
				response.status_code);
	}
#if ENABLE_PROMETHEUS_PUSH == 1 || ENABLE_PROMETHEUS_SCRAPE_SUPPORT == 1
	prom::http_requests_total[request->url()][ {
			(WebRequestMethod) request->method(), response.status_code }]++;
#endif
	const size_t mid = micros();
	request->send(response.response);
	const size_t end = micros();
	log_i("A client tried to access the not existing file \"%s\".",
			request->url().c_str());
	log_d("Handling a request to \"%s\" took %luus + %luus.",
			request->url().c_str(), (long unsigned int ) (mid - start),
			(long unsigned int ) (end - mid));
}

web::ResponseData web::invalidMethodHandler(
		const WebRequestMethodComposite validMethods,
		AsyncWebServerRequest *request) {
	if (request->method() == HTTP_OPTIONS) {
		return optionsHandler(validMethods, request);
	} else {
		std::vector<String> valid;
		valid.reserve(7);
		if (validMethods & HTTP_GET) {
			valid.push_back("GET");
		}
		if (validMethods & HTTP_POST) {
			valid.push_back("POST");
		}
		if (validMethods & HTTP_PUT) {
			valid.push_back("PUT");
		}
		if (validMethods & HTTP_PATCH) {
			valid.push_back("PATCH");
		}
		if (validMethods & HTTP_DELETE) {
			valid.push_back("DELETE");
		}
		if (validMethods & HTTP_HEAD) {
			valid.push_back("HEAD");
		}
		valid.push_back("OPTIONS");

		String validStr = "";
		for (size_t i = 0; i < valid.size(); i++) {
			if (i > 0) {
				validStr += ", ";
				if (i == valid.size() - 1) {
					validStr += "and ";
				}
			}
			validStr += valid[i];
		}

		std::map<String, String> replacements { { "TITLE",
				"Error 405 Method Not Allowed" }, { "ERROR",
				"The page cannot handle " + String(request->methodToString())
						+ " requests!" }, { "DETAILS", "The page \""
				+ request->url() + "\" can handle the request methods "
				+ validStr + "." } };

		ResponseData response = replacingRequestHandler(replacements, 405,
				"text/html", (uint8_t*) ERROR_HTML,
				(uint8_t*) ERROR_HTML + strlen(ERROR_HTML), request);
		if (request->method() == HTTP_HEAD) {
			response.response = new AsyncHeadOnlyResponse(response.response,
					405);
		}
		validStr = "";
		for (size_t i = 0; i < valid.size(); i++) {
			if (i > 0) {
				validStr += ", ";
			}
			validStr += valid[i];
		}
		response.response->addHeader("Allow", validStr);
		log_i("A client tried to access the not existing file \"%s\".", request->url().c_str());
		return response;
	}
}

web::ResponseData web::optionsHandler(
		const WebRequestMethodComposite validMethods,
		AsyncWebServerRequest *request) {
	const uint16_t status_code = 204;
	String valid = "OPTIONS";
	if (validMethods & HTTP_GET) {
		valid += ", GET";
	}
	if (validMethods & HTTP_POST) {
		valid += ", POST";
	}
	if (validMethods & HTTP_PUT) {
		valid += ", PUT";
	}
	if (validMethods & HTTP_PATCH) {
		valid += ", PATCH";
	}
	if (validMethods & HTTP_DELETE) {
		valid += ", DELETE";
	}
	if (validMethods & HTTP_HEAD) {
		valid += ", HEAD";
	}
	AsyncWebServerResponse *response = request->beginResponse(status_code);
	response->addHeader("Allow", valid);
	return ResponseData(response, 0, status_code);
}

web::ResponseData web::staticHandler(const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request) {
	return ResponseData(
			request->beginResponse_P(status_code, content_type, start,
					end - start), end - start, status_code);
}

web::ResponseData web::compressedStaticHandler(const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request) {
	AsyncWebServerResponse *response = NULL;
	size_t content_length;
	if (request->hasHeader("Accept-Encoding")
			&& strstr(request->getHeader("Accept-Encoding")->value().c_str(),
					"gzip")) {
		content_length = end - start;
		response = request->beginResponse_P(200, content_type, start,
				end - start);
		response->addHeader("Content-Encoding", "gzip");
	} else {
		using namespace std::placeholders;
		std::shared_ptr<uzlib_gzip_wrapper> decomp = std::make_shared<
				uzlib_gzip_wrapper>(start, end, GZIP_DECOMP_WINDOW_SIZE);
		content_length = decomp->getDecompressedSize();
		response = request->beginResponse(content_type,
				decomp->getDecompressedSize(),
				std::bind(decompressingResponseFiller, decomp, _1, _2, _3));
	}
	response->setCode(status_code);
	return ResponseData(response, content_length, status_code);
}

web::ResponseData web::replacingRequestHandler(
		const std::map<String, std::function<std::string()>> replacements,
		const uint16_t status_code, const String &content_type,
		const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request) {
	std::map<String, String> repl;
	for (std::pair<String, std::function<std::string()>> replacement : replacements) {
		repl[replacement.first] = replacement.second().c_str();
	}

	return replacingRequestHandler(repl, status_code, content_type, start, end,
			request);
}

web::ResponseData web::replacingRequestHandler(
		const std::map<String, String> replacements, const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request) {
	using namespace std::placeholders;
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
		if (replacements.find(String((char*) buf)) != replacements.end()) {
			len_diff += replacements.at(String((char*) buf)).length()
					- (template_end - idx + 1);
		} else {
			len_diff -= 2;
		}
		delete[] buf;
		idx = template_end + 1;
	}

	const size_t content_length = (end - start) + len_diff;
	std::shared_ptr<int64_t> offset = std::make_shared<int64_t>(0);
	AsyncWebServerResponse *response = request->beginResponse(content_type,
			content_length,
			std::bind(replacingResponseFiller, replacements, offset, start, end,
					_1, _2, _3));
	response->setCode(status_code);
	return ResponseData(response, content_length, status_code);
}

web::ResponseData web::redirectHandler(const char *target,
		AsyncWebServerRequest *request) {
	const uint16_t status_code = 307;
	AsyncWebServerResponse *response = request->beginResponse(status_code);
	response->addHeader("Location", target);
	return ResponseData(response, 0, status_code);
}

void web::registerRequestHandler(const char *uri,
		const WebRequestMethodComposite method, HTTPRequestHandler handler) {
	const String uri_s = uri;
	AsyncTrackingFallbackWebHandler *hand = handlers[uri_s];
	if (!hand) {
		hand = handlers[uri_s] = new AsyncTrackingFallbackWebHandler(uri_s,
				invalidMethodHandler);
		server.addHandler(hand);
	}
	using namespace std::placeholders;
	hand->setHandler(method, handler);
	if ((method & HTTP_GET) && !(hand->getHandledMethods() & HTTP_HEAD)) {
		hand->setHandler(HTTP_HEAD,
				std::bind(defaultHeadRequestHandlerWrapper, handler, _1));
	}
}

void web::registerStaticHandler(const char *uri, const String &content_type,
		const char *page) {
	using namespace std::placeholders;
	registerRequestHandler(uri, HTTP_GET,
			std::bind(staticHandler, 200, content_type, (uint8_t*) page,
					(uint8_t*) page + strlen(page), _1));
}

void web::registerCompressedStaticHandler(const char *uri,
		const String &content_type, const uint8_t *start, const uint8_t *end) {
	using namespace std::placeholders;
	registerRequestHandler(uri, HTTP_GET,
			std::bind(compressedStaticHandler, 200, content_type, start, end,
					_1));
}

void web::registerReplacingStaticHandler(const char *uri,
		const String &content_type, const char *page,
		const std::map<String, std::function<std::string()>> replacements) {
	using namespace std::placeholders;
	registerRequestHandler(uri, HTTP_GET,
			std::bind<
					ResponseData(
							const std::map<String, std::function<std::string()>>,
							const uint16_t, const String&, const uint8_t*,
							const uint8_t*, AsyncWebServerRequest*)>(
					replacingRequestHandler, replacements, 200, content_type,
					(uint8_t*) page, (uint8_t*) page + strlen(page), _1));
}

void web::registerRedirect(const char *uri, const char *target) {
	using namespace std::placeholders;
	registerRequestHandler(uri, HTTP_ANY,
			std::bind(redirectHandler, target, _1));
}
#endif /* ENABLE_WEB_SERVER == 1 */
