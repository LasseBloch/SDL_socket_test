#include <iostream>
#include <netdb.h>
#include <zconf.h>
#include <SDL.h>
#include "../common/message.h"

static constexpr int screenHeight{600};
static constexpr  int screenWidth{800};
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;

// TODO: Move into context struct
auto spriteIndex = 0;
auto frameTimeMs = 16;
auto startTime = 0;
auto endTime = 0;
auto deltaTime = 0;
bool done = false;
SDL_Surface *screenDump;

char buffer[sizeof(Message)];

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
		window = SDL_CreateWindow("SDL client test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
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

SDL_Texture* loadSpriteSheet() {
	SDL_Surface * image = SDL_LoadBMP("../assets/trump_run.bmp");
	if (!image) {
		SDL_Log("Could not load image! SDL_ERROR: %s\n", SDL_GetError());
		return nullptr;
	}
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
	return texture;
}

void renderSprite(int spriteIndex) {

	// Cut the correct frame from the sprite sheet
	SDL_Rect srcrect = { 100 * spriteIndex, 0, 100, 100 };

	SDL_Rect dstrect = {screenWidth / 2 - 50, screenHeight / 2 - 50, 100, 100 };
	SDL_SetRenderDrawColor(renderer, 168, 230, 255, 255);
	SDL_RenderClear(renderer);
	//unsigned int  pxFormat = SDL_GetWindowPixelFormat(window);
	//const char* surfacePixelFormatName = SDL_GetPixelFormatName(pxFormat);
	//SDL_Log("The surface's pixelformat is %s", surfacePixelFormatName);
	SDL_RenderCopy(renderer, texture, &srcrect, &dstrect);
	SDL_RenderPresent(renderer);
}

void sendScreenDump(int socket, const SDL_Surface& screenDump) {
	Message msg;
	int framebufSize = screenDump.h * screenDump.w * screenDump.format->BytesPerPixel;
	msg.type = MsgType::FrameBuf24Bit;
	msg.payloadSize = framebufSize;
	msg.length = sizeof(MessageHeader) + framebufSize;
	std::cout << "Sending a packet with length:" << msg.length << std::endl;
	memcpy(msg.payload, screenDump.pixels, framebufSize);

	memcpy(buffer, &msg, msg.length);
	char* buf = buffer;
	auto sendBytes = 0;
	auto bytesLeft = msg.length;
	while (sendBytes < msg.length) {
		int justSend = send(socket, buf, bytesLeft, 0);
		if (justSend != -1) {
			sendBytes += justSend;
			buf += justSend;
			bytesLeft -= justSend;
		}
		else {
			std::cerr << "could not send message \n";
			break;
		}
	}
}

void gameLoop() {
	SDL_Event evt;
	if (!startTime) {
		// ms since start
		startTime = SDL_GetTicks();
	}
	else {
		deltaTime = endTime - startTime; // how many ms for a frame
	}

	// Handle events
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

	// We have 6 sprites
	spriteIndex++;
	if (spriteIndex == 6) {
		spriteIndex = 0;
	}

	renderSprite(spriteIndex);

	// Sleep the delta between frameTimeMs and the delta time (To hit frameTimeMs pr frame).
	if (deltaTime < frameTimeMs) {
		SDL_Delay(frameTimeMs - deltaTime);
	}

	startTime = endTime;
	endTime = SDL_GetTicks();
}

int main(int argc, char *argv[])
{
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

	if (!initSDL()) {
		return -1;
	}

	texture = loadSpriteSheet();
	if (!texture) {
		return -1;
	}

	while(!done) {
		std::cout << "Before game loop \n";
		gameLoop();
		std::cout << "After game loop \n";
		// Grap screen dump
		std::cout << "Start screen dump \n";
		screenDump = SDL_CreateRGBSurface(0, screenWidth, screenHeight, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
		SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGB888, screenDump->pixels, screenDump->pitch);
		std::cout << "Screen dump done \n";
		sendScreenDump(socketFd, *screenDump);
		std::cout << "Send screen dump \n";
	}

	close(socketFd);
}
