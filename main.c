#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <SDL.h>

#define SOUND_OPENAL 0
#define SOUND_SDL_QUEUED 1
#define SOUND_SDL_CALLBACK 2

#define WINDOW_TITLE_CAPACITY 256

#define SAMPLE_FILE_PATH "enemy_shoot-48000.wav"

const char *compose_window_title(void)
{
    size_t size = 0;
    char *title = malloc(sizeof(char) * WINDOW_TITLE_CAPACITY);

#ifdef ENABLE_VSYNC
    size += snprintf(title + size, WINDOW_TITLE_CAPACITY, "vsync");
#else
    size += snprintf(title + size, WINDOW_TITLE_CAPACITY, "no vsync");
#endif

#if SOUND == SOUND_OPENAL 
    size += snprintf(title + size, WINDOW_TITLE_CAPACITY, ",openal");
#elif SOUND == SOUND_SDL_QUEUED
    size += snprintf(title + size, WINDOW_TITLE_CAPACITY, ",sdl queued");
#elif SOUND == SOUND_SDL_CALLBACK
    size += snprintf(title + size, WINDOW_TITLE_CAPACITY, ",sdl callback");
#else
    size += snprintf(title + size, WINDOW_TITLE_CAPACITY, ",unknown");
#endif

    return title;
}

#if SOUND == SOUND_SDL_CALLBACK
struct Sample
{
    Uint8 *buf;
    Uint32 len;
    Uint32 cur;
};

void audio_callback(void *userdata, Uint8 *output, int output_len)
{
    struct Sample *sample = userdata;

    Uint32 len = output_len;
    if ((sample->len - sample->cur) < len) {
        len = sample->len - sample->cur;
    }

    memcpy(output, sample->buf + sample->cur, len);

    sample->cur += len;
}
#endif

int main(int argc, char *argv[])
{
#if SOUND == SOUND_OPENAL
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
    alutLoadWAVFile(SAMPLE_FILE_PATH, &format, &data, &size, &freq, &loop);

    alBufferData(buffer, format, data, size, freq);
    alSourcei(source, AL_BUFFER, buffer);

    SDL_Init(SDL_INIT_VIDEO);
#define PLAY_SOUND \
    do { \
        alSourcePlay(source); \
    } while (0)
#elif SOUND == SOUND_SDL_QUEUED
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    Uint8 *audio_buf = NULL;
    Uint32 audio_len = 0;

    SDL_AudioSpec want = {};
    if (SDL_LoadWAV(SAMPLE_FILE_PATH, &want, &audio_buf, &audio_len) == NULL) {
        fprintf(stderr, "SDL pooped itself: Failed to load "SAMPLE_FILE_PATH": %s\n", SDL_GetError());
        exit(1);
    }

    SDL_AudioSpec have = {};
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
            NULL,
            0,
            &want,
            &have,
            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        fprintf(stderr, "SDL pooped itself: Failed to open audio: %s\n", SDL_GetError());
        exit(1);
    }

    if (have.format != want.format) {
        fprintf(stderr, "[WARN] We didn't get expected audio format.\n");
        exit(1);
    }
    SDL_PauseAudioDevice(dev, 0);
#define PLAY_SOUND \
    do { \
        SDL_QueueAudio(dev, audio_buf, audio_len);\
    } while(0)
#elif SOUND == SOUND_SDL_CALLBACK
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    struct Sample sample = {0};

    SDL_AudioSpec want = {};
    if (SDL_LoadWAV(SAMPLE_FILE_PATH, &want, &sample.buf, &sample.len) == NULL) {
        fprintf(stderr, "SDL pooped itself: Failed to load "SAMPLE_FILE_PATH": %s\n", SDL_GetError());
        exit(1);
    }
    want.samples = 256;
    want.callback = audio_callback;
    want.userdata = &sample;

    SDL_AudioSpec have = {};
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
            NULL,
            0,
            &want,
            &have,
            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        fprintf(stderr, "SDL pooped itself: Failed to open audio: %s\n", SDL_GetError());
        exit(1);
    }

    if (have.format != want.format) {
        fprintf(stderr, "[WARN] We didn't get expected audio format.\n");
        exit(1);
    }
    SDL_PauseAudioDevice(dev, 0);
#define PLAY_SOUND \
    do { \
        sample.cur = 0; \
    } while (0)
#else
#error "Provided unknown sound driver"
#endif

    SDL_Window *window =
        SDL_CreateWindow(
            compose_window_title(),
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
                PLAY_SOUND;
            } break;
            }
        }

        if (keyboard[SDL_SCANCODE_E]) {
            if (!pressed) {
                pressed = 1;
                flash = 10;
                PLAY_SOUND;
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

#if SOUND == SOUND_OPENAL
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
#endif

    SDL_Quit();

    return 0;
}
