

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "CamReader.h"

bool InitEverything();
bool InitSDL();
bool CreateWindow();
void SetupRenderer();
bool CreateRenderer();
void RunGame();
void HandleInput();
void Render();

SDL_Rect windowRect = { 100, 100, 320, 240 };

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* imageFrame = nullptr;
bool quit = false;

uint8_t *buffer;


int main(int argc, char **argv)
{
        FILE                            *fout;
       

        CamReader* camReader = new CamReader("/dev/video0");
        auto result = camReader->Init();
        if(result < 0) {
                exit(EXIT_FAILURE);
        }
    
        camReader->GetPictureSize(windowRect.w, windowRect.h);

        InitEverything();
        
        
        //Big Problem the Texture muss have the Same Vormat like Cam not so many
        //imageFrame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
        uint32_t pixelFormat = SDL_PIXELFORMAT_ARGB8888;
        if(camReader->GetPixelFormat() == V4L2_PIX_FMT_YUYV) {
                pixelFormat = SDL_PIXELFORMAT_YUY2;
        } else if(camReader->GetPixelFormat() == V4L2_PIX_FMT_UYVY) {
                pixelFormat = SDL_PIXELFORMAT_UYVY;
        }
        imageFrame = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, windowRect.w, windowRect.h);

        camReader->StartStream();

        
        while (!quit) {
                SDL_RenderClear( renderer );
                
                //Todo Show on screen
                //auto rw = SDL_RWFromConstMem(buffers[buf.index].start, buf.bytesused);
                //imageFrame = IMG_LoadTexture_RW(renderer, rw, 0);

                void* pixels;
                void* camPixel;
                int pitch;
                auto size = camReader->GetNextFrame(camPixel);
                if(SDL_LockTexture(imageFrame, NULL, &pixels, &pitch) == 0) {
                        memcpy(pixels, camPixel, size);
                        SDL_UnlockTexture(imageFrame);
                }
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
                SDL_RenderCopy(renderer, imageFrame, NULL, NULL);
                SDL_RenderPresent(renderer);

                SDL_Event event;
                while ( SDL_PollEvent( &event ) )
                {
                        if ( event.type == SDL_QUIT )
                                quit = true;
                }
        }

        camReader->StopStream();
        
        camReader->Deinit();
       
        return 0;
}

bool InitEverything()
{
        if ( !InitSDL() )
                return false;

        if ( !CreateWindow() )
                return false;

        if ( !CreateRenderer() )
                return false;

        SetupRenderer();

        return true;
}

bool InitSDL()
{
	if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
	{
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
	//window = SDL_CreateWindow( "Server", posX, posY, sizeX, sizeY, 0 );
	window = SDL_CreateWindow( "Rear Cam", windowRect.x, windowRect.y, windowRect.w, windowRect.h, 0 );

	if ( window == nullptr )
	{
		std::cout << "Failed to create window : " << SDL_GetError();
		return false;
	}

	return true;
}

bool CreateRenderer()
{
	renderer = SDL_CreateRenderer( window, -1, 0 );

	if ( renderer == nullptr )
	{
		std::cout << "Failed to create renderer : " << SDL_GetError();
		return false;
	}

	return true;
}

void SetupRenderer()
{
	// Set size of renderer to the same as window
	SDL_RenderSetLogicalSize( renderer, windowRect.w, windowRect.h );

	// Set color of renderer to red
	SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
}
