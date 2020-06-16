//
// Created by Lasse Bloch Lauritsen on 13/06/2020.
//

#ifndef SDL_SOCKETTEST_MESSAGE_H
#define SDL_SOCKETTEST_MESSAGE_H

#include <cstdint>
// TODO : This is crazy! but ok for now
// Could buffer be a char vector? or is that just plain stupid?
const int bufferSize = 1024 * 768 * 3 + 5;

enum class MsgType : uint8_t {
	EMPTY,
	CHAR,
	FrameBuf16Bit,
	Event,
	CString,
};

struct MessageHeader {
	uint16_t length;
	MsgType type {MsgType::EMPTY};
	uint16_t payloadSize;
};


struct Message : public MessageHeader {
	char payload[bufferSize];
	Message() = default;
	Message(std::string str) {
		type = MsgType::CString;
		payloadSize = str.length() + 1; // 1 for null termination
		char buffer[payloadSize];
		str.copy(buffer, payloadSize);
		buffer[str.length()] = '\0';
		std::memcpy(&payload, &buffer, payloadSize);

		// set length
		length = sizeof(MessageHeader) + payloadSize;
	}

};




#endif //SDL_SOCKETTEST_MESSAGE_H
