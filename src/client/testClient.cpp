#include <iostream>
#include <netdb.h>
#include <zconf.h>
#include "../common/message.h"


char buffer[sizeof(Message)];

int main(int argc, char *argv[])
{

	Message msg("Hello world! from yours truly the client");


	int socketFd;
	addrinfo hints, *p, *servinfo;
	int retVal;

	std::cout << "Hello, World! this is testClient" << std::endl;

	if (argc < 3) {
		std::cerr << "Hostname and port must be given as arguments\n";
		return EXIT_FAILURE;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;            // use IPv4
	hints.ai_socktype = SOCK_STREAM;

	if ((retVal = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo: " << gai_strerror(retVal) << std::endl;
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {

		if ((socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			std::cerr << "client: socket \n";
			continue;
		}

		if (connect(socketFd, p->ai_addr, p->ai_addrlen) == -1) {
			std::cerr << "client: connect \n";
			continue;
		}
		break;
	}

	// if we have a connection send hello world
	if (socketFd) {
		auto sendBytes = 0;
		while (sendBytes < msg.length) {
			if (int justSend = send(socketFd, &msg, msg.length, 0) && justSend != -1) {
				sendBytes += sendBytes;
			}
			else {
				std::cerr << "could not send message \n";
				break;
			}

		}
	}
	close(socketFd);
}
