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

#include "AsyncTrackingFallbackWebHandler.h"
#include <uzlib_gzip_wrapper.h>
#include <map>

/**
 * A pointer to the first byte of the templated main page of the web interface.
 */
extern const uint8_t INDEX_HTML_START[] asm("_binary_data_index_html_start");

/**
 * A pointer to the first byte after the templated main page of the web interface.
 */
extern const uint8_t INDEX_HTML_END[] asm("_binary_data_index_html_end");

/**
 * A pointer to the first byte of the gzip compressed main css stylesheet.
 */
extern const uint8_t MAIN_CSS_START[] asm("_binary_data_gzip_main_css_gz_start");

/**
 * A pointer to the first byte after the gzip compressed main css stylesheet.
 */
extern const uint8_t MAIN_CSS_END[] asm("_binary_data_gzip_main_css_gz_end");

/**
 * A pointer to the first byte of the gzip compressed main page javascript file.
 */
extern const uint8_t INDEX_JS_START[] asm("_binary_data_gzip_index_js_gz_start");

/**
 * A pointer to the first byte after the gzip compressed main page javascript file.
 */
extern const uint8_t INDEX_JS_END[] asm("_binary_data_gzip_index_js_gz_end");

/**
 * A pointer to the first byte of the gzip compressed web app manifest.
 */
extern const uint8_t MANIFEST_JSON_START[] asm("_binary_data_gzip_manifest_json_gz_start");

/**
 * A pointer to the first byte after the gzip compressed web app manifest.
 */
extern const uint8_t MANIFEST_JSON_END[] asm("_binary_data_gzip_manifest_json_gz_end");

/**
 * A pointer to the first byte of the error html page.
 */
extern const uint8_t ERROR_HTML_START[] asm("_binary_data_error_html_start");

/**
 * A pointer to the first byte after the error html page.
 */
extern const uint8_t ERROR_HTML_END[] asm("_binary_data_error_html_end");

/**
 * A pointer to the first byte of the gzip compressed favicon ico.
 */
extern const uint8_t FAVICON_ICO_GZ_START[] asm("_binary_data_gzip_favicon_ico_gz_start");

/**
 * A pointer to the first byte after the gzip compressed favicon ico.
 */
extern const uint8_t FAVICON_ICO_GZ_END[] asm("_binary_data_gzip_favicon_ico_gz_end");

/**
 * A pointer to the first byte of the gzip compressed favicon png.
 */
extern const uint8_t FAVICON_PNG_GZ_START[] asm("_binary_data_gzip_favicon_png_gz_start");

/**
 * A pointer to the first byte after the gzip compressed favicon png.
 */
extern const uint8_t FAVICON_PNG_GZ_END[] asm("_binary_data_gzip_favicon_png_gz_end");

/**
 * A pointer to the first byte of the gzip compressed favicon svg.
 */
extern const uint8_t FAVICON_SVG_GZ_START[] asm("_binary_data_gzip_favicon_svg_gz_start");

/**
 * A pointer to the first byte after the gzip compressed favicon svg.
 */
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
 * The Cache-Control header value to send for pages that should not be cached.
 */
static constexpr char CACHE_CONTROL_NOCACHE[] = "no-store";

/**
 * The Cache-Control header value to send for pages that may be cached.
 */
static constexpr char CACHE_CONTROL_CACHE[] = "public, no-cache";

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
 * Checks whether the given comma separated values header accepts the has the given value as an option.
 *
 * This function is case sensitive.
 *
 * The value has be exactly match one of the given options, excluding the factors after semicolons.
 * While this function can, in theory, also match these factors, this requires the order of the factors to match, not just their values.
 *
 * Note: While spaces after commas are allowed, spaces after values are not at this time.
 *
 * @param header	The header value to check.
 * @param value		The value to look for.
 * @return	True if the given value is accepted by the client.
 */
bool csvHeaderContains(const char *header, const char *value);

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
 * An AwsResponseFiller decompressing a file from memory using uzlib.
 *
 * @param decomp	The uzlib decompressing persistent data.
 * @param buffer	The output buffer to write the decompressed data to.
 * @param max_len	The max number of bytes to write to the output buffer.
 * @param index		The number of bytes already generated for this response.
 * @return	The number of bytes written to the output buffer.
 */
size_t decompressingResponseFiller(
		const std::shared_ptr<gzip::uzlib_ungzip_wrapper> decomp,
		uint8_t *buffer, const size_t max_len, const size_t index);

/**
 * An AwsResponseFiller copying the given static file, and replacing the given template strings.
 * If the output buffer can hold more data than available, the remainder is filled with null bytes.
 *
 * @param replacements	A map containing the template strings to replace, and their replacement values.
 * @param offset		The offset between the current static page position and output index.
 * 						Will automatically be updated by this function.
 * @param start			A pointer to the first byte of the static page.
 * @param end			A pointer to the first byte after the static page.
 * 						For C strings this is the terminating NUL byte.
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
 * An AwsResponseFiller doing absolutely nothing, used to avoid sending the content length of 0.
 *
 * @param buffer	The output buffer a real response filler would write to.
 * @param max_len	The max number of bytes to write to the output buffer.
 * @param index		The number of bytes already written to the client.
 * @return	Zero. Always.
 */
