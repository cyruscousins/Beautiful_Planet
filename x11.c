// Bindings to use X11 widows to display graphics.
// Adapted from libpng example X display code by Greg Roelofs, 1998-2008, under GPL-BSD dual license.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#include "x11.h"

static int calculate_msb(unsigned long u32val);

#define PROGNAME "RENDERER"
static char *appname = "Beautiful Planet";
static char *icon_name = "Beautiful Planet";
static char *res_name = "Beautiful Planet";
static char *res_class = "Beautiful Planet";
static FILE *infile;

static char titlebar[1024], *window_name = titlebar;
static double display_exponent;

static unsigned long image_width, image_height;
static uint8_t *image_data;

/* X-specific variables */
static char *displayname;
static XImage *ximage;
static Display *display;
static int depth;
static Visual *visual;
static XVisualInfo *visual_list;
static int RShift, GShift, BShift;
static unsigned long RMask, GMask, BMask;
static Window window;
static GC gc;
static Colormap colormap;

#define FALSE 0
static int have_nondefault_visual = FALSE;
static int have_colormap = FALSE;
static int have_window = FALSE;
static int have_gc = FALSE;


//int main(int argc, char **argv) {
//int x11test(int argc, char ** argv) {
void x_window_init() {
#ifdef sgi
    char tmpline[80];
#endif
    char *p;
    int error = 0;
    double LUT_exponent;               /* just the lookup table */
    double CRT_exponent = 2.2;         /* just the monitor */
    double default_display_exponent;   /* whole display system */

    displayname = (char *)NULL;

    /* First set the default value for our display-system exponent, i.e.,
     * the product of the CRT exponent and the exponent corresponding to
     * the frame-buffer's lookup table (LUT), if any.  This is not an
     * exhaustive list of LUT values (e.g., OpenStep has a lot of weird
     * ones), but it should cover 99% of the current possibilities. */

#if defined(NeXT)
    LUT_exponent = 1.0 / 2.2;
    /*
    if (some_next_function_that_returns_gamma(&next_gamma))
        LUT_exponent = 1.0 / next_gamma;
     */
#elif defined(sgi)
    LUT_exponent = 1.0 / 1.7;
    /* there doesn't seem to be any documented function to get the
     * "gamma" value, so we do it the hard way */
    infile = fopen("/etc/config/system.glGammaVal", "r");
    if (infile) {
        double sgi_gamma;
        fgets(tmpline, 80, infile);
        fclose(infile);
        sgi_gamma = atof(tmpline);
        if (sgi_gamma > 0.0)
            LUT_exponent = 1.0 / sgi_gamma;
    }
#elif defined(Macintosh)
    LUT_exponent = 1.8 / 2.61;
    /*
    if (some_mac_function_that_returns_gamma(&mac_gamma))
        LUT_exponent = mac_gamma / 2.61;
     */
#else
    LUT_exponent = 1.0;   /* assume no LUT:  most PCs */
#endif

    /* the defaults above give 1.0, 1.3, 1.5 and 2.2, respectively: */
    default_display_exponent = LUT_exponent * CRT_exponent;


    /* If the user has set the SCREEN_GAMMA environment variable as suggested
     * (somewhat imprecisely) in the libpng documentation, use that; otherwise
     * use the default value we just calculated.  Either way, the user may
     * override this via a command-line option. */

    if ((p = getenv("SCREEN_GAMMA")) != NULL)
        display_exponent = atof(p);
    else
        display_exponent = default_display_exponent;
    
    display = XOpenDisplay(displayname);
    if (!display) {
        //readpng_cleanup(1);
        fprintf(stderr, PROGNAME ":  can't open X display [%s]\n",
          displayname? displayname : "default");
        ++error;
    }

    //TODO handle these sensibly.
    char* imgname = "Test Render";

    sprintf(titlebar, "Beautiful Planet: \"%s\"", imgname);


#if 0
    #define TILEW 32
    #define TILES 8
    image* img = image_new(TILES * TILEW, TILES * TILEW);
    for(unsigned i = 0; i < TILES * TILEW * TILES * TILEW; i++) {
      unsigned x = i % (TILES * TILEW);
      unsigned y = i / (TILES * TILEW);
      *(image_pixel(img, x, y, R)) = 
      *(image_pixel(img, x, y, G)) = 
      *(image_pixel(img, x, y, B)) = ((x % (TILEW / 2) == 0) || (y % (TILEW / 2) == 0)) ? (x + y) % 2 * 0.5 : ((x / TILEW) + (y / TILEW)) % 2;
    }
  
  

    /* do the basic X initialization stuff, make the window and fill it
     * with the background color */

    if (x_window_create(img->width, img->height))
        exit(2);

  
    if (x_window_display_image(img)) {
        free(image_data);
        exit(4);
    }


    /* wait for the user to tell us when to quit */

    printf(
      "Done.  Press Q, Esc or mouse button 1 (within image window) to quit.\n");
    fflush(stdout);

    do
        XNextEvent(display, &e);
    while (!(e.type == ButtonPress && e.xbutton.button == Button1) &&
           !(e.type == KeyPress &&    /*  v--- or 1 for shifted keys */
             ((k = XLookupKeysym(&e.xkey, 0)) == XK_q || k == XK_Escape) ));


    /* OK, we're done:  clean up all image and X resources and go away */

    x_window_cleanup();

    return 0;
#endif
}

