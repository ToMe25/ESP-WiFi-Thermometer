/*
 * webhander.h
 *
 *  Created on: May 30, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_WEBHANDLER_H_
#define SRC_WEBHANDLER_H_

#include "config.h"
#if ENABLE_WEB_SERVER == 1
#include <ESPAsyncWebServer.h>
namespace web {
class ResponseData;
class AsyncHeadOnlyResponse;
class AsyncTrackingFallbackWebHandler;

/**
 * A function handling HTTP requests for some set of urls.
 * Gets the HTTP request to handle.
 * Returns the response to be sent to the client.
 */
typedef std::function<ResponseData(AsyncWebServerRequest *request)> HTTPRequestHandler;

/**
 * A function handling all the HTTP request methods for which no handler was registered.
 * Gets the HTTP request methods for the uri to handle, for which handlers were registered.
 * Gets the HTTP request to handle.
 * Returns the response to be sent to the client.
 */
typedef std::function<
		ResponseData(const WebRequestMethodComposite handledMethods,
				AsyncWebServerRequest *request)> HTTPFallbackRequestHandler;
}

#include <AsyncTrackingFallbackWebHandler.h>
#include "uzlib_gzip_wrapper.h"
#include <map>

extern const char INDEX_HTML_START[] asm("_binary_data_index_html_start");
extern const char INDEX_HTML_END[] asm("_binary_data_index_html_end");
extern const uint8_t MAIN_CSS_START[] asm("_binary_data_gzip_main_css_gz_start");
extern const uint8_t MAIN_CSS_END[] asm("_binary_data_gzip_main_css_gz_end");
extern const uint8_t INDEX_JS_START[] asm("_binary_data_gzip_index_js_gz_start");
extern const uint8_t INDEX_JS_END[] asm("_binary_data_gzip_index_js_gz_end");
extern const uint8_t MANIFEST_JSON_START[] asm("_binary_data_gzip_manifest_json_gz_start");
extern const uint8_t MANIFEST_JSON_END[] asm("_binary_data_gzip_manifest_json_gz_end");
extern const char ERROR_HTML_START[] asm("_binary_data_error_html_start");
extern const char ERROR_HTML_END[] asm("_binary_data_error_html_end");
extern const uint8_t FAVICON_ICO_GZ_START[] asm("_binary_data_gzip_favicon_ico_gz_start");
extern const uint8_t FAVICON_ICO_GZ_END[] asm("_binary_data_gzip_favicon_ico_gz_end");
extern const uint8_t FAVICON_PNG_GZ_START[] asm("_binary_data_gzip_favicon_png_gz_start");
extern const uint8_t FAVICON_PNG_GZ_END[] asm("_binary_data_gzip_favicon_png_gz_end");
extern const uint8_t FAVICON_SVG_GZ_START[] asm("_binary_data_gzip_favicon_svg_gz_start");
extern const uint8_t FAVICON_SVG_GZ_END[] asm("_binary_data_gzip_favicon_svg_gz_end");

/**
 * The namespace for all the web server related stuff in this project.
 */
namespace web {
/**
 * A custom struct containing all the info required to send a response to the client.
 */
class ResponseData {
public:
	/**
	 * The actual response to send.
	 */
	AsyncWebServerResponse *response;

	/**
	 * The number of bytes the response body contains or will contain.
	 */
	size_t content_length;

	/**
	 * The HTTP status code the response sends to the client.
	 */
	uint16_t status_code;