size_t dummyResponseFiller(const uint8_t *buffer, const size_t max_len,
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
 * Automatically adds a "default-src 'self'" content security policy to "text/html" responses.
 *
 * Allows ETag based caching, if an etag was given.
 *
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the static file.
 * @param start			A pointer to the first byte of the compressed static file.
 * @param end			A pointer to the first byte after the end of the compressed static file.
 * 						For C strings this is the terminating NUL byte.
 * @param request		The request to handle.
 * @param etag			The HTTP entity tag to use for caching.
 * 						Use NULL to disable sending an ETag for this page.
 * @return	The response to be sent to the client.
 */
ResponseData staticHandler(const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request, const char *etag = NULL);

/**
 * A web request handler for a compressed static file.
 *
 * If the client accepts gzip compressed files, the file is sent as is.
 * Otherwise it is decompressed on the fly.
 *
 * Automatically adds a "default-src 'self'" content security policy to "text/html" responses.
 *
 * Allows ETag based caching, if an etag was given.
 *
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the static file.
 * @param start			A pointer to the first byte of the compressed static file.
 * @param end			A pointer to the first byte after the end of the compressed static file.
 * 						For C strings this is the terminating NUL byte.
 * @param request		The request to handle.
 * @param etag			The HTTP entity tag to use for caching.
 * 						Use NULL to disable sending an ETag for this page.
 * @return	The response to be sent to the client.
 */
ResponseData compressedStaticHandler(const uint16_t status_code,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		AsyncWebServerRequest *request, const char *etag = NULL);

/**
 * A web request handler for a static file with some templates to replace.
 * Template strings have to be formatted like this: $TEMPLATE$.
 *
 * The templates will be replaced with the result of the function registered for them.
 * Note that each function will be called once, no matter how often its template appears.
 *
 * Automatically adds a "default-src 'self'" content security policy to "text/html" responses.
 *
 * This request handler automatically adds a Cache-Control header forbidding caching, since the page is dynamic.
 *
 * @param replacements	A map mapping a template string to be replaced,
 * 						to a function returning its replacement value.
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the file to send.
 * @param start			A pointer to the first byte of the static file.
 * @param end			A pointer to the first byte after the static file.
 * 						For C strings this is the terminating NUL byte.
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
 * Automatically adds a "default-src 'self'" content security policy to "text/html" responses.
 *
 * This request handler automatically adds a Cache-Control header forbidding caching, since the page is dynamic.
 *
 * @param replacements	A map mapping a template string to be replaced,
 * 						to its replacement string.
 * @param status_code	The HTTP response status code to send to the client.
 * @param content_type	The content type of the file to send.
 * @param start			A pointer to the first byte of the static file.
 * @param end			A pointer to the first byte after the static file.
 * 						For C strings this is the terminating NUL byte.
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
 *
 * Will automatically increment the prometheus request counter.
 * Allows ETag based caching, if an etag was given.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param page			The content for the page to be sent to the client.
 * @param etag			The HTTP entity tag to use for caching.
 * 						Use NULL to disable sending an ETag for this page.
 */
void registerStaticHandler(const char *uri, const String &content_type,
		const char *page, const char *etag = NULL);

/**
 * Registers a request handler that returns the given content type and web page each time it is called.
 * Registers request handlers for the request methods GET, HEAD, and OPTIONS.
 * Always sends response code 200.
 *
 * Will automatically increment the prometheus request counter.
 * Allows ETag based caching, if an etag was given.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param start			The pointer to the first byte of the file.
 * @param end			The pointer to the first byte after the end of the file.
 * 						For C strings this is the terminating NUL byte.
 * @param etag			The HTTP entity tag to use for caching.
 * 						Use NULL to disable sending an ETag for this page.
 */
void registerStaticHandler(const char *uri, const String &content_type,
		const uint8_t *start, const uint8_t *end, const char *etag = NULL);

/**
 * Registers a request handler that returns the given content each time it is called.
 * Registers request handlers for the request methods GET, HEAD, and OPTIONS.
 * Always sends response code 200.
 *
 * Will automatically increment the prometheus request counter.
 * Expects the content to be a gzip compressed binary.
 * Allows ETag based caching, if an etag was given.
 *
 * @param uri			The path on which the file can be found.
 * @param content_type	The content type for the file.
 * @param start			The pointer to the first byte of the file.
 * @param end			The pointer to the first byte after the end of the file.
 * 						For C strings this is the terminating NUL byte.
 * @param etag			The HTTP entity tag to use for caching.
 * 						Use NULL to disable sending an ETag for this page.
 */
void registerCompressedStaticHandler(const char *uri,
		const String &content_type, const uint8_t *start, const uint8_t *end,
		const char *etag = NULL);

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
 * This request handler automatically adds a Cache-Control header forbidding caching, since the page is dynamic.
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
 * This request handler automatically adds a Cache-Control header forbidding caching, since the page is dynamic.
 *
 * @param uri			The path on which the page can be found.
 * @param content_type	The content type for the page.
 * @param start			The pointer to the first byte of the file.
 * @param end			The pointer to the first byte after the end of the file.
 * 						For C strings this is the terminating NUL byte.
 * @param replacements	A map mapping a template string to be replaced,
 * 						to a function returning its replacement value.
 */
void registerReplacingStaticHandler(const char *uri, const String &content_type,
		const uint8_t *start, const uint8_t *end,
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
