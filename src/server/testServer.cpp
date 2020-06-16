#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "../common/message.h"


char buffer[sizeof(Message)];


void doWork(int* clientSocket) {
	if (clientSocket == nullptr) {
		std::cerr << "Connected socket was no good\n";
	}
}

Message readMessageFromSocket(int fd) {
	int numberOfBytesRead = 0;
	int bytesToRead = 2;
	// We must read two bytes before we know the size of the packet
	Message msg;

	while (numberOfBytesRead < bytesToRead) {
		int bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
		if (bytesReceived !=  -1) {
			numberOfBytesRead += bytesReceived;
			if (numberOfBytesRead >= 2) {
				// read lenght of message
				uint16_t length = static_cast<uint16_t>(buffer[0]);
				bytesToRead = length;
			}
		}
		else {
			std::cerr << "Could not read from socket\n";
		}
	}
	memcpy(&msg, buffer, bytesToRead);
	return msg;
}

void handleMsg(const Message& msg) {
	switch (msg.type) {
	case MsgType::EMPTY:
		std::cerr << "What are we gonna do with an empty message\n";
	break;
	case MsgType::CHAR:
		break;
	case MsgType::CString:
		std::cout << std::string(msg.payload);
		break;
	case MsgType::Event:
		break;
	case MsgType::FrameBuf16Bit:
		break;
	}
}


int main(int argc, char *argv[])
{
	std::cout << "Size of Msg:" << sizeof(Message) << std::endl;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int serverSocket, new_fd;
	addrinfo hints, *res;
	int backlog = 10;

	if (argc < 2) {
		std::cerr << "Port must be given as argument\n";
		return EXIT_FAILURE;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;            // use IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;        // fill in my IP for me

	// TODO: Maybe not use argv directly
	getaddrinfo(NULL, argv[1], &hints, &res);

	// TODO: Error handling! of cause nothing will ever go wrong.. right :)
	// Create server socket
	serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	// Bind to it
	bind(serverSocket, res->ai_addr, res->ai_addrlen);

	// Listen for new connections
	listen(serverSocket, backlog);

	addr_size = sizeof their_addr;
	// Accept loop
	//while(true)
	{
		// Blocking
		new_fd = accept(serverSocket, (struct sockaddr *)&their_addr, &addr_size);
		// Now we have a client connected lets get to work
		// Read from socket
		const auto msg = readMessageFromSocket(new_fd);
		handleMsg(msg);
		//close(new_fd);
	}

	close(serverSocket);
	freeaddrinfo(res);
	return 0;
}
