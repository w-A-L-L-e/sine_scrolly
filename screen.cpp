/*=============================================================================
author        : Walter Schreppers
filename      : screen.cpp
created       : 30/4/2022 at 22:50:23
modified      :
version       :
copyright     : Walter Schreppers
bugreport(log):
=============================================================================*/

#include "screen.h"
#include <iostream>
#include <string>

/*-----------------------------------------------------------------------------
name        : init
description : initialize SDL screen
parameters  :
return      : void
exceptions  :
algorithm   : trivial
-----------------------------------------------------------------------------*/
void Screen::init() {
  SDL_Init(SDL_INIT_EVERYTHING);

  window = SDL_CreateWindow(this->window_title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, width, height,
                            SDL_WINDOW_SHOWN);

  // the VSYNC makes it cap at 60fps (or whatever the screen refresh is) and
  // have smooth animation
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // showRenderInfo(renderer);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, width, height);

  running = true;
  setFullscreen(this->fullscreen);
}

/*-----------------------------------------------------------------------------
name        : Screen
description : constructor
parameters  :
return      :
exceptions  :
algorithm   : trivial
-----------------------------------------------------------------------------*/
Screen::Screen(Uint32 width, Uint32 height, const std::string &title,
               bool full_screen) {
  this->width = width;
  this->height = height;
  this->center_x = width / 2;
  this->center_y = height / 2;
  this->window_title = title;
  this->fullscreen = full_screen;

  window = NULL;
  renderer = NULL;
  texture = NULL;
  pixels = NULL;

  init();
}

/*-----------------------------------------------------------------------------
name        : ~Screen
description : destructor
parameters  :
return      :
exceptions  :
algorithm   : trivial
-----------------------------------------------------------------------------*/
Screen::~Screen() {
  running = false;
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();
}

void Screen::setFullscreen(bool fs) {
  this->fullscreen = fs;
  if (fs) {
    SDL_SetWindowFullscreen(
        window, // SDL_WINDOW_FULLSCREEN); // true mode change
        SDL_WINDOW_FULLSCREEN_DESKTOP); // window stretched on desktop and is
                                        // cross platform generally better way
  } else {
    SDL_SetWindowFullscreen(window, 0);
  }

  // keep track of scaling factors when stretched on full screen mode.
  int screen_w, screen_h;
  SDL_GL_GetDrawableSize(window, &screen_w, &screen_h);
  this->x_scale = (double)screen_w / (double)this->width;
  this->y_scale = (double)screen_h / (double)this->height;
}

void Screen::handle_events() {

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      running = false;
      break;
    case SDL_KEYDOWN: // SDL_KEYUP also exists
      if (event.key.keysym.scancode == SDL_SCANCODE_F) {
        this->fullscreen = !this->fullscreen;
        setFullscreen(this->fullscreen);
      }
      if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
        running = SDL_FALSE;
      }
      break;
    default:
      break;
    }
  }
}

void Screen::printFPS() {
  static Uint32 countedFrames = 0;
  static Uint32 startTick = SDL_GetTicks();
  Uint32 ticks = SDL_GetTicks();
  countedFrames++;
  if (countedFrames % 30 == 0) {
    float avgFPS = (int)countedFrames / ((ticks - startTick) / 1000.f);
    printf("fps %f\n", avgFPS);
  }
  if (countedFrames % 150 == 0) {
    startTick = ticks;
    countedFrames = 0;
  }
}

void Screen::showRenderInfo() {
  SDL_RendererInfo info;
  SDL_GetRendererInfo(renderer, &info);
  std::cout << "Renderer name: " << info.name << std::endl;
  std::cout << "Texture formats: " << std::endl;
  for (Uint32 i = 0; i < info.num_texture_formats; i++) {
    std::cout << SDL_GetPixelFormatName(info.texture_formats[i]) << std::endl;
  }
}

void Screen::clear(Uint32 r, Uint32 g, Uint32 b, Uint32 alpha) {
  // clear screen, is automatically done wiht locking texture
  SDL_SetRenderDrawColor(renderer, r, g, b,
                         alpha); // 0 = transparent, 255=opaque

  if (alpha != 255) {
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  }

  SDL_RenderClear(renderer);

  // lock double buffer texture so we can manipulate lockedPixels directly
  int pitch = 0;
  SDL_LockTexture(texture, NULL, reinterpret_cast<void **>(&pixels), &pitch);

  // as optimization, normally all is zeroed and thus already ready for next
  // frame
  if (r != 0 || g != 0 || b != 0 || alpha != 255) {
    for (unsigned int offset = 0; offset < (width * height * 4) - 4;
         offset += 4) {
      pixels[offset + 0] = b;
      pixels[offset + 1] = g;
      pixels[offset + 2] = r;
      pixels[offset + 3] = alpha;
    }
  }
  // we avoid this here ;)
  // std::memcpy( lockedPixels, pixels.data(), pixels.size() );
}

void Screen::draw(bool present) {
  // unlock and copy to renderer
  SDL_UnlockTexture(texture);
  SDL_RenderCopy(renderer, texture, NULL, NULL);

  if (present) {
    // show renderer on screen
    SDL_RenderPresent(renderer);
  }
}