//Maintain state mutated by X11 events.
//The function processXEvents must be called to update these.
//In order to avoid concurrency issues, it is intended that these variables are used by the same thread that calls processXEvents.

#ifdef INTERACTIVE

#define KEY_COUNT 255
bool keys[KEY_COUNT] = {0};
bool isKeyPressed(char k) {
  return keys[k];
}
void setKeyPressed(char k, bool pressed) {
  keys[k] = pressed;
  //printf("set %u %u\n", k, pressed);
}

#define MOUSE_BUTTONS 3
bool mouse_buttons[MOUSE_BUTTONS] = {0};
int mouseX, mouseY;
vec2 getMousePosition() {
  vec2 res = { mouseX, mouseY };
  return res;
}
int getMouseX() {
  return mouseX;
}
int getMouseY() {
  return mouseY;
}

bool terminated;

bool isTerminated() {
  return terminated;
}

void printXState() {
  printf("Keys:\n");
  for(unsigned i = 0; i < KEY_COUNT; i++) {
    printf("%u", (unsigned)keys[i]);
  }
  printf("\n");
  
  printf("Mouse Buttons:\n");
  for(unsigned i = 0; i < MOUSE_BUTTONS; i++) {
    printf("%u", (unsigned)mouse_buttons[i]);
  }
  printf("\n");
  printf("Mouse Position:\n");
  printf("(%u, %u)\n", getMouseX(), getMouseY());
}

void processXEvents() {
  //printXState();
  
  XEvent e;
  KeySym k;
  
  while (XPending(display) && !terminated) {
    XNextEvent(display, &e);
    switch(e.type) {
      case KeyPress:
      case KeyRelease:
        {
          bool pressed = e.type == KeyPress;
          XLookupString(&e.xkey, NULL, 0, &k, NULL);
          //printf("PR %u %u\n", (unsigned)k, (unsigned)pressed);
          //printf("E %u S %u S %u A %u a %u\n",(unsigned)XK_Escape,(unsigned)XK_Shift_L,(unsigned)XK_space,(unsigned)XK_A,(unsigned)XK_a);
          switch(k) {
            case XK_Escape:
              //TODO: This isn't really a useful feature: it's here as more of a test and example of a more complicated event response.
              setKeyPressed(ESCAPE, pressed);
              terminated = true;
              return;
            case XK_space:
              setKeyPressed(SPACE, pressed);
              break;
            case XK_Shift_L:
            case XK_Shift_R:
              setKeyPressed(SHIFT, pressed);
              break;
            default:
              if(k >= XK_A && k <= XK_Z) {
                setKeyPressed('a' + (k - XK_A), pressed);
              }
              if(k >= XK_a && k <= XK_z) {
                setKeyPressed('a' + (k - XK_a), pressed);
              }
              break;
          }
        }
        break;
      case MotionNotify:
        mouseX = e.xmotion.x;
        mouseY = e.xmotion.y;
        //printf("mouse (%u, %u) (%u, %u) (%u, %u)\n", getMouseX(), getMouseY(), e.xmotion.x, e.xmotion.y, e.xmotion.x_root, e.xmotion.y_root);
        break;
      //TODO mouse buttons.
    }
  }
}

