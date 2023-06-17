/*
 * AsyncHeadOnlyResponse.cpp
 *
 *  Created on: Jun 16, 2023
 *      Author: ToMe25
 */

#include "AsyncHeadOnlyResponse.h"

#if ENABLE_WEB_SERVER == 1
web::AsyncHeadOnlyResponse::AsyncHeadOnlyResponse(
		AsyncWebServerResponse *wrapped, const int status_code) :
		AsyncBasicResponse(status_code), _wrapped(wrapped) {

}

web::AsyncHeadOnlyResponse::~AsyncHeadOnlyResponse() {
	delete _wrapped;
}

String web::AsyncHeadOnlyResponse::_assembleHead(uint8_t version) {
	return _wrapped->_assembleHead(version);
}

bool web::AsyncHeadOnlyResponse::_sourceValid() const {
	return _wrapped->_sourceValid();
}

void web::AsyncHeadOnlyResponse::setCode(const int code) {
	_wrapped->setCode(code);
	AsyncWebServerResponse::setCode(code);
}

int web::AsyncHeadOnlyResponse::getCode() const {
	return _code;
}

void web::AsyncHeadOnlyResponse::setContentType(const String& type) {
	_wrapped->setContentType(type);
}

void web::AsyncHeadOnlyResponse::addHeader(const String& name, const String& value) {
	_wrapped->addHeader(name, value);
}
#endif /* ENABLE_WEB_SERVER == 1 */
