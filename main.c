#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <SDL.h>

int main(int argc, char *argv[])
{
    ALCdevice *device = alcOpenDevice(NULL);
    if (!device) {
        fprintf(stderr, "Could not open device for some reason\n");
        exit(1);
    }

    ALCcontext *context = alcCreateContext(device, NULL);
    if (!context) {
        fprintf(stderr, "Could not create the context for some reason\n");
        exit(1);
    }

    if (!alcMakeContextCurrent(context)) {
        fprintf(stderr, "Could not set the current context for some reason\n");
        exit(1);
    }

    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, listenerOri);

    ALuint source;
    alGenSources((ALuint)1, &source);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    ALuint buffer;
    alGenBuffers((ALuint)1, &buffer);

    ALsizei size, freq;
    ALenum format;
    ALvoid *data;
    ALboolean loop = AL_FALSE;
    alutLoadWAVFile("enemy_shoot-48000.wav", &format, &data, &size, &freq, &loop);

    alBufferData(buffer, format, data, size, freq);
    alSourcei(source, AL_BUFFER, buffer);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window =
        SDL_CreateWindow(
#ifdef ENABLE_VSYNC
            "Sound probe (vsync)",
#else
            "Sound probe (no vsync)",
#endif
            0, 0, 800, 600,
            SDL_WINDOW_RESIZABLE);

    SDL_Renderer *renderer =
        SDL_CreateRenderer(
            window, -1,
#ifdef ENABLE_VSYNC
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
#else
            SDL_RENDERER_ACCELERATED 
#endif
            );

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    int pressed = 0;
    int flash = 0;
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;

            case SDL_MOUSEBUTTONDOWN: {
                flash = 10;
                alSourcePlay(source);
            } break;
            }
        }

        if (keyboard[SDL_SCANCODE_E]) {
            if (!pressed) {
                pressed = 1;
                flash = 10;
                alSourcePlay(source);
            }
        } else {
            pressed = 0;
        }

        if (flash) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            flash -= 1;
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}