#endif

int x_window_create(unsigned w, unsigned h) {
  image_width = w;
  image_height = h;
  
    uint8_t *xdata;
    int need_colormap = FALSE;
    int screen, pad;
    unsigned long attrmask;
    Window root;
    XEvent e;
    XGCValues gcvalues;
    XSetWindowAttributes attr;
    XTextProperty windowName, *pWindowName = &windowName;
    XTextProperty iconName, *pIconName = &iconName;
    XVisualInfo visual_info;
    XSizeHints *size_hints;
    XWMHints *wm_hints;
    XClassHint *class_hints;


    screen = DefaultScreen(display);
    depth = DisplayPlanes(display, screen);
    root = RootWindow(display, screen);

#ifdef DEBUG
    XSynchronize(display, True);
#endif

#if 0
/* GRR:  add 8-bit support */
    if (/* depth != 8 && */ depth != 16 && depth != 24 && depth != 32) {
        fprintf(stderr,
          "screen depth %d not supported (only 16-, 24- or 32-bit TrueColor)\n",
          depth);
        return 2;
    }

    XMatchVisualInfo(display, screen, depth,
      (depth == 8)? PseudoColor : TrueColor, &visual_info);
    visual = visual_info.visual;
#else
    if (depth != 16 && depth != 24 && depth != 32) {
        int visuals_matched = 0;

        /* 24-bit first */
        visual_info.screen = screen;
        visual_info.depth = 24;
        visual_list = XGetVisualInfo(display, VisualScreenMask | VisualDepthMask, &visual_info, &visuals_matched);
        if (visuals_matched == 0) {
/* GRR:  add 15-, 16- and 32-bit TrueColor visuals (also DirectColor?) */
            fprintf(stderr, "default screen depth %d not supported, and no"
              " 24-bit visuals found\n", depth);
            return 2;
        }
        visual = visual_list[0].visual;
        depth = visual_list[0].depth;
/*
        colormap_size = visual_list[0].colormap_size;
        visual_class = visual->class;
        visualID = XVisualIDFromVisual(visual);
 */
 #define TRUE 1
        have_nondefault_visual = TRUE;
        need_colormap = TRUE;
    } else {
        XMatchVisualInfo(display, screen, depth, TrueColor, &visual_info);
        visual = visual_info.visual;
    }
#endif

    RMask = visual->red_mask;
    GMask = visual->green_mask;
    BMask = visual->blue_mask;

/* GRR:  add/check 8-bit support */
    if (depth == 8 || need_colormap) {
        colormap = XCreateColormap(display, root, visual, AllocNone);
        if (!colormap) {
            fprintf(stderr, "XCreateColormap() failed\n");
            return 2;
        }
        have_colormap = TRUE;
    }
    if (depth == 15 || depth == 16) {
        RShift = 15 - calculate_msb(RMask);    /* these are right-shifts */
        GShift = 15 - calculate_msb(GMask);
        BShift = 15 - calculate_msb(BMask);
    } else if (depth > 16) {
#define NO_24BIT_MASKS
#ifdef NO_24BIT_MASKS
        RShift = calculate_msb(RMask) - 7;     /* these are left-shifts */
        GShift = calculate_msb(GMask) - 7;
        BShift = calculate_msb(BMask) - 7;
#else
        RShift = 7 - calculate_msb(RMask);     /* these are right-shifts, too */
        GShift = 7 - calculate_msb(GMask);
        BShift = 7 - calculate_msb(BMask);
#endif
    }
    if (depth >= 15 && (RShift < 0 || GShift < 0 || BShift < 0)) {
        fprintf(stderr, "rpng internal logic error:  negative X shift(s)!\n");
        return 2;
    }

/*---------------------------------------------------------------------------
    Finally, create the window.
  ---------------------------------------------------------------------------*/

    attr.backing_store = Always;
    attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask;
    attrmask = CWBackingStore | CWEventMask;
    if (have_nondefault_visual) {
        attr.colormap = colormap;
        attr.background_pixel = 0;
        attr.border_pixel = 1;
        attrmask |= CWColormap | CWBackPixel | CWBorderPixel;
    }
    
    #ifdef INTERACTIVE
    attr.event_mask |= ExposureMask | KeyPressMask | ButtonPress | StructureNotifyMask | ButtonReleaseMask | KeyReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | Button1MotionMask | VisibilityChangeMask | ColormapChangeMask;
    attrmask |= CWEventMask | CWBackPixel | CWBorderPixel;
    //attrmask |= CWCursor;
    
    //TODO: I read that CWCursor was necessary to recieve cursor events.  However, it seems that using CWCursor causes a crash:
    /*
      X Error of failed request:  BadCursor (invalid Cursor parameter)
      Major opcode of failed request:  1 (X_CreateWindow)
      Resource id in failed request:  0x65696088
      Serial number of failed request:  7
      Current serial number in output stream:  8
    */
    #endif
    
    //printf("%d %d %d\n", image_width, image_height, depth);
    //depth = 24; 

    window = XCreateWindow(display, root, 0, 0, image_width, image_height, 0, depth, InputOutput, visual, attrmask, &attr);

    if (window == None) {
        fprintf(stderr, "XCreateWindow() failed\n");
        return 2;
    } else
        have_window = TRUE;

    if (depth == 8)
        XSetWindowColormap(display, window, colormap);

    if (!XStringListToTextProperty(&window_name, 1, pWindowName))
        pWindowName = NULL;
    if (!XStringListToTextProperty(&icon_name, 1, pIconName))
        pIconName = NULL;

    /* OK if any hints allocation fails; XSetWMProperties() allows NULLs */

    if ((size_hints = XAllocSizeHints()) != NULL) {
        /* window will not be resizable */
        size_hints->flags = PMinSize | PMaxSize;
        size_hints->min_width = size_hints->max_width = (int)image_width;
        size_hints->min_height = size_hints->max_height = (int)image_height;
    }

    if ((wm_hints = XAllocWMHints()) != NULL) {
        wm_hints->initial_state = NormalState;
        wm_hints->input = True;
     /* wm_hints->icon_pixmap = icon_pixmap; */
        wm_hints->flags = StateHint | InputHint  /* | IconPixmapHint */ ;
    }

    if ((class_hints = XAllocClassHint()) != NULL) {
        class_hints->res_name = res_name;
        class_hints->res_class = res_class;
    }

    XSetWMProperties(display, window, pWindowName, pIconName, NULL, 0,
      size_hints, wm_hints, class_hints);

    /* various properties and hints no longer needed; free memory */
    if (pWindowName)
       XFree(pWindowName->value);
    if (pIconName)
       XFree(pIconName->value);
    if (size_hints)
        XFree(size_hints);
    if (wm_hints)
       XFree(wm_hints);
    if (class_hints)
       XFree(class_hints);

    XMapWindow(display, window);

    gc = XCreateGC(display, window, 0, &gcvalues);
    have_gc = TRUE;

/*---------------------------------------------------------------------------
    Fill window with the specified background color.
  ---------------------------------------------------------------------------*/
#if 0
    if (depth == 24 || depth == 32) {
        bg_pixel = ((unsigned long)bg_red   << RShift) |
                   ((unsigned long)bg_green << GShift) |
                   ((unsigned long)bg_blue  << BShift);
    } else if (depth == 16) {
        bg_pixel = ((((unsigned long)bg_red   << 8) >> RShift) & RMask) |
                   ((((unsigned long)bg_green << 8) >> GShift) & GMask) |
                   ((((unsigned long)bg_blue  << 8) >> BShift) & BMask);
    } else /* depth == 8 */ {

        /* GRR:  add 8-bit support */

    }

    XSetForeground(display, gc, bg_pixel);
    XFillRectangle(display, window, gc, 0, 0, image_width, image_height);
#endif
/*---------------------------------------------------------------------------
    Wait for first Expose event to do any drawing, then flush.
  ---------------------------------------------------------------------------*/

    //TODO should search for an expose in interactive.
    #ifdef INTERACTIVE
    #else
    do XNextEvent(display, &e);
    while (e.type != Expose || e.xexpose.count);
    #endif
    
    XFlush(display);

/*---------------------------------------------------------------------------
    Allocate memory for the X- and display-specific version of the image.
  ---------------------------------------------------------------------------*/

    if (depth == 24 || depth == 32) {
        xdata = (uint8_t *)malloc(4*image_width*image_height);
        pad = 32;
    } else if (depth == 16) {
        xdata = (uint8_t *)malloc(2*image_width*image_height);
        pad = 16;
    } else /* depth == 8 */ {
        xdata = (uint8_t *)malloc(image_width*image_height);
        pad = 8;
    }

    if (!xdata) {
        fprintf(stderr, PROGNAME ":  unable to allocate image memory\n");
        return 4;
    }

    ximage = XCreateImage(display, visual, depth, ZPixmap, 0, (char*)xdata, image_width, image_height, pad, 0);
    if (!ximage) {
        fprintf(stderr, PROGNAME ":  XCreateImage() failed\n");
        free(xdata);
        return 3;
    }

    ximage->byte_order = MSBFirst;
    return 0;
}

