/*=============================================================================
author        : Walter Schreppers
filename      : benchmarks/pixeldemo.cpp
created       : 10/5/2022 at 18:21:12
modified      :
version       :
copyright     : Walter Schreppers
description   : Show full screen filled with random pixels and circle drawn
                and some lines. This was optimized so that 60FPS can be done
                with < 10% cpu load.
=============================================================================*/

#include "screen.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <cstring>
#include <iostream>
#include <math.h>
#include <vector>

#define YPOS 200
#define SINE_MAX 50
#define SPEED 0.4
#define SINE_SPEED 0.1

std::string scroll_text = "Pass a command line arg to customize this message!";
int offset = 0;
int sine_offset = 640;
SDL_Renderer *renderer;
int sintable[640];
std::vector<SDL_Rect> letters;

SDL_Texture *loadTexture(const std::string &file, SDL_Renderer *ren) {
  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr) {
    std::cout << "LoadTexture error: " << SDL_GetError() << std::endl;
  }
  return texture;
}

// render texture, specify x,y target width, target height
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w,
                   int h) {
  // Setup the destination rectangle to be at the position we want
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(ren, tex, NULL, &dst);
}

// render texture at pos x,y
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y) {
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  renderTexture(tex, ren, x, y, w, h);
}

// maps coords in bitmapfont so we can chop into textures
void mapFontToLetters() {
  letters.clear();
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      SDL_Rect src;
      src.x = x * 49 + 1;
      src.y = y * 48;
      src.w = 49;
      src.h = 49;
      letters.push_back(src);
    }
  }

  letters[0].x += 5; // tweak A
  letters[0].w -= 2;

  // tweak positions so letters are clipped correctly
  letters[1].x = letters[1].x + 2; // B more to right
  letters[2].x += 2;               // C more to right + narrower
  letters[2].w = 47;

  letters[3].x += 2; // D a bit narrower and offset to center
  letters[3].w -= 2;
  letters[4].w = 47; // E a bit narrower
  letters[5].x -= 3; // F
  letters[5].w -= 4;

  letters[6].x += 5; // G
  letters[6].w -= 2;

  letters[7].x += 2; // H
  letters[8].x += 5; // I
  letters[8].w -= 13;

  letters[10].w -= 2; // K
  letters[11].w -= 3; // L
  letters[12].x += 3; // M
  letters[13].x += 2; // N
  letters[14].x += 1; // O
  letters[15].w -= 5; // P
  letters[16].w -= 1; // Q
  letters[17].x += 1; // R
  letters[17].w -= 3;

  letters[18].x += 3; // S
  letters[19].x += 2; // T
  letters[20].x += 1; // U
  letters[22].x -= 2; // W
  letters[24].y += 4; // Y
  letters[24].x += 3;

  letters[25].x += 3; // Z
  letters[25].y += 4;

  letters[26].x += 1; // 1
  letters[26].y += 2;
  letters[26].w -= 10;

  letters[27].y += 3; // 3
  letters[28].y += 3;

  letters[29].y += 3; // 4
  letters[30].y += 3; // 5
  letters[30].x += 3;

  letters[31].x += 3; // 6
  letters[31].y += 5;

  letters[32].y += 3; // 7
  letters[32].x += 2;

  letters[33].y += 4; // 8
  letters[33].x += 1;
  letters[33].h -= 1;

  letters[34].y += 4; // 9

  letters[35].y += 5; // .
  letters[36].y += 5; // ,
  letters[36].w -= 7;

  letters[37].y += 5; // -
  letters[37].h -= 1;

  letters[38].y += 5; // :
  letters[39].y += 5; // !
  letters[39].h -= 1;

  letters[40].y += 5; // logo
  letters[41].y += 5; // logo

  letters[42].y += 5; // (
  letters[42].x += 5; // (

  letters[43].y += 5; // )
  letters[43].x += 5; // )

  // 44 is space
  letters[45].y += 5; // ?
  letters[46].y += 5; // 0
}

