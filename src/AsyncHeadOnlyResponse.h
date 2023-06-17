/*
 * AsyncHeadOnlyResponse.h
 *
 *  Created on: Jun 16, 2023
 *      Author: ToMe25
 */

#ifndef SRC_ASYNCHEADONLYRESPONSE_H_
#define SRC_ASYNCHEADONLYRESPONSE_H_

#include "config.h"
#if ENABLE_WEB_SERVER == 1
#include <ESPAsyncWebServer.h>
namespace web {

/**
 * A response wrapper that sends the exact head of the wrapped response.
 * It doesn't, however, send any body at all.
 * This type of response should only be used to respond to HEAD requests.
 *
 * Adding headers or changing the content type redirects said operation to the wrapped response.
 * Setting the status code, using the method or constructor, sets the code
 * in both this response and the wrapped response.
 * This allows retrieving the status code from this response later.
 */
class AsyncHeadOnlyResponse: public AsyncBasicResponse {
private:
	/**
	 * The wrapped response, the headers of which will be sent.
	 */
	AsyncWebServerResponse *_wrapped;
public:
	/**
	 * Creates a new head only response wrapping the given response.
	 * This type of response should only be used to respond to HEAD requests.
	 *
	 * @param wrapped	The response to get the head from.
	 * @param code		The status code of the wrapped response.
	 */
	AsyncHeadOnlyResponse(AsyncWebServerResponse *wrapped, const int code = 200);

	/**
	 * Destroys this response, as well as the wrapped response.
	 */
	virtual ~AsyncHeadOnlyResponse();

	/**
	 * Assembles the response head to send to the client.
	 *
	 * @param version	Whether the request is HTTP 1.0 or 1.1+.
	 * @return	The assembled response head.
	 */
	virtual String _assembleHead(uint8_t version) override;

	/**
	 * Checks whether the data source of the wrapped response is valid.
	 *
	 * @return	True of the data source is valid.
	 */
	virtual bool _sourceValid() const override;

	/**
	 * Sets the status code of this response, as well as the wrapped response.
	 *
	 * @param code	The status code to send to the client.
	 */
	virtual void setCode(const int code) override;

	/**
	 * Gets the status code set for this response.
	 * SHOULD match the status code of the wrapped response, but is not guaranteed to.
	 *
	 * @return	The status code set for this response.
	 */
	virtual int getCode() const;

	/**
	 * Sets the content type of the wrapped response, to be sent to the client.
	 *
	 * @param type	The content type to send to the client.
	 */
	virtual void setContentType(const String& type) override;

	/**
	 * Adds the given header to the wrapped response.
	 * This will cause it to be sent to the client.
	 *
	 * @param name	The name of the header to send to the client.
	 * @param value	The value of the header to send to the client.
	 */
	virtual void addHeader(const String& name, const String& value) override;
};

}
#endif /* ENABLE_WEB_SERVER == 1 */
#endif /* SRC_ASYNCHEADONLYRESPONSE_H_ */
