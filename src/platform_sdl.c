// The platform on SDL2 (zlib license, not part of this repo: tools/fetch_sdl2.py).
#include "platform.h"

#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

enum {
  kPictureWidth = 256,
  kPictureHeight = 240,
  kStickDeadZone = 16000,
  kPadA = 0x01, kPadB = 0x02, kPadSelect = 0x04, kPadStart = 0x08,
  kPadUp = 0x10, kPadDown = 0x20, kPadLeft = 0x40, kPadRight = 0x80,
  kAudioChunk = 512,      // samples the sound card takes at a time
  kAudioTargetFrames = 3, // how much to keep waiting: enough not to run dry, little enough to feel prompt
  kAudioMaxFrames = 10,   // more than this and it has fallen behind (a stall): start over
};

// The NES's NTSC frame rate: 1,789,772.7 CPU cycles a second, 29,780.5 cycles a frame.
static const double kFramesPerSecond = 60.0988;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static SDL_GameController *controller;
static Uint64 next_frame;
static SDL_AudioDeviceID audio;
static unsigned audio_rate;
static int audio_started;

int platform_open(int scale) {
  SDL_SetMainReady();
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER)) {
    fprintf(stderr, "dejavu: can't start SDL: %s\n", SDL_GetError());
    return -1;
  }
  window = SDL_CreateWindow("Deja Vu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            kPictureWidth * scale, kPictureHeight * scale, SDL_WINDOW_RESIZABLE);
  if(!window) {
    fprintf(stderr, "dejavu: can't open a window: %s\n", SDL_GetError());
    return -1;
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  if(!renderer) {
    fprintf(stderr, "dejavu: can't draw in the window: %s\n", SDL_GetError());
    return -1;
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // sharp pixels
  SDL_RenderSetLogicalSize(renderer, kPictureWidth, kPictureHeight);
  SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                              kPictureWidth, kPictureHeight);
  if(!texture) {
    fprintf(stderr, "dejavu: can't make the picture's texture: %s\n", SDL_GetError());
    return -1;
  }
  next_frame = SDL_GetPerformanceCounter();
  return 0;
}

void platform_open_audio(unsigned sample_rate) {
  SDL_AudioSpec want = {0};
  want.freq = (int)sample_rate;
  want.format = AUDIO_S16SYS;
  want.channels = 1;
  want.samples = kAudioChunk;
  if(SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    fprintf(stderr, "dejavu: no sound: %s\n", SDL_GetError());
    return;
  }
  audio = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);  // SDL converts if the card wants otherwise
  if(!audio) {
    fprintf(stderr, "dejavu: no sound: %s\n", SDL_GetError());
    return;
  }
  audio_rate = sample_rate;
}

// Samples waiting, in the audio's units.
static unsigned audio_waiting(void) {
  return SDL_GetQueuedAudioSize(audio) / sizeof(int16_t);
}

void platform_queue_audio(const int16_t *samples, size_t count) {
  if(!audio) {
    return;
  }
  unsigned frame = audio_rate / 60;
  if(audio_waiting() > frame * kAudioMaxFrames) {
    SDL_ClearQueuedAudio(audio);
    SDL_PauseAudioDevice(audio, 1);
    audio_started = 0;
  }
  SDL_QueueAudio(audio, samples, (Uint32)(count * sizeof(int16_t)));
  if(!audio_started && audio_waiting() >= frame * kAudioTargetFrames) {
    SDL_PauseAudioDevice(audio, 0);  // it starts paused, until there's enough not to run dry at once
    audio_started = 1;
  }
}

void platform_show_error(const char *message) {
#if defined(NDEBUG) && defined(_WIN32)
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Deja Vu", message, NULL);
#else
  (void)message;
#endif
}

static uint8_t read_buttons(void) {
  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  uint8_t pad = 0;
  if(keys[SDL_SCANCODE_X]) pad |= kPadA;
  if(keys[SDL_SCANCODE_Z]) pad |= kPadB;
  if(keys[SDL_SCANCODE_RSHIFT]) pad |= kPadSelect;
  if(keys[SDL_SCANCODE_RETURN]) pad |= kPadStart;
  if(keys[SDL_SCANCODE_UP]) pad |= kPadUp;
  if(keys[SDL_SCANCODE_DOWN]) pad |= kPadDown;
  if(keys[SDL_SCANCODE_LEFT]) pad |= kPadLeft;
  if(keys[SDL_SCANCODE_RIGHT]) pad |= kPadRight;
  if(controller) {
    // By position: the NES's A is on the right and its B to the left of it.
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) pad |= kPadA;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) pad |= kPadB;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK)) pad |= kPadSelect;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) pad |= kPadStart;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) pad |= kPadUp;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) pad |= kPadDown;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) pad |= kPadLeft;
    if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) pad |= kPadRight;
    int x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    int y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
    if(y < -kStickDeadZone) pad |= kPadUp;
    if(y > kStickDeadZone) pad |= kPadDown;
    if(x < -kStickDeadZone) pad |= kPadLeft;
    if(x > kStickDeadZone) pad |= kPadRight;
  }
  return pad;
}

static void write_screenshot(const char *path) {
  int width, height;
  SDL_GetRendererOutputSize(renderer, &width, &height);
  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
  if(surface && !SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels,
                                      surface->pitch)) {
    SDL_SaveBMP(surface, path);
  }
  SDL_FreeSurface(surface);
}

int platform_frame(const uint32_t *rgb, uint8_t *pad, const char *screenshot) {
  int running = 1;
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    if(event.type == SDL_QUIT) {
      running = 0;
    } else if(event.type == SDL_CONTROLLERDEVICEADDED && !controller) {
      controller = SDL_GameControllerOpen(event.cdevice.which);
    } else if(event.type == SDL_CONTROLLERDEVICEREMOVED && controller &&
              event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
      SDL_GameControllerClose(controller);
      controller = NULL;
    }
  }
  *pad = read_buttons();

  SDL_UpdateTexture(texture, NULL, rgb, kPictureWidth * (int)sizeof(rgb[0]));
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  if(screenshot) {
    write_screenshot(screenshot);
  }
  SDL_RenderPresent(renderer);

  // To the frame's time: most of the way asleep, the rest watching the clock.
  Uint64 frequency = SDL_GetPerformanceFrequency();
  double frame_time = (double)frequency / kFramesPerSecond;
  // The sound card's clock and this one drift apart. Leaning the frame time by up to 1% keeps the
  // sound's queue about where it should be, without changing its pitch or any sample.
  if(audio_started) {
    double target = (double)audio_rate / kFramesPerSecond * kAudioTargetFrames;
    double error = ((double)audio_waiting() - target) / target;
    frame_time *= 1.0 + (error > 1 ? 1 : error < -1 ? -1 : error) * 0.01;
  }
  next_frame += (Uint64)frame_time;
  Uint64 now = SDL_GetPerformanceCounter();
  if(now > next_frame + frequency / 4) {
    next_frame = now;  // far behind (the window was being dragged, say): don't rush to catch up
  }
  while(now < next_frame) {
    Uint64 ms = (next_frame - now) * 1000 / frequency;
    if(ms > 2) {
      SDL_Delay((Uint32)(ms - 2));
    }
    now = SDL_GetPerformanceCounter();
  }
  return running;
}

void platform_close(void) {
  if(audio) {
    SDL_CloseAudioDevice(audio);
  }
  if(controller) {
    SDL_GameControllerClose(controller);
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
