/*
 * webhander.h
 *
 *  Created on: May 30, 2023
 *      Author: ToMe25
 */

#ifndef SRC_WEBHANDLER_H_
#define SRC_WEBHANDLER_H_

#include "config.h"
#if ENABLE_WEB_SERVER == 1
#include "uzlib_gzip_wrapper.h"
#include <ESPAsyncWebServer.h>
#endif

#if ENABLE_WEB_SERVER == 1
extern const char INDEX_HTML[] asm("_binary_data_index_html_start");
extern const uint8_t MAIN_CSS_START[] asm("_binary_data_gzip_main_css_gz_start");
extern const uint8_t MAIN_CSS_END[] asm("_binary_data_gzip_main_css_gz_end");
extern const uint8_t INDEX_JS_START[] asm("_binary_data_gzip_index_js_gz_start");
extern const uint8_t INDEX_JS_END[] asm("_binary_data_gzip_index_js_gz_end");
extern const uint8_t MANIFEST_JSON_START[] asm("_binary_data_gzip_manifest_json_gz_start");
extern const uint8_t MANIFEST_JSON_END[] asm("_binary_data_gzip_manifest_json_gz_end");
extern const uint8_t NOT_FOUND_HTML_START[] asm("_binary_data_gzip_not_found_html_gz_start");
extern const uint8_t NOT_FOUND_HTML_END[] asm("_binary_data_gzip_not_found_html_gz_end");
extern const uint8_t FAVICON_ICO_GZ_START[] asm("_binary_data_gzip_favicon_ico_gz_start");
extern const uint8_t FAVICON_ICO_GZ_END[] asm("_binary_data_gzip_favicon_ico_gz_end");
extern const uint8_t FAVICON_PNG_GZ_START[] asm("_binary_data_gzip_favicon_png_gz_start");
extern const uint8_t FAVICON_PNG_GZ_END[] asm("_binary_data_gzip_favicon_png_gz_end");
extern const uint8_t FAVICON_SVG_GZ_START[] asm("_binary_data_gzip_favicon_svg_gz_start");
extern const uint8_t FAVICON_SVG_GZ_END[] asm("_binary_data_gzip_favicon_svg_gz_end");
#endif

/**
 * The namespace for all the web server related stuff in this project.
 */
namespace web {
#if ENABLE_WEB_SERVER == 1
typedef std::function<uint16_t(AsyncWebServerRequest *request)> HTTPRequestHandler;

/**
 * The global async web server instance used by this handler.
 */
extern AsyncWebServer server;
#endif
/**
 * Initializes the web server and the related components.
 */
void setup();

/**
 * A method that does everything that should be done every loop iteration.
 */
void loop();

/**
 * A handler for things that should happen when a new WiFi connection is established.
 */
void connect();

#if ENABLE_WEB_SERVER == 1
/**
 * Process the templates for the index page.
 *
 * @param temp	The template to replace.
 * @return	The value replacing the template.
 */
const String processIndexTemplates(const String &temp);

/**
 * The request handler for /data.json.
 * Responds with a json object containing the current temperature and humidity,
 * as well as the time since the last measurement.
 *
 * @param request	The web request to handle.
 * @return	The returned http status code.
 */
uint16_t getJson(AsyncWebServerRequest *request);

/**
 * A AwsResponseFiller decompressing a file from memory using uzlib.
 *
 * @param decomp	The uzlib decompressing persistent data.
 * @param buffer	The output buffer to write the decompressed data to.
 * @param max_len	The max number of bytes to write to the output buffer.
 * @param index		The number of bytes already generated for this response.
 * @return	The number of bytes written to the output buffer.
 */
size_t decompressingResponseFiller(const std::shared_ptr<uzlib_gzip_wrapper> decomp,
		uint8_t *buffer, const size_t max_len, const size_t index);

/**
 * The method to be called by the AsyncWebServer to call a request handler.
 * Calls the handler and updates the prometheus web request statistics.
 *
 * @param uri		The uri to be handled by the request handler.
 * @param handler	The request handler to be wrapped by this method.
 * @param request	The request to be handled.
 */
void trackingRequestHandlerWrapper(const char *uri,
		const HTTPRequestHandler handler, AsyncWebServerRequest *request);

/**
 * A web request handler for a compressed static file.
 * If the client accepts gzip compressed files, the file is sent as is.
 * Otherwise it is decompressed on the fly.
 *
 * @param status_code	The http response status code to send to the client.
 * @param content_type	The content type of the static file.
 * @param start			A pointer to the first byte of the compressed static file.
 * @param end			A pointer to the first byte after the end of the compressed static file.
 * @param request		The request to handle.
 * @return	The sent status code. Always the same as the status_code argument.
 */
uint16_t compressedStaticHandler(const uint16_t status_code, const char *content_type, const uint8_t *start,
		const uint8_t *end, AsyncWebServerRequest *request);

/**
 * Registers the given handler for the web server, and increments the web requests counter
 * by one each time it is called.
 *
 * @param uri		The path on which the page can be found.
 * @param method	The HTTP request method for which to register the handler.
 * @param handler	A function responding to AsyncWebServerRequests and returning the response HTTP status code.
 */
void registerRequestHandler(const char *uri, WebRequestMethodComposite method,
		HTTPRequestHandler handler);

/**
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Only registers a handler for request type get.
 * Always sends response code 200.
 * Also increments the request counter.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param page			The content for the page to be sent to the client.
 */
void registerStaticHandler(const char *uri, const char *content_type,
		const char *page);

/**
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Also registers the given template processor.
 * Only registers a handler for request type get.
 * Always sends response code 200.
 * Also increments the request counter.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param page			The content for the page to be sent to the client.
 */
void registerProcessedStaticHandler(const char *uri, const char *content_type,
		const char *page, const AwsTemplateProcessor processor);

/**
 * Registers a request handler that returns the given content each time it is called.
 * Only registers a handler for request type get.
 * Always sends response code 200.
 * Also increments the request counter.
 * Expects the content to be a gzip compressed binary.
 *
 * @param uri			The path on which the file can be found.
 * @param content_type	The content type for the file.
 * @param start			The pointer for the start of the file.
 * @param end			The pointer for the end of the file.
 */
void registerCompressedStaticHandler(const char *uri, const char *content_type,
		const uint8_t *start, const uint8_t *end);
#endif /* ENABLE_WEB_SERVER */
}

#endif /* SRC_WEBHANDLER_H_ */
