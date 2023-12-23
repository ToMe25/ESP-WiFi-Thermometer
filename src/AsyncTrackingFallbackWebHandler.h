/*
 * AsyncMultitypeFallbackWebHandler.h
 *
 *  Created on: Jun 16, 2023
 *
 * Copyright (C) 2023 ToMe25.
 * This project is licensed under the MIT License.
 * The MIT license can be found in the project root and at https://opensource.org/licenses/MIT.
 */

#ifndef SRC_ASYNCTRACKINGFALLBACKWEBHANDLER_H_
#define SRC_ASYNCTRACKINGFALLBACKWEBHANDLER_H_

#include "config.h"
#if ENABLE_WEB_SERVER == 1
#include "webhandler.h"

namespace web {
/**
 * A web handler that can handle multiple request methods.
 * Also has a fallback for request types without a set handler.
 *
 * Can not handle uri templates, all uris are treated as literals.
 * Can not handle a custom upload or body handler.
 *
 * Also tracks the requests for the prometheus integration, if it is enabled.
 */
class AsyncTrackingFallbackWebHandler: public AsyncWebHandler {
protected:
	/**
	 * The uri handled by this handler.
	 * Can not be a template string.
	 */
	const String _uri;

	/**
	 * The request handler handling all request methods for which no handler was set.
	 */
	HTTPFallbackRequestHandler _fallbackHandler;

	/**
	 * A vector containing the handler for each request method.
	 * An empty function object is used for request methods with no set handler.
	 */
	std::vector<HTTPRequestHandler> _handlers;

	/**
	 * Gets the request handler for a given request method.
	 * Returns an empty function object if no handler was set for the given method.
	 *
	 * @param method	The HTTP request method to get the handler for.
	 * @return	The handler to use for this type of request.
	 */
	virtual HTTPRequestHandler _getHandler(const WebRequestMethod method) const;

public:
	/**
	 * Creates a new AsyncMultitypeFallbackWebHandler handling requests for the given uri.
	 *
	 * @param uri		The uri to be handled by the new handler.
	 * 					Can not be a template string.
	 * @param fallback	The fallback handler for request methods for which no handler was registered.
	 */
	AsyncTrackingFallbackWebHandler(const String &uri, HTTPFallbackRequestHandler fallback);

	/**
	 * Destroys this handler and all its allocated resources.
	 */
	virtual ~AsyncTrackingFallbackWebHandler();

	/**
	 * Sets the handler to be used for the given request methods.
	 * Replaces the current handler, if one was already set.
	 *
	 * @param methods	The methods for which to use the given handler.
	 * @param handler	The request handler to use for the given methods.
	 */
	virtual void setHandler(const WebRequestMethodComposite methods, HTTPRequestHandler handler);

	/**
	 * Sets the handler to be used for request methods for which no handler was set.
	 *
	 * @param handler	The fallback handler to use.
	 */
	virtual void setFallbackHandler(HTTPFallbackRequestHandler fallback);

	/**
	 * Gets the HTTP request methods for which a handler was set.
	 *
	 * @return	The methods for which a handler was set.
	 */
	virtual WebRequestMethodComposite getHandledMethods() const;

	/**
	 * The function checking whether this handler can handle the given request.
	 *
	 * @param request	The request to check.
	 * @return	True if this handler can handle the given request.
	 */
	virtual bool canHandle(AsyncWebServerRequest *request) override;

	/**
	 * The function being called for each request to be handled by this handler.
	 * Automatically tracks the request if prometheus support is enabled.
	 *
	 * @param request	The request to handle.
	 */
	virtual void handleRequest(AsyncWebServerRequest *request) override;

	/**
	 * Checks whether this request handler is trivial, meaning post requests don't need to be parsed.
	 *
	 * @return	True if this handler is trivial.
	 */
	virtual bool isRequestHandlerTrivial() override;
};
}
#endif /* ENABLE_WEB_SERVER == 1 */
#endif /* SRC_ASYNCTRACKINGFALLBACKWEBHANDLER_H_ */
