/* File: main-sdl.c */
/* Purpose: SDL2 frontend for mangband */
/* Original SDL2 written by "kts of kettek (kettek1@kettek.net)". */

#include "c-angband.h"

#ifdef USE_SDL2
#include "main-sdl2.h"
#ifdef WINDOWS
#define snprintf _snprintf
double fmin(double x, double y) {
  return (x < y ? x : y);
}
#endif
/* ==== Global data ==== */
char *term_title[8] = {             // Our terminal titles
  "MAngband",
  "Character Sheet",
  "Equipment",
  "Inventory",
  "Chat",
  "Term-5",
  "Term-6"
};
static struct TermData terms[8];    // Our terminals
static struct FontData fonts[8];    // Our fonts, tied to the term limit of 7
static struct PictData picts[8];    // Our picts, ^
char default_font[128];
int default_font_size = 16;
#define CONF_TERM_WINDOWED (1 << 0)
#define CONF_TERM_VIRTUAL (1 << 1)
int conf = CONF_TERM_WINDOWED;
/* ==== Initialize function ==== */
/* init_sdl2
Our initializer function. Sets up SDL2 and creates our z-terms (windows).
*/
errr init_sdl2(void) {
  char buf[1024];
  int i;
#ifdef WINDOWS
  // Initialize WinSock
  WSADATA wsadata;
  WSAStartup(MAKEWORD(1, 1), &wsadata);
#endif
#ifdef __APPLE__
  // Set our "cwd" to the directory the application bundle is in on OS X
  char path[PATH_MAX];
  CFURLRef res = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
  CFURLGetFileSystemRepresentation(res, TRUE, (UInt8 *)path, PATH_MAX);
  CFRelease(res);
  chdir(path);
#endif
  init_stuff(); // load in paths
  // **** Load in Configuration ****
  // The following global vars are set AFTER init_sdl2(), but as per below, we need 'em
  use_graphics = TRUE;
  ANGBAND_SYS = "sdl2";
  // FIXME: this should be handled elsewhere, but we want colors now.
  sprintf(buf, "font-%s.prf", ANGBAND_SYS);
  process_pref_file(buf);
  // **** Initialize SDL libraries ****
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Init()", SDL_GetError(), NULL);
    return 1;
  }
  if (IMG_Init(IMG_INIT_PNG) == -1) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "IMG_Init()", IMG_GetError(), NULL);
    return 2;
  }
  if (TTF_Init() == -1) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "TTF_Init()", TTF_GetError(), NULL);
    return 3;
  }
  SDL_StartTextInput(); // This may be better than massive keymaps, but not sure.
  // **** Load Preferences ****
  memset(terms, 0, sizeof(TermData)*7); // FIXME: 0 is not guaranteed to be NULL, use a "clearTermData" func
  loadConfig();
  // **** Merge command-line ****
  // **** Load Fonts and Picts ****
  memset(fonts, 0, 3*sizeof(struct FontData));
  if (ttfToFont(&fonts[FONT_NORMAL], "font/AnonymousPro.ttf", 12, TRUE) != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ttfToFont()", "error while loading", NULL);
  }
  if (ttfToFont(&fonts[FONT_SMALL], "font/AnonymousPro.ttf", 10, TRUE) != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ttfToFont()", "error while loading", NULL);
  }
  memset(picts, 0, 3*sizeof(struct PictData));
  if (imgToPict(&picts[PICT_NORMAL], "graf/16x16.png") != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "imgToPict()", "error while loading", NULL);
  }
  // **** Initialize z-terms ****
  initTermData(&terms[TERM_MAIN], term_title[TERM_MAIN], TERM_MAIN, NULL);
  attachFont(&fonts[FONT_NORMAL], &terms[TERM_MAIN]);
  attachPict(&picts[PICT_NORMAL], &terms[TERM_MAIN]);
  setTermCells(&terms[TERM_MAIN], picts[PICT_NORMAL].w, picts[PICT_NORMAL].h);
  setTermTitle(&terms[TERM_MAIN]);
  refreshTerm(&terms[TERM_MAIN]);
  for (i = 1; i < 7; i++) { // FIXME: this was "i < 8"
    if (terms[i].config & TERM_IS_HIDDEN) continue;
    initTermData(&terms[i], term_title[i], i, NULL);
    attachFont(&fonts[FONT_SMALL], &terms[i]);
    setTermCells(&terms[i], fonts[FONT_SMALL].w, fonts[FONT_SMALL].h);
    setTermTitle(&terms[i]);
    refreshTerm(&terms[i]);
    ang_term[i] = &(terms[i].t);
  }
  // **** Activate Main z-term and gooo ****
  Term_activate(&(terms[TERM_MAIN].t));	// set active Term to terms[TERM_MAIN]
  term_screen = Term;                   // set term_screen to terms[TERM_MAIN]

  return 0;
}
/* ==== Term related functions ==== */
static errr initTermData(TermData *td, cptr name, int id, cptr font) {
  int width, height, key_queue;
  term *t = &td->t;
  /*td->config = 0;
  td->font_data = NULL;
  td->pict_data = NULL;
  td->font_texture = td->pict_texture = NULL;
  td->window = NULL;
  td->window_id = 0;
  td->id = id;
  td->renderer = NULL;
  td->width = td->height = 0;
  td->framebuffer = NULL;*/

  td->id = id;

  if (td->cell_w < 1) td->cell_w = 8;
  if (td->cell_h < 1) td->cell_h = 16;
  if (td->rows < 1) td->rows = 80;
  if (td->cols < 1) td->cols = 24;

  width = td->cell_w*td->rows, height = td->cell_h*td->cols, key_queue = 1024; // assume key queue of 1024 atm

  // If this a virtual term, we assume that TERM_MAIN has already been fully initialized and use its window and renderer
  if (td->config & TERM_IS_VIRTUAL && td->id != 0) {
    td->window = terms[TERM_MAIN].window;
    td->renderer = terms[TERM_MAIN].renderer;
    td->ren_rect.x = 32*id; td->ren_rect.y = 32*id; // TODO: read position from input file
    td->ren_rect.w = width; td->ren_rect.h = height;
  } else {
    /* Initialize SDL2 window */
    if ((td->window = SDL_CreateWindow(td->title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      width, height, SDL_WINDOW_RESIZABLE)) == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create Window",
          SDL_GetError(), NULL);
        return 1;
    }
    td->window_id = SDL_GetWindowID(td->window);
    if ((td->renderer = SDL_CreateRenderer(td->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE)) == NULL) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create Renderer",
        SDL_GetError(), NULL);
      return 1;
    }
  }
  // Create our framebuffer via refreshing
  refreshTerm(td);

  /* Initialize Z-term stuff */
  term_init(t, td->rows, td->cols, key_queue);
  /* Use a "soft" cursor */
  t->soft_cursor = TRUE;
  t->never_bored = TRUE; // dunno
  /* Erase with "white space" */
  t->attr_blank = TERM_WHITE;
  t->char_blank = ' ';

  t->always_text = FALSE;	// TODO: remove me?
  /* Hooks */
  t->init_hook = initTermHook;
  t->nuke_hook = nukeTermHook;
  t->xtra_hook = xtraTermHook;
  t->curs_hook = cursTermHook;
  t->wipe_hook = wipeTermHook;
  t->text_hook = textTermHook;
  t->pict_hook = pictTermHook;
  t->data = (vptr)(td);		// point our z-term to TermData
  Term_activate(t);
  td->config |= TERM_IS_ONLINE;
  return 0;
}
static errr setTermCells(TermData *td, int w, int h) {
  if (w <= 0 || h <= 0) return 1;
  td->cell_w = w;
  td->cell_h = h;
  // set our window to our cell size FIXME
  if (td->config & TERM_IS_VIRTUAL && td->id != 0) {
    td->ren_rect.w = w; td->ren_rect.h = h;
  } else {
    SDL_SetWindowSize(td->window, w*td->rows, h*td->cols);
  }
  return 0;
}
static errr refreshTerm(TermData *td) {
  int w, h;
  if (td->config & TERM_IS_VIRTUAL && td->id != 0) {
    w = td->ren_rect.w; h= td->ren_rect.h;
  } else {
    SDL_GetWindowSize(td->window, &w, &h);
  }
  if (td->width == w && td->height == h) return 0;
  if (td->framebuffer) SDL_DestroyTexture(td->framebuffer);

  if ((td->framebuffer = SDL_CreateTexture(td->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h)) == NULL) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "refreshTerm", SDL_GetError(), td->window);
    return 1;
  }
  td->width = w;
  td->height = h;
  // refresh the renderer to match the new window dimensions
  SDL_RenderSetViewport(td->renderer, NULL);
  // set the draw rect to the window dimensions
  td->ren_rect.w = w;
  td->ren_rect.h = h;
  // wipe the new framebuffer
  SDL_SetRenderTarget(td->renderer, td->framebuffer);
  SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 255);
  SDL_RenderClear(td->renderer);

  return 0;
}
static errr resizeTerm(TermData *td, int rows, int cols) {
  term *old_td;
  if (rows <= 0 || cols <= 0) return 1;
  if (rows == td->rows && cols == td->cols) return 0;

  td->rows = rows;
  td->cols = cols;

  if (td->config & TERM_IS_VIRTUAL && td->id != 0) {
    td->ren_rect.w = td->cell_w*rows; td->ren_rect.h = td->cell_h*cols;
  } else {
    SDL_SetWindowSize(td->window, td->cell_w*rows, td->cell_h*cols);
  }
  setTermTitle(td);

  // store current term, activate this, resize, and restore previous
  old_td = Term;
  Term_activate(&(td->t));
  Term_resize(rows, cols);
  Term_activate(old_td);

  return 0;
}
static errr setTermTitle(TermData *td) {
  char buf[1024];
  sprintf(buf, "%s (%dx%d)", td->title, td->rows, td->cols);
  SDL_SetWindowTitle(td->window, buf);
  return 0;
}
/* attachFont
This function creates an SDL_Texture from the given FontData's surface, then sets
needed Term options.
*/
errr attachFont(FontData *fd, TermData *td) {
  if ((td->font_texture = SDL_CreateTextureFromSurface(td->renderer, fd->surface)) == NULL) {
    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "attachFont Error", SDL_GetError(), td->window);
    printf("Error: attachFont(): %s\n", SDL_GetError());
    return 1;
  }
  SDL_SetTextureBlendMode(td->font_texture, SDL_BLENDMODE_BLEND);
  td->font_data = fd;
  return 0;
}
/* detachFont
This function destroys the SDL_Texture created by attachFont
*/
errr detachFont(TermData *td) {
  if (td->font_texture) SDL_DestroyTexture(td->font_texture);
  td->font_texture = NULL;
  td->font_data = NULL;
  return 0;
}
/* attachPict
This function creates an SDL_Texture from the given PictData's surface. It then sets required
*/
errr attachPict(PictData *pd, TermData *td) {
  if ((td->pict_texture = SDL_CreateTextureFromSurface(td->renderer, pd->surface)) == NULL) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "attachFont Error", SDL_GetError(), td->window);
    return 1;
  }
  td->pict_data = pd;
  td->t.higher_pict = TRUE; // enable "pict" graphics
  return 0;
}
/* detachPict
This function "detaches' the given term's pict data - in effect, it destroys the SDL_Texture
and deactivates pict-related settings
*/
errr detachPict(TermData *td) {
  if (td->pict_texture) SDL_DestroyTexture(td->pict_texture);
  td->pict_texture = NULL;
  td->pict_data = NULL;
  td->t.higher_pict = FALSE;
  return 0;
}
/* ==== z-term hooks ==== */
static void initTermHook(term *t) {}
static void nukeTermHook(term *t) {
  TermData *td = (TermData*)(t->data);
  // detach (free texture and NULL pointers) to font/pict if attached
  detachFont(td);
  detachPict(td);
  // destroy self
  if (td->config & TERM_IS_VIRTUAL && td->id != 0) {
  } else{
    SDL_DestroyRenderer(td->renderer);
    SDL_DestroyWindow(td->window);
  }
  td->config &= ~TERM_IS_ONLINE;
  // FIXME: nasty hack to clean up -- isn't there some sort of global quit hook? :S
  if (td->id == TERM_MAIN) {
    // Make sure our other windows are destroyed as well
    int i;
    for (i = 1; i < 7, terms[i].config & TERM_IS_ONLINE; i++) {
      term_nuke(&(terms[i].t));
    }
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Quit", "Adios, wish I could provide a quit message", NULL);
    // clean up our fonts and picts
    // close our libraries
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    //quit(NULL);
  }
}
static errr xtraTermHook(int n, int v) {
  term *old_td;
  TermData *td = (TermData*)(Term->data);
  SDL_Event event;
  switch (n) {
  case TERM_XTRA_NOISE: // generic noise
    //return (Term_xtra_win_noise());
    return 0;
  case TERM_XTRA_SOUND:
    return 0; // send "v"
  case TERM_XTRA_BORED:
    return 0; // send "0" to Event processing
  case TERM_XTRA_LEVEL: // v of 0 = deactivate, v of 1 = activate
    return 0;
  case TERM_XTRA_EVENT:
    do {
      // MMM BEGIN
      if (menu_mode > MENU_HIDE) renderMenu(td); // nom nom nom
      // MMM END
      if (v) {
        if (!SDL_WaitEvent(&event)) return(0);
        v = 0;
      } else {
        if(!SDL_PollEvent(&event)) return(0);
      }
      if (event.type == SDL_QUIT) {
        Term_keypress(KTRL('x'));
        //quit("Goodbye.");
      } else if (event.type == SDL_TEXTINPUT) {
        // this sends off "printable" characters (shifted variants as well)
        char *c = event.text.text;
        while (*c) Term_keypress(*c++);
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.state == SDL_PRESSED) {
          if (event.key.keysym.sym < 33) {
            // Send low-level ASCII char codes (backspace, delete, etc.)
            Term_keypress(event.key.keysym.sym);
          } else {
            unsigned int key = event.key.keysym.sym;
            key &= ~(1 << 30);
            if (event.key.keysym.sym >= 1073741881) {
              // send off odd keys (arrows, keypad, function keys, etc.) as macros
              char buf[32];
              snprintf(buf, 32, "%c%s%s%s%s_%lX%c", 31,
                event.key.keysym.mod & KMOD_CTRL  ? "N" : "",
                event.key.keysym.mod & KMOD_SHIFT ? "S" : "",
                "",
                event.key.keysym.mod & KMOD_ALT   ? "M" : "",
                (unsigned long) key, 13);
              {
                char *c = buf;
                while (*c) Term_keypress(*c++);
              }
            } else {
              // handle "ctrl" keymod of regular keys since TEXTINPUT does not handle them
              if (event.key.keysym.mod & KMOD_CTRL) Term_keypress(KTRL(key));
            }
          }
        }
      } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
          int i;
          for (i = 0; i < 8; i++) {
            if (terms[i].window_id == event.window.windowID) {
              Term_activate(&(terms[i].t));
              break;
            }
          }
        } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          int i;
          for (i = 0; i < 8; i++) {
            if (terms[i].window_id == event.window.windowID) {
              // resize to nearest whole cell
              int w = event.window.data1;
              int h = event.window.data2;
              int cw = w/terms[i].cell_w;
              int ch = h/terms[i].cell_h;

              resizeTerm(&terms[i], cw, ch);
              refreshTerm(&terms[i]);

              // store current term, activate this, refresh, and restore previous
              old_td = Term;
              Term_activate(&(terms[i].t));
              Term_redraw();
              Term_activate(old_td);
              break;
            }
          }
        } else if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
          int i;
          for (i = 0; i < 8; i++) {
            if (terms[i].window_id == event.window.windowID) {
              term *old_td = Term;
              Term_activate(&(terms[i].t));
              Term_redraw();
              Term_activate(old_td);
              break;
            }
          }
        } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
          int i;
          for (i = 0; i < 8; i++) {
            if (terms[i].window_id == event.window.windowID) {
              term_nuke(&(terms[i].t));
              ang_term[i] = NULL;
            }
          }
        }
      }
      // MMM BEGIN
      else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
          switch (menu_mode) {
          case MENU_HIDE:
            menu_mode = MENU_MAIN;
            break;
          case MENU_WINDOWS:
          case MENU_PROPERTIES:
          case MENU_BINDS:
            menu_mode = MENU_MAIN;
            break;
          case MENU_MAIN:
            menu_mode = MENU_HIDE;
            Term_keypress(KTRL('R'));
            break;
          }
        } else if (event.button.button = SDL_BUTTON_LEFT) {
          if (menu_mode != MENU_HIDE) {
            int i;
            for (i = 0; i < 8; i++) {
              if (terms[i].window_id == event.window.windowID) {
                int wx = event.button.x;
                int wy = event.button.y;
                int cx = wx/(terms[i].cell_w ? terms[i].cell_w : 1);
                int cy = wy/(terms[i].cell_h ? terms[i].cell_h : 1);
                handleMenu(cx, cy);
                break;
              }
            }
          }
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        int i;
        for (i = 0; i < 8; i++) {
          if (terms[i].window_id == event.window.windowID) {
            menu_x = event.motion.x / terms[i].cell_w;
            menu_y = event.motion.y / terms[i].cell_h;
            break;
          }
        }
      }
      // MMM END
    } while (SDL_PollEvent(NULL));
    return 0; // okay, I guess?
  case TERM_XTRA_FLUSH:
    while (SDL_PollEvent(&event));
    return 0; // force a redraw?
  case TERM_XTRA_FRESH:
    SDL_SetRenderTarget(td->renderer, NULL);
    SDL_RenderCopy(td->renderer, td->framebuffer, NULL, &td->ren_rect);
    SDL_RenderPresent(td->renderer);
    SDL_SetRenderTarget(td->renderer, td->framebuffer);
    return 0;
  case TERM_XTRA_CLEAR:
    SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 255);
    SDL_RenderClear(td->renderer);
    return 0; // clear?
  case TERM_XTRA_REACT:
    SDL_SetRenderTarget(td->renderer, NULL);
    SDL_RenderCopy(td->renderer, td->framebuffer, NULL, &td->ren_rect);
    SDL_RenderPresent(td->renderer);
    SDL_SetRenderTarget(td->renderer, td->framebuffer);
    return 0; // react?
  case TERM_XTRA_FROSH: // TODO: refresh row "v"
    return 0;
  case TERM_XTRA_DELAY:
    SDL_Delay(v);
    return 0;
  }
  return 1;
}
static errr cursTermHook(int x, int y) {
  TermData *td = (TermData*)(Term->data);
  SDL_Rect cell_rect = { x*td->cell_w, y*td->cell_h, td->cell_w, td->cell_h};
  SDL_SetRenderDrawColor(td->renderer, 128, 255, 64, 255);
  SDL_RenderDrawRect(td->renderer, &cell_rect);
  return 0;
}
static errr wipeTermHook(int x, int y, int n) {
  TermData *td = (TermData*)(Term->data);
  SDL_Rect cell_rect = { x*td->cell_w, y*td->cell_h, td->cell_w*n, td->cell_h };
  SDL_SetRenderDrawColor(td->renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(td->renderer, &cell_rect);
  return 0;
}
static errr textTermHook(int x, int y, int n, byte attr, cptr s) {
  int w, h, offsetx, offsety, i;
  float r;
  TermData *td = (TermData*)(Term->data);
  struct FontData *fd = td->font_data;
  SDL_SetTextureColorMod(td->font_texture, color_table[attr][1], color_table[attr][2], color_table[attr][3]);
  // If the cell width/height of the term does not match, center ourself within it
  // TODO: also make "stretch" and "scale" available!
  //int offsetx = 0, offsety = 0;
  /*int w = terms[cur_term].cell_w, h = terms[cur_term].cell_h;
  if (w > terms[cur_term].font_data->w) {
  offsetx = (w - terms[cur_term].font_data->w) / 2;
  }
  if (h > terms[cur_term].font_data->h) {
  offsety = (h - terms[cur_term].font_data->h) / 2;
  }*/
  w = td->font_data->w;
  h = td->font_data->h;
  r = fmin((float)td->cell_w / (float)w, (float)td->cell_h / (float)h);
  if (r < 1.0) {
    w *= r;
    h *= r;
  }

  offsetx = (td->cell_w / 2) - (w/2);
  offsety = (td->cell_h / 2) - (h/2);

  //int offsetx = td->cell_w;
  //int offsety = td->cell_h;
  //int w = fd->w;
  //int h = fd->h;

  // let's wipe the region using the hook due to potential glyph vs. cell dimensions
  wipeTermHook(x, y, n);

  for (i = 0; i < n; i++) {
    SDL_Rect cell_rect = { (x+i)*td->cell_w+offsetx, y*td->cell_h+offsety, w, h };
    int si = s[i];
    int row = si / 16;
    int col = si - (row*16);
    SDL_Rect char_rect = { col*fd->w, row*fd->h, fd->w, fd->h };
    SDL_RenderCopy(td->renderer, td->font_texture, &char_rect, &cell_rect);
  }
  return 0;
}
static errr pictTermHook(int x, int y, byte attr, char ch) {
  SDL_Rect cell_rect, sprite_rect;
  char row, col;
  TermData *td = (TermData*)(Term->data);
  if (td->font_data == NULL || td->pict_data == NULL) return 1;
  row = attr;
  col = ch;
  row &= ~(1 << 7);
  col &= ~(1 << 7);
  printf("pict at %dx%d(%dx%d)\n", row, col, attr, ch);
  SDL_SetRenderDrawColor(td->renderer, 0, 0, 128, 255);
  cell_rect.x = x*(td->cell_w); cell_rect.y = y*(td->cell_h);
  cell_rect.w = td->cell_w; cell_rect.h = td->cell_h;
  sprite_rect.x = col*(td->pict_data->w); sprite_rect.y = row*(td->pict_data->h);
  sprite_rect.w = td->pict_data->w; sprite_rect.h = td->pict_data->h;
  SDL_RenderDrawRect(td->renderer, &cell_rect);

  SDL_RenderCopy(td->renderer, td->pict_texture, &sprite_rect, &cell_rect);

  return 0;
}
// MMM BEGIN
/* ==== Magical Mangband Menu ==== */
// NOTE: These Menus should be scrapped, but I wanted a quick, cross-platform method to access
// and modify configurations for the SDL2 client. It is mostly an ugly growth that sits on top
// of the regular system. I have kept all of the menu-related code separated down here so that
// removal could be done with ease. To remove, search for "MMM" and remove the relevant code. :)
// (I am thankful that of all the things in this file, I consider this the only "real" hack)
int menu_mode = MENU_HIDE;	// Our extra "SDL2" menu mode
int menu_x, menu_y;			// our menu x and y coordinates
char *menu[][12] = {
  {""},
  {
    "==== GUI Configuration Menu ====",
      "Use the mouse to click on an item, right-click to close menu",
      "1. Open/Close windows",
      "2. Modify windows properties",
      "3. Change key bindings"
  },
  {
    "==== Windows List ====",
      "Select a window to open or close"
    },
    {
      "==== Windows properties ====",
        "Select a field to modify a window's properties",
        "1. Save windows' positions",
        "2. Reload windows' positions"
    },
    {
      "==== Keybindings ====",
        "Click on an appropriate field to change its keybinding, click again to save"
      },
};
/* renderMenu
This function will render the appropriate state if menu_mode is not set to hidden.

The menu system itself is used to configure the SDL2 interface and set up windows
*/
errr renderMenu(TermData *td) {
  term *old_td;
  SDL_Rect menu_rect;
  int width, height, i, y;
  td = &(terms[0]);
  //TermData *td = (TermData*)(Term->data);
  old_td = Term;
  Term_activate(&(terms[0].t));
  width = td->cell_w * (td->rows-8);	// menu width
  height = td->cell_h * (td->cols-2);	// menu width
  menu_rect.x = td->cell_w*4;
  menu_rect.y = td->cell_h;
  menu_rect.w = width;
  menu_rect.h = height;
  // draw background
  SDL_SetRenderDrawColor(td->renderer, 8, 16, 32, 255);
  SDL_RenderFillRect(td->renderer, &menu_rect);
  // draw border
  SDL_SetRenderDrawColor(td->renderer, 32, 64, 128, 255);
  SDL_RenderDrawRect(td->renderer, &menu_rect);
  y = 2;
  // draw title and info line
  for (i = 0; i < 2; i++) {
    int w = strlen(menu[menu_mode][i]);
    textTermHook( td->rows/2 - w/2, y++, w, 4+i, menu[menu_mode][i]);
  }
  y++;
  if (menu_mode == MENU_MAIN) {
    for (; i < 5; i++,y++) {
      textTermHook(5, y, strlen(menu[menu_mode][i]), (menu_y == y ? 3 : 1), menu[menu_mode][i]);
    }
  } else if (menu_mode == MENU_WINDOWS) {
    for (i = 1; i < 7; i++,y++) {
      char buf[128];
      snprintf(buf, 128, "%d. %s Window: %s",
        i,
        (terms[i].config & TERM_IS_ONLINE ? "Close" : "Open"),
        term_title[i]);
      textTermHook(5, y, strlen(buf), (menu_y == y ? 3 : 1), buf);
    }
  } else if (menu_mode == MENU_PROPERTIES) {
    for (; i < 4; i++, y++) {
      textTermHook(5, y, strlen(menu[menu_mode][i]), (menu_y == y ? 3 : 1), menu[menu_mode][i]);
    }
    y++;
    for (i = 0; i < 7; i++) {
      char buf[128];
      snprintf(buf, 128, "%d. %s", i+3, term_title[i]);
      textTermHook(5, y++, strlen(buf), (menu_y == y ? 3 : 1), buf);
    }
  } else if (menu_mode == MENU_BINDS) {
  }

  SDL_SetRenderTarget(td->renderer, NULL);
  SDL_RenderCopy(td->renderer, td->framebuffer, NULL, &td->ren_rect);
  SDL_RenderPresent(td->renderer);
  SDL_SetRenderTarget(td->renderer, td->framebuffer);
  //SDL_RenderCopy(td->renderer,
  Term_activate(old_td);
  return 0;
}
/* handleMenu
This function takes in two points corresponding to the "main" term's cells and does an action
based on some bounds checking. This is about as hard-coded as UI and menu-systems go -- no
abstraction here -- and for the complexity we need, it'd be silly to go beyond this.
*/
errr handleMenu(int x, int y) {
  y -= 5; // menu coords are provided in absolute terms, so we get relative by negative the menu offset
  if (menu_mode == MENU_MAIN) {
    if (y == 0) {
      menu_mode = MENU_WINDOWS;
    } else if (y == 1) {
      menu_mode = MENU_PROPERTIES;
    } else if (y == 2) {
      menu_mode = MENU_BINDS;
    }
  } else if (menu_mode == MENU_WINDOWS) {
    if (y >= 0 && y < 7) {
      y++;
      if (terms[y].config & TERM_IS_ONLINE) {
        term_nuke(&(terms[y].t));
        ang_term[y] = NULL;
      } else {
        initTermData(&terms[y], term_title[y], y, NULL);
        attachFont(&fonts[FONT_SMALL], &terms[y]);
        setTermCells(&terms[y], fonts[FONT_SMALL].w, fonts[FONT_SMALL].h);
        setTermTitle(&terms[y]);
        refreshTerm(&terms[y]);
        ang_term[y] = &(terms[y].t);
      }
    }
  } else if (menu_mode == MENU_PROPERTIES) {
    if (y == 0) {
      printf("saving\n");
      saveConfig();
    } else if (y == 1) {
      loadConfig();
    } else if (y >= 2 && y < 9) {
      printf("modifying %d\n", y-2);
    }
  } else if (menu_mode == MENU_BINDS) {

  }
  return 0;
}
// MMM END
/* ==== Configuration loading/saving functions ==== */
errr loadConfig() {
  FILE *fp;
  char buffer[1024];  // string up to 1024 chars
  char *p, *start;    // pointer to buffer position(token end) and pointer to token begin position
  char section[128];	// the "section" string, up to 127 chars
  char token1[1024];  // the key of a key=>value pair
  char token2[1024];  // the value of a key=>value pair
  memset(section, 0, 128);
  memset(token1, 0, 1024);
  memset(token2, 0, 1024);

  if ((fp = fopen("MAngband.ini", "r"))) {
    printf("LOADING");
    while (!feof(fp)) {
      // Parse our line for tokens delimited by "=", token1 will be populated with key, token2 will be populated with value(or key if no delimiter exists)
      fgets(buffer, 1024, fp);
      start = p = buffer; // default or token position pointers to beginning of string
      token1[0] = token2[0] = '\0'; // "clear" the tokens
      do {
        if (*p == '=') { // check for key=>value delimiter (=) and collect the key
          memcpy(token1, start, p-start);
          token1[p-start] = '\0';
          start = p+1;
        } else if (*p == '\0' || *p == ';') { // EOL and comments trigger parse end
          memcpy(token2, start, p-start);
          token2[p-start] = '\0';
          break;
        }
      } while(*p++);
      // Parse our acquired tokens
      if (token1[0] == '\0') { // It's either a value-less key, a section, or a comment
        if (token2[0] == '[' && token2[p-start-2] == ']') { // section
          memcpy(section, token2+1, p-start-3);
          section[p-start-3] = '\0';
          token2[0] = '\0'; // 'null" our value token
        } else if (token2[0] != '\0') { // not a comment
          memcpy(token1, start, p-start); // copy to "key" token1
          token2[0] = '\0'; // "null" our value token
        }
      }
      // At this point, token1 will always be a key, token2 will always be a value, and section might be set.
      if (token1[0] != '\0') {
        parseConfig(section, token1, token2);
      }
    }
  }

  return 0;
}
errr parseConfig(cptr section, cptr key, cptr value) {
  if (strcmp(section, "SDL2") == 0) {
    if (strcmp(key, "term_mode") == 0) {
      if (strcmp(value, "window") == 0) {
        conf |= CONF_TERM_WINDOWED;
      } else if (strcmp(value, "virtual") == 0) {
        conf |= CONF_TERM_VIRTUAL;
      }
    } else if (strcmp(key, "font_default") == 0) {
      strcpy(default_font, value);
    } else if (strcmp(key, "font_size") == 0) {
      default_font_size = atoi(value);
    }
  } else if (strncmp(section, "window", 6) == 0) {
    // Since we refer to windows by their index, let's just extract that and do common operations on all
    if (strlen(section) > 6) {
      char *window = (char*)&(section[7]);
      int window_id = atoi(window);
      if (window_id >= 0 && window_id <= 7) {
        if (strcmp(key, "title") == 0) {
          strncpy(terms[window_id].title, value, 128);
        } else if (strcmp(key, "pict_file") == 0) {
          strncpy(terms[window_id].pict_file, value, 128);
        } else if (strcmp(key, "font_file") == 0) {
          strncpy(terms[window_id].pict_file, value, 128);
        } else if (strcmp(key, "font_size") == 0) {
          terms[window_id].font_size = atoi(value);
        } else if (strcmp(key, "cell_width") == 0) {
          terms[window_id].cell_w = atoi(value);
        } else if (strcmp(key, "cell_height") == 0) {
          terms[window_id].cell_h = atoi(value);
        } else if (strcmp(key, "hidden") == 0) {
          if (atoi(value) == 1) {
            terms[window_id].config |= TERM_IS_HIDDEN;
          }
        } else if (strcmp(key, "x") == 0) {
          terms[window_id].x = atoi(value);
        } else if (strcmp(key, "y") == 0) {
          terms[window_id].y = atoi(value);
        } else if (strcmp(key, "width") == 0) {
          terms[window_id].width = atoi(value);
        } else if (strcmp(key, "height") == 0) {
          terms[window_id].height = atoi(value);
        }
      }
    }
  }
  return 0;
}
errr saveConfig() {
  return 0;
}
/* ==== Font-related functions ==== */
/* cleanFontData
This function takes the FontData, destroys its SDL_Texture (if it exists), nulls it, then sets its entire struct to 0.
*/
errr cleanFontData(FontData *fd) {
  if (fd->surface) SDL_FreeSurface(fd->surface);
  fd->surface = NULL;
  memset(fd, 0, sizeof(FontData));
  return 0;
}
/* ttfToFont
This function takes the given FontData structure and attempts to make a Glyph Table texture from the filename at the point size fontsize with sharp or smooth rendering via the smoothing boolean.
*/
errr ttfToFont(FontData *fd, cptr filename, int fontsize, int smoothing) {
  TTF_Font *font;
  int minx, maxx, miny, maxy, width, height;
  int i;
  char buf[1036];
  if (fd->w || fd->h || fd->surface) return 1; // Return if FontData is not clean
  // Get and open our TTF font from the xtra dir
  path_build(buf, 1024, ANGBAND_DIR_XTRA, filename);
  font = TTF_OpenFont(buf, fontsize);
  if (!font) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "TTF_OpenFont", TTF_GetError(), NULL);
    return 1;
  }
  // Poorly get our font metrics and maximum cell size in pixels
  width = 0;
  height = 0;
  for (i = 0; i < 255; i++) {
    TTF_GlyphMetrics(font, (char)i, &minx, &maxx, &miny, &maxy, NULL);
    if (minx+maxx > width) width = minx+maxx;
    if (miny+maxy > height) height = miny+maxy;
  }
  fd->w = fd->dw = width;
  fd->h = fd->dh = height;
  // Create our glyph surface that will store 256 characters in a 16x16 matrix
  fd->surface = SDL_CreateRGBSurface(0, width*16, height*16, 32, 0, 0, 0, 0);
  if (fd->surface == NULL) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ttfToFont Error", SDL_GetError(), NULL);
    return 1;
  }
  // Painstakingly create each glyph as a surface and blit to our glyphs surface
  {
    int ch = 0;
    for (ch = 0; ch < 256; ch++) {
      SDL_Color color = { 255, 255, 255 };
      SDL_Surface *char_surface = (smoothing ? TTF_RenderGlyph_Blended(font, ch, color) : TTF_RenderGlyph_Solid(font, ch, color));
      int row = ch / 16;
      int col = ch - (row*16);
      SDL_Rect glyph_rect = { col*width, row*height, width, height };
      SDL_BlitSurface(char_surface, NULL, fd->surface, &glyph_rect);
      SDL_FreeSurface(char_surface);
    }
  }
  // We're done with the font
  TTF_CloseFont(font);
  return 0;
}
/* ==== Pict-related functions ==== */
errr cleanPictData(PictData *pd) {
  if (pd->surface) SDL_FreeSurface(pd->surface);
  pd->surface = NULL;
  memset(pd, 0, sizeof(PictData));
  return 0;
}
errr imgToPict(PictData *pd, cptr filename) {
  Uint32 width, height;
  char buf[1036];
  if (pd->w || pd->h || pd->surface) return 1; // return if PictData is unclean
  // Get and open our BMP from the xtra dir
  path_build(buf, 1024, ANGBAND_DIR_XTRA, filename);
  // Load 'er up
  pd->surface = IMG_Load(buf);
  if (pd->surface == NULL) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "imgToPict", IMG_GetError(), NULL);
    return 1;
  }
  // Cool, get our dimensions
  width = 0, height = 0;
  if (strtoii(filename, &width, &height) != 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "imgToPict", "some strtoii error", NULL);
    // TODO: free and return
  }
  if (width == 0 || height == 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "imgToPict", "width or height of 0 is not allowed!", NULL);
    // TODO: free and return
  }
  // set up our PictData
  pd->w = width; pd->h = height;
  return 0;
}
/* ==== Other people's code ==== */
/* strtoii
Function that extracts the numerics from a string such as "sprites8x16.bmp"

Taken from "maim_sdl.c" and presumably: Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
*/
errr strtoii(const char *str, Uint32 *w, Uint32 *h) {
  char buf[1024];
  char *s = buf;
  char *tok;
  char *numeric = "0123456789";

  size_t l; /* length of numeric string */

  if (!str || !w || !h) return -1;

  if (strlen(str) < 3) return -1; /* must have room for at least "1x1" */

  strncpy(buf, str, 1023);
  buf[1023] = '\0';

  tok = strpbrk(buf, numeric);
  if (!tok) return -1;

  l = strspn(tok, numeric);
  if (!l) return -1;

  tok[l] = '\0';

  s = tok + l + 1;

  if(!sscanf(tok, "%d", w)) return -1;

  /* next token */
  tok = strpbrk(s, numeric);
  if (!tok) return -1;

  l = strspn(tok, numeric);
  if (!l) return -1;

  tok[l] = '\0';
  /* no need to set s since this is the last token */

  if(!sscanf(tok, "%d", h)) return -1;

  return 0;
}
#endif
