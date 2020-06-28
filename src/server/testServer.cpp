#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "../common/message.h"
#include <SDL.h>

static constexpr int screenHeight{600};
static constexpr  int screenWidth{800};
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;


char buffer[sizeof(Message)];

void doWork(int* clientSocket) {
	if (clientSocket == nullptr) {
		std::cerr << "Connected socket was no good\n";
	}
}

Message readMessageFromSocket(int fd) {
	int numberOfBytesRead = 0;
	auto bytesLeft = sizeof(MessageHeader);
	bool headerRead = false;

	Message msg;
	MessageHeader header;
	char *buf = buffer;
	bool done = false;

	while (!done) {
		int bytesReceived = recv(fd, buf, bytesLeft, 0);
		if (bytesReceived !=  -1) {
			// Increment buffer pos
			// and bytes left
			buf += bytesReceived;
			bytesLeft -= bytesReceived;
			// If we have read enough bytes to have a MessageHeader
			if (bytesLeft == 0) {
				// Is this the header
				if (!headerRead) {
					memcpy(&header, buffer, sizeof(MessageHeader));
					std::cout << "We found a header with length: " << header.length << std::endl;
					headerRead = true;
					// We are only missing they payload section now
					bytesLeft = header.payloadSize;
				}
				else {
					// We must be done reading the message
					done = true;
				}
			}
		}
		else {
			std::cerr << "Could not read from socket\n";
			break;
		}
	}
	memcpy(&msg, buffer, header.length);
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
	case MsgType::FrameBuf24Bit:
		std::cout << "Received FrameBuf24Bit with payload length: " << msg.payloadSize << std::endl;
		break;
	}
}

bool initSDL()
{
	bool success = true;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("SDL could not be initialized! SDL_ERROR: %s\n", SDL_GetError());
		success = false;
	} else
	{
		// Create window
		window = SDL_CreateWindow("SDL server test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
		if (window) {
			// Create renderer for window_
			renderer = SDL_CreateRenderer(window, -1, 0);
			if (!renderer) {
				SDL_Log("Could not create renderer! SDL_ERROR: %s\n", SDL_GetError());
				success = false;
			}
			else {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
			}
		}
		else {
			SDL_Log("Could not create window! SDL_ERROR: ‰s \n", SDL_GetError());
			success = false;
		}
	}
	return success;
}

int main(int argc, char *argv[])
{

	if (!initSDL()) {
		return -1;
	}


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

	// Blocking
	new_fd = accept(serverSocket, (struct sockaddr *)&their_addr, &addr_size);

	bool done = false;

	while(!done)
	{
		SDL_Event evt;
		while(SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				SDL_Log("Quit event\n");
				done = true;
				break;
			default:
				break;
			}
		}

		// Now we have a client connected lets get to work
		// Read from socket
		const auto msg = readMessageFromSocket(new_fd);
		handleMsg(msg);

		if (msg.type == MsgType::FrameBuf24Bit) {
			SDL_RenderClear(renderer);

			SDL_Surface* framebufferSurface = SDL_CreateRGBSurfaceWithFormatFrom((void*)msg.payload, screenWidth, screenHeight, 24, 3 * screenWidth, SDL_PIXELFORMAT_RGB888);
			SDL_Texture* framebufferTexture = SDL_CreateTextureFromSurface(renderer, framebufferSurface);
			SDL_RenderCopy(renderer, framebufferTexture, NULL, NULL);

			int w, h;
			unsigned int pxFormat = SDL_GetWindowPixelFormat(window);
			SDL_QueryTexture(framebufferTexture, &pxFormat, NULL, &w, &h);
			const char* surfacePixelFormatName = SDL_GetPixelFormatName(pxFormat);
			SDL_Log("The surface's pixelformat is %s", surfacePixelFormatName);



			SDL_RenderPresent(renderer);
			SDL_DestroyTexture(framebufferTexture);
			SDL_FreeSurface(framebufferSurface);
		}
		//close(new_fd);
	}

	close(serverSocket);
	freeaddrinfo(res);
	return 0;
}