	/**
	 * Creates a new ResponseData object from its values.
	 *
	 * @param response		The response to send to the client.
	 * @param content_len	The number of bytes of the response body to send.
	 * @param status_code	The HTTP status code this response will send.
	 */
	ResponseData(AsyncWebServerResponse *response, const size_t content_len,
			const uint16_t status_code);
};

/**
 * The character to use as a template delimiter.
 */
static const char TEMPLATE_CHAR = '$';

/**
 * The global async web server instance used by this handler.
 */
extern AsyncWebServer server;

/**
 * A map containing the registered request handler for each uri.
 */
extern std::map<String, AsyncTrackingFallbackWebHandler*> handlers;
#else /* ENABLE_WEB_SERVER == 1 */
namespace web {
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
 * The request handler for /data.json.
 * Responds with a json object containing the current temperature and humidity,
 * as well as the time since the last measurement.
 *
 * @param request	The web request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData getJson(AsyncWebServerRequest *request);

/**
 * A AwsResponseFiller decompressing a file from memory using uzlib.
 *
 * @param decomp	The uzlib decompressing persistent data.
 * @param buffer	The output buffer to write the decompressed data to.
 * @param max_len	The max number of bytes to write to the output buffer.
 * @param index		The number of bytes already generated for this response.
 * @return	The number of bytes written to the output buffer.
 */
size_t decompressingResponseFiller(
		const std::shared_ptr<uzlib_gzip_wrapper> decomp, uint8_t *buffer,
		const size_t max_len, const size_t index);

/**
 * A AwsResponseFiller copying the given static file, and replacing the given template strings.
 * If the output buffer can hold more data than available, the remainder is filled with null bytes.
 *
 * @param replacements	A map containing the template strings to replace, and their replacement values.
 * @param offset		The offset between the current static page position and output index.
 * 						Will automatically be updated by this function.
 * @param start			A pointer to the first byte of the static page.
 * @param end			A pointer to the first byte after the static page.
 * @param buffer		The output buffer to write to.
 * @param max_len		The max number of bytes to write to the output buffer.
 * @param index			The number of bytes already created by this method.
 * @return	The number of bytes written to the output buffer.
 */
size_t replacingResponseFiller(const std::map<String, String> &replacements,
		std::shared_ptr<int64_t> offset, const uint8_t *start,
		const uint8_t *end, uint8_t *buffer, const size_t max_len,
		const size_t index);

/**
 * A request handler wrapper for GET request handlers that automatically adapts them for HEAD requests.
 *
 * @param handler	The request handler to be wrapped by this method.
 * @param request	The request to be handled.
 * @return	The response to be sent to the client.
 */
ResponseData defaultHeadRequestHandlerWrapper(const HTTPRequestHandler &handler,
		AsyncWebServerRequest *request);

/**
 * The request handler for pages that couldn't be found.
 * Sends an error 404 page.
 *
 * @param request	The request to handle.
 */
void notFoundHandler(AsyncWebServerRequest *request);

/**
 * The request handler for pages that received an invalid request method.
 * Sends an error 405 page.
 *
 * @param validMethods	The request methods that will be accepted by this uri.
 * @param request		The request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData invalidMethodHandler(const WebRequestMethodComposite validMethods,
		AsyncWebServerRequest *request);

/**
 * Handles a received OPTIONS request, and responds with a 204 No Content response.
 * This response will contain an Allow header with the given methods.
 * OPTIONS will automatically be added to the start of the valid methods.
 *
 * @param validMethods	The request methods that will be accepted by this uri.
 * @param request		The request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData optionsHandler(const WebRequestMethodComposite validMethods,
		AsyncWebServerRequest *request);

/**
 * A web request handler for a static page.
 *
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the static file.
 * @param start			A pointer to the first byte of the compressed static file.
 * @param end			A pointer to the first byte after the end of the compressed static file.
 * @param request		The request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData staticHandler(const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request);

/**
 * A web request handler for a compressed static file.
 * If the client accepts gzip compressed files, the file is sent as is.
 * Otherwise it is decompressed on the fly.
 *
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the static file.
 * @param start			A pointer to the first byte of the compressed static file.
 * @param end			A pointer to the first byte after the end of the compressed static file.
 * @param request		The request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData compressedStaticHandler(const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request);

/**
 * A web request handler for a static file with some templates to replace.
 * Template strings have to be formatted like this: $TEMPLATE$.
 *
 * The templates will be replaced with the result of the function registered for them.
 * Note that each function will be called once, no matter how often its template appears.
 *
 * @param replacements	A map mapping a template string to be replaced,
 * 						to a function returning its replacement value.
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the file to send.
 * @param start			A pointer to the first byte of the static file.
 * @param end			A pointer to the first byte after the static file.
 * @param request		The request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData replacingRequestHandler(
		const std::map<String, std::function<std::string()>> replacements,
		const uint16_t status_code, const String &content_type,
		const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request);

/**
 * A web request handler for a static file with some templates to replace.
 * Template strings have to be formatted like this: $TEMPLATE$.
 *
 * @param replacements	A map mapping a template string to be replaced,
 * 						to its replacement string.
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the file to send.
 * @param start			A pointer to the first byte of the static file.
 * @param end			A pointer to the first byte after the static file.
 * @param request		The request to handle.
 * @return	The response to be sent to the client.
 */
ResponseData replacingRequestHandler(
		const std::map<String, String> replacements, const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request);

/**
 * A web request handler generating a Temporary Redirect(307) response.
 *
 * @param target	The target url to redirect the client to.
 * @param request	The request to handle.
 * @return	The response to send to the client.
 */
ResponseData redirectHandler(const char *target,
		AsyncWebServerRequest *request);

/**
 * Registers the given request handler for the given uri and request methods.
 * Will automatically increment the prometheus request counter.
 *
 * OPTIONS requests for the given uri will automatically be handled, if no OPTIONS handler is registered for the uri.
 * A HEAD request handler will automatically be registered, if no HEAD handler was registered for the uri yes,
 * and the new handler handles GET requests.
 *
 * @param uri		The path on which the page can be found.
 * @param method	The HTTP request method(s) for which to register the handler.
 * @param handler	A function responding to AsyncWebServerRequests and returning the response to send.
 */
void registerRequestHandler(const char *uri,
		const WebRequestMethodComposite method, HTTPRequestHandler handler);

/**
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Registers request handlers for the request methods GET, HEAD, and OPTIONS.
 * Always sends response code 200.
 * Will automatically increment the prometheus request counter.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param page			The content for the page to be sent to the client.
 */
void registerStaticHandler(const char *uri, const String &content_type,
		const char *page);

/**
 * Registers a request handler that returns the given content each time it is called.
 * Registers request handlers for the request methods GET, HEAD, and OPTIONS.
 * Always sends response code 200.
 * Will automatically increment the prometheus request counter.
 * Expects the content to be a gzip compressed binary.
 *
 * @param uri			The path on which the file can be found.
 * @param content_type	The content type for the file.
 * @param start			The pointer for the start of the file.
 * @param end			The pointer for the end of the file.
 */
void registerCompressedStaticHandler(const char *uri,
		const String &content_type, const uint8_t *start, const uint8_t *end);

/**
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Replaces the given strings in the static file.
 * Template strings have to be formatted like this: $TEMPLATE$.
 *
 * The templates will be replaced with the result of the function registered for them.
 * Note that each function will be called once per request, no matter how often its template appears.
 *
 * Registers request handlers for the request methods GET, HEAD, and OPTIONS.
 * Always sends response code 200.
 * Will automatically increment the prometheus request counter.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param page			The content for the page to be sent to the client.
 * @param replacements	A map mapping a template string to be replaced,
 * 						to a function returning its replacement value.
 */
void registerReplacingStaticHandler(const char *uri, const String &content_type,
		const char *page,
		const std::map<String, std::function<std::string()>> replacements);

/**
 * Registers a request handler redirecting to the given target url.
 * The request handler will be registered for all request methods.
 *
 * @param uri		The uri to register the redirect for.
 * @param target	The url to redirect to.
 */
void registerRedirect(const char *uri, const char *target);
#endif /* ENABLE_WEB_SERVER */
}

#endif /* SRC_WEBHANDLER_H_ */
