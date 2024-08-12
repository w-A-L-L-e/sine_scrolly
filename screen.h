/*=============================================================================
author        : Walter Schreppers
filename      : screen.h
created       : 30/4/2022 at 22:50:23
modified      :
version       :
copyright     : Walter Schreppers
bugreport(log):
=============================================================================*/

#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>
#include <string>

class Screen {

public:
  // constructor & destructor
  //==========================
  Screen(Uint32 width = 640, Uint32 height = 480,
         const std::string &title = "SDL2", bool full_screen = false);
  ~Screen();

  // public members
  //==============
  void handle_events();
  // void clear(Uint32 r=0, Uint32 g=0, Uint32 b=0, Uint32
  // alpha=SDL_ALPHA_OPAQUE);
  void clear(Uint32 r = 0, Uint32 g = 0, Uint32 b = 0, Uint32 alpha = 0);
  void setFullscreen(bool fs);
  void draw(bool present = true);
  SDL_Renderer *getRenderer() { return renderer; }
  SDL_Window *getWindow() { return window; }

  // fast pixel with color and alpha
  void pixel(Uint32 x, Uint32 y, Uint32 red, Uint32 green, Uint32 blue,
             Uint32 alpha = SDL_ALPHA_OPAQUE);

  void setColor(Uint32 red, Uint32 green, Uint32 blue,
                Uint32 alpha = SDL_ALPHA_OPAQUE);
  void pixel(Uint32 x, Uint32 y);
  void circle(Uint32 centreX, Uint32 centreY, Uint32 radius);
  void line(int x0, int y0, int x1, int y1);
  void box(int x, int y, int width, int height);
  void triangle(int x0, int y0, int x1, int y1, int x2, int y2);
  void fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2);

  void printFPS();
  void showRenderInfo();

  bool opened() { return running; }
  bool closed() { return !running; }
  void quit() { running = false; }

  // public locals
  //=============
  SDL_Event event;
  Uint32 width, height, center_x, center_y;
  double x_scale,
      y_scale; // scale factors to use when stretched on fullscreen mode

private:
  // private members
  //===============
  void init();
  int abs(int);

  // private locals
  //==============
  bool fullscreen;
  bool running;
  std::string window_title;

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  unsigned char *pixels;

  Uint32 red;
  Uint32 green;
  Uint32 blue;
  Uint32 alpha;
}; // end of class Screen

#endif