void Screen::pixel(Uint32 x, Uint32 y, Uint32 red, Uint32 green, Uint32 blue,
                   Uint32 alpha) {
  if (x >= width)
    return;
  if (y >= height)
    return;

  const unsigned int offset = (width * 4 * y) + x * 4;
  pixels[offset + 0] = blue;  // b
  pixels[offset + 1] = green; // g
  pixels[offset + 2] = red;   // r
  pixels[offset + 3] = alpha; // a
}

void Screen::pixel(Uint32 x, Uint32 y) {
  if (x >= width)
    return;
  if (y >= height)
    return;

  const unsigned int offset = (width * 4 * y) + x * 4;
  pixels[offset + 0] = blue;  // b
  pixels[offset + 1] = green; // g
  pixels[offset + 2] = red;   // r
  pixels[offset + 3] = alpha; // a
}

void Screen::setColor(Uint32 red, Uint32 green, Uint32 blue, Uint32 alpha) {
  if (red > 255)
    red = 255;
  if (green > 255)
    green = 255;
  if (blue > 255)
    blue = 255;

  this->red = red;
  this->green = green;
  this->blue = blue;
  this->alpha = alpha;
}

// midpoint circle algorithm
// https://en.wikipedia.org/w/index.php?title=Midpoint_circle_algorithm&oldid=889172082#C_example
void Screen::circle(Uint32 centreX, Uint32 centreY, Uint32 radius) {
  const int32_t diameter = (radius * 2);

  int32_t x = (radius - 1);
  int32_t y = 0;
  int32_t tx = 1;
  int32_t ty = 1;
  int32_t error = (tx - diameter);

  while (x >= y) {
    //  Each of the following renders an octant of the circle
    pixel(centreX + x, centreY - y);
    pixel(centreX + x, centreY + y);
    pixel(centreX - x, centreY - y);
    pixel(centreX - x, centreY + y);
    pixel(centreX + y, centreY - x);
    pixel(centreX + y, centreY + x);
    pixel(centreX - y, centreY - x);
    pixel(centreX - y, centreY + x);

    if (error <= 0) {
      ++y;
      error += ty;
      ty += 2;
    }

    if (error > 0) {
      --x;
      tx += 2;
      error += (tx - diameter);
    }
  }
}

int Screen::abs(int v) {
  if (v < 0)
    return -v;
  return v;
}

// bresenham line
void Screen::line(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2; /* error value e_xy */

  while (true) {
    pixel(x0, y0);
    if (x0 == x1 && y0 == y1)
      break;

    /* e_xy+e_x > 0 */
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }

    /* e_xy+e_y < 0 */
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void Screen::box(int x, int y, int width, int height) {
  line(x, y, x + width, y);
  line(x, y, x, y + height);
  line(x + width, y, x + width, y + height);
  line(x, y + height, x + width, y + height);
}

void Screen::triangle(int x0, int y0, int x1, int y1, int x2, int y2) {
  line(x0, y0, x1, y1);
  line(x1, y1, x2, y2);
  line(x2, y2, x0, y0);
}

// Inspiration
// https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
void Screen::fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2) {

  if (y0 == y1 && y0 == y2)
    return; // skip one line triangles

  // sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!)
  if (y0 > y1) {
    std::swap(y0, y1);
    std::swap(x0, x1);
  }
  if (y0 > y2) {
    std::swap(y0, y2);
    std::swap(x0, x2);
  }
  if (y1 > y2) {
    std::swap(y1, y2);
    std::swap(x1, x2);
  }

  int total_height = y2 - y0;

  for (int i = 0; i < total_height; i++) {
    bool second_half = i > y1 - y0 || y1 == y0;
    int segment_height = second_half ? y2 - y1 : y1 - y0;

    float ma = (float)i / total_height;
    // be careful: with above conditions no division by zero here
    float mb = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;

    int Ax, Bx;
    Ax = x0 + (x2 - x0) * ma;
    // Ay = y0 + (y2-y0)*ma;

    if (second_half) {
      Bx = x1 + (x2 - x1) * mb;
      // By = y1 + (y2-y1)*mb;
    } else {
      Bx = x0 + (x1 - x0) * mb;
      // By = y0 + (y1-y0)*mb;
    }

    if (Ax > Bx) {
      std::swap(Ax, Bx);
    }

    if ((y0 + i) >= (int)height)
      continue;
    if ((y0 + i) < 0)
      continue;

    // This is simpler but slower:
    // for (int j=Ax; j<=Bx; j++) {
    //  pixel(j, y0+i);
    //}

    // Instead: fast draw the horizonal lines without using pixel
    // and only compute offset once
    if (Ax < 0)
      Ax = 0; // clip at left edge screen;

    const unsigned int offset = (width * 4 * (y0 + i)) + Ax * 4;
    for (int j = 0; j < (Bx - Ax); j++) {
      if ((j + Ax) >= (int)width)
        break;
      int xpos = offset + j * 4;

      pixels[xpos + 0] = blue;  // b
      pixels[xpos + 1] = green; // g
      pixels[xpos + 2] = red;   // r
      pixels[xpos + 3] = alpha; // a
    }
  }
}
