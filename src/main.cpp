#include <SDL.h>
#include <SDL_image.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>

#include <iostream>

#include "CamReader.h"

#define MILLESECONDS_PER_FRAME 1000.0 / 120.0 /* about 120 frames per second */
//#define MILLESECONDS_PER_FRAME 1000.0/60.0       /* about 60 frames per second */

bool InitEverything();
bool InitSDL();
bool CreateWindow();
void SetupRenderer();
bool CreateRenderer();
void RunGame();
void HandleInput();
void Render();

SDL_Rect windowRect = { 100, 100, 1024, 600 };

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* imageFrame = nullptr;
bool quit = false;

uint8_t* buffer;
CamReader* camReader = nullptr;
bool newPixels = false;
unsigned char* _camPixel = nullptr;
int _camPixelSize = -1;
SDL_mutex* _camPixelLock;

void NewPixels(void* camPixel, int size)
{
    SDL_LockMutex(_camPixelLock);
    if(_camPixelSize != size) {
        if(_camPixel != nullptr) {
            delete[] _camPixel;
        }
        _camPixel = new unsigned char[size];
        _camPixelSize = size;
    }
    memcpy(_camPixel, camPixel, _camPixelSize);
    newPixels = true;
    SDL_UnlockMutex(_camPixelLock);
}

int main(int argc, char** argv)
{
    FILE* fout;
    _camPixelLock = SDL_CreateMutex();
    // Grabber Cam Supports 720x480 NTSC
    camReader = new CamReader("/dev/video0");
    auto result = camReader->Init(720, 480);
    if(result < 0) {
        exit(EXIT_FAILURE);
    }

    camReader->GetPictureSize(windowRect.w, windowRect.h);

    if(!InitEverything()) {
		std::cout << "Failed to start SDL2" << std::endl;
	}


    // SDL_PIXELFORMAT_YV12 -> PIX_FMT_YUV420P (FFMpeg)  SDL_UpdateYUVTexture
    // Big Problem the Texture muss have the Same Vormat like Cam not so many
    // imageFrame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
    uint32_t pixelFormat = SDL_PIXELFORMAT_YV12;
    if(camReader->GetPixelFormat() == V4L2_PIX_FMT_YUYV) {
        pixelFormat = SDL_PIXELFORMAT_YUY2;
    } else if(camReader->GetPixelFormat() == V4L2_PIX_FMT_UYVY) {
        pixelFormat = SDL_PIXELFORMAT_UYVY;
    }
	
    imageFrame = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, windowRect.w, windowRect.h);

    camReader->StartStream(&NewPixels);

    auto delay = static_cast<int>(MILLESECONDS_PER_FRAME);
    while(!quit) {
        const auto startFrame = SDL_GetTicks();

        if(newPixels) {
            SDL_LockMutex(_camPixelLock);
            void* pixels;
            int pitch;

            if(SDL_LockTexture(imageFrame, NULL, &pixels, &pitch) == 0) {
                memcpy(pixels, _camPixel, pitch * windowRect.h);
                SDL_UnlockTexture(imageFrame);
            }
            SDL_UnlockMutex(_camPixelLock);
            newPixels = false;
        }
        // Todo Show on screen
        // auto rw = SDL_RWFromConstMem(buffers[buf.index].start, buf.bytesused);
        // imageFrame = IMG_LoadTexture_RW(renderer, rw, 0);
        // auto size = camReader->GetNextFrame(camPixel);

        /*sprintf(out_name, "out%03d.ppm", i);
        fout = fopen(out_name, "w");
        if (!fout) {
                perror("Cannot open image");
                exit(EXIT_FAILURE);
        }
        fprintf(fout, "P6\\n%d %d 255\\n",
                fmt.fmt.pix.width, fmt.fmt.pix.height);
        fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
        fclose(fout);*/

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, imageFrame, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN: {
					const Uint8* state = SDL_GetKeyboardState(nullptr);
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE:
							quit = true;
							break;
						case SDLK_q:
							if(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL]) {
								quit = true;
							}
							break;
					}
				}
            }
        }

        const auto endFrame = SDL_GetTicks();

        /* figure out how much time we have left, and then sleep */
        delay = static_cast<int>(MILLESECONDS_PER_FRAME - (endFrame - startFrame));
        if(delay < 0) {
            delay = 0;
        } else if(delay > MILLESECONDS_PER_FRAME) {
            delay = static_cast<int>(MILLESECONDS_PER_FRAME);
        }

        SDL_Delay(delay);
    }

    camReader->StopStream();
    camReader->Deinit();

    if(_camPixel != nullptr) {
        delete[] _camPixel;
    }
    return 0;
}

bool InitEverything()
{
    if(!InitSDL()) return false;

    if(!CreateWindow()) return false;

    if(!CreateRenderer()) return false;

    SetupRenderer();

    return true;
}

bool InitSDL()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        std::cout << " Failed to initialize SDL : " << SDL_GetError() << std::endl;
        return false;
    }

    if(IMG_Init(IMG_INIT_JPG) == -1) {
        std::cout << " Failed to initialize SDL_Image : " << IMG_GetError() << std::endl;
        return false;
    }
    return true;
}

bool CreateWindow()
{
    // window = SDL_CreateWindow( "Server", posX, posY, sizeX, sizeY, 0 );
    window = SDL_CreateWindow("Rear Cam", windowRect.x, windowRect.y, windowRect.w, windowRect.h, 0);

    if(window == nullptr) {
        std::cout << "Failed to create window : " << SDL_GetError();
        return false;
    }

    return true;
}

bool CreateRenderer()
{
    renderer = SDL_CreateRenderer(window, -1, 0);

    if(renderer == nullptr) {
        std::cout << "Failed to create renderer : " << SDL_GetError();
        return false;
    }

    return true;
}

void SetupRenderer()
{
    // Set size of renderer to the same as window
    SDL_RenderSetLogicalSize(renderer, windowRect.w, windowRect.h);

    // Set color of renderer to red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
}
