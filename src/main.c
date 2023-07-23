#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

int main(int argc, char* argv[]){
    if (argc < 2){
        printf("No File to execute specified");
        return -1;
    }
    printf("Executable path: %s\n",argv[argc-1]);
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not be initialized!\n"
               "SDL_Error: %s\n", SDL_GetError());
        return -2;
    }
#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
    {
        printf("SDL can not disable compositor bypass!\n");
        return -3;
    }
#endif
    SDL_Window *window = SDL_CreateWindow("PUCMetaCH8",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("SDL can not create Window\nSDL_Error: %s\n", SDL_GetError());
        return -4;
    }else{
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if(!renderer){
            printf("Renderer could not be created!\n"
                   "SDL_Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            return -5;
        } else{
            SDL_SetWindowMinimumSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_RenderSetIntegerScale(renderer, 10);
            SDL_Surface * surface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE,
            SCREEN_WIDTH, SCREEN_HEIGHT, 1, SDL_PIXELFORMAT_INDEX1MSB);
            SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
            SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
            bool quit = false;
            /*if(state == NULL){
                printf("Couldn't initialize vm state");
                quit = true;
            }*/
            while (!quit) {
                SDL_Event e;
                SDL_WaitEvent(&e);
                if(e.type == SDL_QUIT)
                {
                    quit = true;
                }
                for (size_t i = 0; i < 10; i++)
                {
                    /* code */
                }
                
                SDL_RenderClear(renderer);
                SDL_Texture * screen_texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_DestroyTexture(screen_texture);
            }
            SDL_DestroyRenderer(renderer);
        }
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    return 0;
} 