int x_window_display_image(image* img) {
    //uint8_t *src;
    //char *dest;
    //uint8_t r, g, b, a;
    //unsigned long i, row, 
    unsigned lastrow = 0;
    //unsigned long pixel;
    int ximage_rowbytes = ximage->bytes_per_line;
    //int bpp = ximage->bits_per_pixel;
/*
    Trace((stderr, "beginning display loop (image_channels == %d)\n",
      image_channels))
    Trace((stderr, "   (width = %ld, rowbytes = %ld, ximage_rowbytes = %d)\n",
      image_width, image_rowbytes, ximage_rowbytes))
    Trace((stderr, "   (bpp = %d)\n", ximage->bits_per_pixel))
    Trace((stderr, "   (byte_order = %s)\n", ximage->byte_order == MSBFirst?
      "MSBFirst" : (ximage->byte_order == LSBFirst? "LSBFirst" : "unknown")))
*/
  //ARGB 8 bit systems.
  for(unsigned y = 0; y < img->height; y++) {
    uint8_t* d = (uint8_t*)(ximage->data + y * ximage_rowbytes);
    for(unsigned x = 0; x < img->width; x++) {
      d++; //Skip the alpha channel.
      for(unsigned c = 0; c < C; c++) {
        *d++ = image_pixel_8bit_full(img, x, y, c);
      }
    }
  }
  
  //More generic code for other color modes.
  #if 0
      if (depth == 24 || depth == 32) {
        ulg red, green, blue;

        for (lastrow = row = 0;  row < image_height;  ++row) {
            src = image_data + row*image_rowbytes;
            dest = ximage->data + row*ximage_rowbytes;
            if (image_channels == 3) {
                for (i = image_width;  i > 0;  --i) {
                    red   = *src++;
                    green = *src++;
                    blue  = *src++;
#ifdef NO_24BIT_MASKS
                    pixel = (red   << RShift) |
                            (green << GShift) |
                            (blue  << BShift);
                    /* recall that we set ximage->byte_order = MSBFirst above */
                    /* GRR BUG:  this assumes bpp == 32, but may be 24: */
                    *dest++ = (char)((pixel >> 24) & 0xff);
                    *dest++ = (char)((pixel >> 16) & 0xff);
                    *dest++ = (char)((pixel >>  8) & 0xff);
                    *dest++ = (char)( pixel        & 0xff);
#else
                    red   = (RShift < 0)? red   << (-RShift) : red   >> RShift;
                    green = (GShift < 0)? green << (-GShift) : green >> GShift;
                    blue  = (BShift < 0)? blue  << (-BShift) : blue  >> BShift;
                    pixel = (red & RMask) | (green & GMask) | (blue & BMask);
                    /* recall that we set ximage->byte_order = MSBFirst above */
                    *dest++ = (char)((pixel >> 24) & 0xff);
                    *dest++ = (char)((pixel >> 16) & 0xff);
                    *dest++ = (char)((pixel >>  8) & 0xff);
                    *dest++ = (char)( pixel        & 0xff);
#endif
                }
            } else /* if (image_channels == 4) */ {
                for (i = image_width;  i > 0;  --i) {
                    r = *src++;
                    g = *src++;
                    b = *src++;
                    a = *src++;
                    if (a == 255) {
                        red   = r;
                        green = g;
                        blue  = b;
                    } else if (a == 0) {
                        red   = bg_red;
                        green = bg_green;
                        blue  = bg_blue;
                    } else {
                        /* this macro (from png.h) composites the foreground
                         * and background values and puts the result into the
                         * first argument */
                        alpha_composite(red,   r, a, bg_red);
                        alpha_composite(green, g, a, bg_green);
                        alpha_composite(blue,  b, a, bg_blue);
                    }
                    pixel = (red   << RShift) |
                            (green << GShift) |
                            (blue  << BShift);
                    /* recall that we set ximage->byte_order = MSBFirst above */
                    *dest++ = (char)((pixel >> 24) & 0xff);
                    *dest++ = (char)((pixel >> 16) & 0xff);
                    *dest++ = (char)((pixel >>  8) & 0xff);
                    *dest++ = (char)( pixel        & 0xff);
                }
            }
            /* display after every 16 lines */
            if (((row+1) & 0xf) == 0) {
                XPutImage(display, window, gc, ximage, 0, (int)lastrow, 0,
                  (int)lastrow, image_width, 16);
                XFlush(display);
                lastrow = row + 1;
            }
        }
    } else if (depth == 16) {
        ush red, green, blue;

        for (lastrow = row = 0;  row < image_height;  ++row) {
            src = image_data + row*image_rowbytes;
            dest = ximage->data + row*ximage_rowbytes;
            if (image_channels == 3) {
                for (i = image_width;  i > 0;  --i) {
                    red   = ((ush)(*src) << 8);
                    ++src;
                    green = ((ush)(*src) << 8);
                    ++src;
                    blue  = ((ush)(*src) << 8);
                    ++src;
                    pixel = ((red   >> RShift) & RMask) |
                            ((green >> GShift) & GMask) |
                            ((blue  >> BShift) & BMask);
                    /* recall that we set ximage->byte_order = MSBFirst above */
                    *dest++ = (char)((pixel >>  8) & 0xff);
                    *dest++ = (char)( pixel        & 0xff);
                }
            } else /* if (image_channels == 4) */ {
                for (i = image_width;  i > 0;  --i) {
                    r = *src++;
                    g = *src++;
                    b = *src++;
                    a = *src++;
                    if (a == 255) {
                        red   = ((ush)r << 8);
                        green = ((ush)g << 8);
                        blue  = ((ush)b << 8);
                    } else if (a == 0) {
                        red   = ((ush)bg_red   << 8);
                        green = ((ush)bg_green << 8);
                        blue  = ((ush)bg_blue  << 8);
                    } else {
                        /* this macro (from png.h) composites the foreground
                         * and background values and puts the result back into
                         * the first argument (== fg byte here:  safe) */
                        alpha_composite(r, r, a, bg_red);
                        alpha_composite(g, g, a, bg_green);
                        alpha_composite(b, b, a, bg_blue);
                        red   = ((ush)r << 8);
                        green = ((ush)g << 8);
                        blue  = ((ush)b << 8);
                    }
                    pixel = ((red   >> RShift) & RMask) |
                            ((green >> GShift) & GMask) |
                            ((blue  >> BShift) & BMask);
                    /* recall that we set ximage->byte_order = MSBFirst above */
                    *dest++ = (char)((pixel >>  8) & 0xff);
                    *dest++ = (char)( pixel        & 0xff);
                }
            }
            /* display after every 16 lines */
            if (((row+1) & 0xf) == 0) {
                XPutImage(display, window, gc, ximage, 0, (int)lastrow, 0,
                  (int)lastrow, image_width, 16);
                XFlush(display);
                lastrow = row + 1;
            }
        }
    }
    #endif
    
  XPutImage(display, window, gc, ximage, 0, (int)lastrow, 0, (int)lastrow, image_width, image_height-lastrow);
  XFlush(display);

  return 0;
}

void x_window_cleanup() {
    if (image_data) {
        free(image_data);
        image_data = NULL;
    }
    if (ximage) {
        if (ximage->data) {
            free(ximage->data);
            ximage->data = (char*)NULL;
        }
        XDestroyImage(ximage);
        ximage = NULL;
    }
    if (have_gc) XFreeGC(display, gc);
    if (have_window) XDestroyWindow(display, window);
    if (have_colormap) XFreeColormap(display, colormap);
    if (have_nondefault_visual) XFree(visual_list);
}

static int calculate_msb(unsigned long u32val) {
  for (int i = 31; i >= 0;  --i) {
    if (u32val & 0x80000000L) return i;
    u32val <<= 1;
  }
  return -1;
}
