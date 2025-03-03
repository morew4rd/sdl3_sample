#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* Constants */
#define windowStartWidth 900
#define windowStartHeight 400

/* Application context structure */
typedef struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* messageTex;
    SDL_Texture* imageTex;
    SDL_FRect messageDest;
    SDL_AudioDeviceID audioDevice;
    Mix_Music* music;
    SDL_AppResult app_quit;
} AppContext;

/* Helper function to handle SDL errors */
SDL_AppResult SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

/* Initialization function */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    /* Initialize SDL with video and audio */
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return SDL_Fail();
    }

    /* Initialize SDL_ttf */
    if (!TTF_Init()) {
        return SDL_Fail();
    }

    /* Create the window */
    SDL_Window* window = SDL_CreateWindow("SDL Minimal Sample",
                                          windowStartWidth,
                                          windowStartHeight,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        return SDL_Fail();
    }

    /* Create the renderer */
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Get the base path for assets */
    const char* basePath = SDL_GetBasePath();
    if (!basePath) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Load the font */
    char fontPath[256];
    snprintf(fontPath, sizeof(fontPath), "%sInter-VariableFont.ttf", basePath);
    TTF_Font* font = TTF_OpenFont(fontPath, 36);
    if (!font) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Render text to a surface */
    const char* text = "Hello SDL!";
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text, strlen(text), (SDL_Color){255, 255, 255});
    if (!surfaceMessage) {
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Create texture from surface */
    SDL_Texture* messageTex = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_DestroySurface(surfaceMessage);
    TTF_CloseFont(font);
    if (!messageTex) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Load the SVG image */
    char svgPath[256];
    snprintf(svgPath, sizeof(svgPath), "%sgs_tiger.svg", basePath);
    SDL_Surface* svg_surface = IMG_Load(svgPath);
    if (!svg_surface) {
        SDL_DestroyTexture(messageTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Create texture from SVG surface */
    SDL_Texture* imageTex = SDL_CreateTextureFromSurface(renderer, svg_surface);
    SDL_DestroySurface(svg_surface);
    if (!imageTex) {
        SDL_DestroyTexture(messageTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    /* Get texture dimensions */
    SDL_PropertiesID messageTexProps = SDL_GetTextureProperties(messageTex);
    float w = (float)SDL_GetNumberProperty(messageTexProps, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
    float h = (float)SDL_GetNumberProperty(messageTexProps, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
    SDL_FRect messageDest;
    messageDest.x = 0;
    messageDest.y = 0;
    messageDest.w = w;
    messageDest.h = h;

    /* Initialize audio */
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!audioDevice) {
        SDL_DestroyTexture(imageTex);
        SDL_DestroyTexture(messageTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    if (!Mix_OpenAudio(audioDevice, NULL)) {
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyTexture(imageTex);
        SDL_DestroyTexture(messageTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }

    // if(!SDL_SetAudioDeviceGain(audioDevice, 0.1)) {
    //     SDL_LogInfo(0, "Audio volume issue");
    // }
    // SDL_LogInfo(0, "Audio volume set :)");

    /* Load and play music */
    char musicPath[256];
    snprintf(musicPath, sizeof(musicPath), "%sthe_entertainer.ogg", basePath);
    Mix_Music* music = Mix_LoadMUS(musicPath);
    Mix_VolumeMusic(10);
    if (!music) {
        Mix_CloseAudio();
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyTexture(imageTex);
        SDL_DestroyTexture(messageTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_Fail();
    }
    Mix_PlayMusic(music, 0);

    /* Show the window and log information */
    SDL_ShowWindow(window);
    int width, height, bbwidth, bbheight;
    SDL_GetWindowSize(window, &width, &height);
    SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
    SDL_Log("Window size: %dx%d", width, height);
    SDL_Log("Backbuffer size: %dx%d", bbwidth, bbheight);
    if (width != bbwidth) {
        SDL_Log("This is a highdpi environment.");
    }

    /* Allocate and initialize the application context */
    struct AppContext* app = (AppContext *)malloc(sizeof(struct AppContext));
    if (!app) {
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        SDL_CloseAudioDevice(audioDevice);
        SDL_DestroyTexture(imageTex);
        SDL_DestroyTexture(messageTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return SDL_APP_FAILURE;
    }

    app->window = window;
    app->renderer = renderer;
    app->messageTex = messageTex;
    app->imageTex = imageTex;
    app->messageDest = messageDest;
    app->audioDevice = audioDevice;
    app->music = music;
    app->app_quit = SDL_APP_CONTINUE;

    *appstate = app;

    /* Enable VSync */
    SDL_SetRenderVSync(renderer, -1);

    SDL_Log("Application started successfully!");
    return SDL_APP_CONTINUE;
}

/* Event handling function */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    struct AppContext* app = (struct AppContext*)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_APP_SUCCESS;
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_F4) {
            app->app_quit = SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

/* Main loop iteration function */
SDL_AppResult SDL_AppIterate(void* appstate) {
    struct AppContext* app = (struct AppContext*)appstate;

    /* Calculate background color based on time */
    float time = (float)SDL_GetTicks() / 1000.0f;
    float red = (sinf(time) + 1.0f) / 2.0f * 255.0f;
    float green = (sinf(time / 2.0f) + 1.0f) / 2.0f * 255.0f;
    float blue = (sinf(time * 2.0f) + 1.0f) / 2.0f * 255.0f;

    /* Clear the screen with the color */
    SDL_SetRenderDrawColor(app->renderer, (Uint8)red, (Uint8)green, (Uint8)blue, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    /* Render the image and text */
    SDL_RenderTexture(app->renderer, app->imageTex, NULL, NULL);
    SDL_RenderTexture(app->renderer, app->messageTex, NULL, &app->messageDest);

    /* Present the rendered frame */
    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

/* Cleanup function */
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    struct AppContext* app = (struct AppContext*)appstate;

    if (app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        Mix_FadeOutMusic(1000); /* Fade out music over 1 second */
        Mix_FreeMusic(app->music); /* Blocks until fade is complete */
        Mix_CloseAudio();
        SDL_CloseAudioDevice(app->audioDevice);
        free(app);
    }

    TTF_Quit();
    Mix_Quit();
    SDL_Log("Application quit successfully!");
    SDL_Quit();
}