int getLetterPos(char c) {
  // get correct pos for a-z
  int lowcase = (int)c - 97;
  if (lowcase >= 0 && lowcase < 26)
    return lowcase; // a-z -> 0-25

  // get correct pos for A-Z
  int upcase = (int)c - 65;
  if (upcase >= 0 && upcase < 26)
    return upcase; // A-Z -> 0-25

  int lpos = 44; // space as default

  if (c == ' ')
    lpos = 44;
  else if (c == '.')
    lpos = 35;
  else if (c == ',')
    lpos = 36;
  else if (c == '?')
    lpos = 45;
  else if (c == '0')
    lpos = 46;
  else if (c == '1')
    lpos = 26;
  else if (c == '2')
    lpos = 27;
  else if (c == '3')
    lpos = 28;
  else if (c == '4')
    lpos = 29;
  else if (c == '5')
    lpos = 30;
  else if (c == '6')
    lpos = 31;
  else if (c == '7')
    lpos = 32;
  else if (c == '8')
    lpos = 33;
  else if (c == '9')
    lpos = 34;
  else if (c == '-')
    lpos = 37;
  else if (c == ':')
    lpos = 38;
  else if (c == '!')
    lpos = 39;
  else if (c == '(')
    lpos = 42;
  else if (c == ')')
    lpos = 43;
  // else {
  //   std::cout << "warning could not map char '" << c
  //             << "' ascii value=" << (int)c << std::endl;
  // }

  return lpos;
}

void calc_sintable() {
  for (int i = 0; i < 640; i++) {
    sintable[i] = sin((2 * M_PI) / 640 * i) * SINE_MAX;
  }
}

void drawLetter(Screen &scr, SDL_Texture *font, int letter_pos, int x, int y) {
  // sine scroll get slices and map them onto sine y-offset
  const SDL_Rect &src = letters[letter_pos];

  for (int xs = 0; xs < src.w; xs++) {
    if ((x + xs > -5) && (x + xs < 640)) {
      SDL_Rect src_slice;
      src_slice.x = src.x + xs;
      src_slice.y = src.y;
      src_slice.h = src.h;
      src_slice.w = 1;

      SDL_Rect dst_slice;
      dst_slice.x = x + xs;
      int sinpos = ((int)(x + xs) + sine_offset) % 640;
      dst_slice.y = y + sintable[sinpos];
      dst_slice.w = 1;
      dst_slice.h = 48;

      SDL_RenderCopy(renderer, font, &src_slice, &dst_slice);
    }
  }
}

bool drawString(Screen &scr, SDL_Texture *font, const std::string &str,
                int xpos, int ypos) {
  int letter_x = xpos;

  for (unsigned long int i = 0; i < str.size(); i++) {
    int lpos = getLetterPos(str[i]);
    int lwidth = letters[lpos].w + 10;

    if (letter_x + lwidth < 0) {
      letter_x += lwidth;
      if (i == str.size() - 1) {
        return true; // last letter went offscreen on left
      }
      continue;
    }

    if (letter_x > 640) {
      letter_x += lwidth;
      continue;
    }

    drawLetter(scr, font, lpos, letter_x, ypos);
    letter_x += lwidth;
  }

  return false; // we havent scrolled to end yet
}

void drawFrame(Screen &scr, SDL_Texture *font, double deltaTime) {
  offset += SPEED * deltaTime;

  sine_offset -= SINE_SPEED * deltaTime;
  if (sine_offset < 0)
    sine_offset = 640;

  scr.clear();

  bool end_reached = drawString(scr, font, scroll_text, 640 - offset, YPOS);
  if (end_reached)
    offset = 0;

  // draw on screen examples
  // scr.setColor(255, 255, 255);
  // scr.box(0 + offset / 10, 0 + offset / 10, 48, 48);
  // scr.pixel(x, y, 20, 40, 20);
  // scr.draw(false); // dont present yet

  // swap double buffer using renderpresent
  SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
  if (argc > 1)
    scroll_text = argv[1];

  // Screen screen(1440, 900); is my native resolution in ubuntu
  Screen screen(640, 400);
  renderer = screen.getRenderer();
  SDL_RenderSetLogicalSize(renderer, 640, 400);
  screen.setFullscreen(true);

  // The font texture we'll be using
  SDL_Texture *font = loadTexture("./gold_034.png", renderer);
  if (font == nullptr) {
    std::cerr << "cant load image..." << std::endl;
    return 1;
  }

  mapFontToLetters();
  calc_sintable();

  int fW, fH;
  SDL_QueryTexture(font, NULL, NULL, &fW, &fH);

  Uint64 last_frame = 0;
  double deltaTime = 0;
  Uint64 current_frame = SDL_GetPerformanceCounter();

  while (screen.opened()) {
    last_frame = current_frame;
    current_frame = SDL_GetPerformanceCounter();

    deltaTime = (double)((current_frame - last_frame) * 1000 /
                         (double)SDL_GetPerformanceFrequency());
    screen.handle_events();

    drawFrame(screen, font, deltaTime);
    SDL_Delay(10);
  }

  return 0;
}
