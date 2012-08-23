#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"

#include "rational_bezier.h"
#include "pnt.h"
#include "util.h"

#include <math.h>
#include <vector>

// screen width, height, and bit depth
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     16

enum LogLevel {
    TRACE,
    INFO,
    WARN,
    ERROR,
    FATAL
} debug_lvl;

#define D(lvl, ...) \
    if (debug_lvl <= lvl) { \
        fprintf(stderr, __VA_ARGS__); \
    }

SDL_Surface *surface;

double near_treshold = 10.0;
int moving_point_idx = -1;

std::vector<pnt> points;

int nearestPoint(pnt p) {
    if (points.empty()) return -1;
    int res = 0;
    double sl = p.len2(points[0]);
    for (int i = 1; i < (int) points.size(); ++i) {
        double cl = p.len2(points[i]);
        if (sl > cl) {
            sl = cl;
            res = i;
        }
    }
    return res;
}

int focus_pnt_idx = -1;

int weight_point_idx = -1;

// weight widget variables
double ww_height = 80;
double ww_width = 20;
double ww_margin = 4;
double ww_bar_width = 4;
double ww_fill;  // must be in [-1.0, 1.0]
pnt ww_po = pnt(20, -ww_height / 2.0);

pnt ww_origin;

int width, height;

// receives weight in [-1.0, 1.0] and returns in [-0.5, 2.0]
double scale_weight(double w) {
    return w > 0.0 ? w * 2 : w * 0.5;
}

// receives weight in [-0.5, 2.0] and returns in [-1.0, 1.0]
double unscale_weight(double w) {
    return w < 0.0 ? w * 2 : w * 0.5;
}

void display_ww(int idx) {
    pnt p = points[idx];
    pnt orig = p + ww_po;
    ww_origin = orig.bound(pnt(0, 0), pnt(width - ww_width, height - ww_height));
    weight_point_idx = idx;
    ww_fill = unscale_weight(p.w);
}

void hide_ww() {
    weight_point_idx = -1;
}

void draw_ww() {
    glColor4f(0.2f, 0.2f, 0.8f, 0.0f);
    glBegin(GL_POLYGON);
        ww_origin.draw();
        (ww_origin + pnt(0, ww_height)).draw();
        (ww_origin + pnt(ww_width, ww_height)).draw();
        (ww_origin + pnt(ww_width, 0)).draw();
    glEnd();

    // little white line in the middle
    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    glDisable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);
        (ww_origin + pnt(ww_margin, ww_height / 2.)).draw();
        (ww_origin + pnt(ww_width - ww_margin, ww_height / 2.)).draw();
    glEnd();

    // white blue adjustable bar indicator
    double usable_height = ww_height / 2. - ww_margin;
    glColor4f(0.0f, 1.0f, 1.0f, 0.0f);
    glLineWidth(ww_bar_width);
    glDisable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);
        (ww_origin + pnt(ww_margin + ww_bar_width / 2., ww_height / 2.0)).draw();
        (ww_origin + pnt(ww_margin + ww_bar_width / 2.,
                         ww_height / 2. - ww_fill * usable_height)).draw();
    glEnd();
}

bool wwHandleMouse(struct SDL_MouseButtonEvent *mouse, const pnt &p) {
    if (weight_point_idx == -1) {
        return false;
    }
    if (p.inRect(ww_origin, ww_origin + pnt(ww_width, ww_height))) {
        ww_fill = (ww_origin.y + ww_height / 2. - p.y) / (ww_height / 2. - ww_margin);
        ww_fill = bound(ww_fill, -1, 1);
        double scaled_weight = scale_weight(ww_fill);
        D(INFO, "ww_fill %lf, scaled %lf\n", ww_fill, scaled_weight);
        // set the weight of the point
        points[weight_point_idx].w = scaled_weight;
        return true;
    }
    return false;
}

// function to release/destroy our resources and restore the old desktop
void Quit(int returnCode) {
    // clean up the window and exit appropriately
    SDL_Quit();
    exit(returnCode);
}

// function to reset our viewport after a window resize
bool resizeWindow(int width_, int height_) {
    width = width_;
    height = height_;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, 0, 1);

    // make sure we're chaning the model view and not the projection
    glMatrixMode(GL_MODELVIEW);

    // reset the view
    glLoadIdentity();

    return true;
}

void recalcFocus(pnt p) {
    int npi = nearestPoint(p);
    if (npi != -1 && points[npi].len2(p) < sq(near_treshold)) {
        focus_pnt_idx = npi;
    } else {
        focus_pnt_idx = -1;
    }
}
 
void handleMouseMove(struct SDL_MouseMotionEvent *mouseMove) {
    D(TRACE, "%3d %3d | %d\n", mouseMove->x, mouseMove->y, mouseMove->state);
    pnt p(mouseMove->x, mouseMove->y);
    if (moving_point_idx != -1) {
        p.w = points[moving_point_idx].w;
        points[moving_point_idx] = p;
    }
    recalcFocus(p);
}

void handleMouse(struct SDL_MouseButtonEvent *mouse, int down) {
    D(TRACE, "%3d %3d | %d %d %d\n",
            mouse->x, mouse->y, down, mouse->button, mouse->state);


    pnt p = pnt(mouse->x, mouse->y);
    if (!down) {
        if (moving_point_idx != -1) {
            p.w = points[moving_point_idx].w;
            points[moving_point_idx] = p;
            moving_point_idx = -1;
            D(INFO, "move completed\n");
        }
        return;
    }
    if (moving_point_idx != -1) {
        moving_point_idx = -1;
        D(ERROR, "moving_point_idx != -1 and mouse was up\n");
    }
    if (weight_point_idx != -1) {
        if (!wwHandleMouse(mouse, p)) {
            D(INFO, "wwHandle passed\n");
            hide_ww();
            return;
        } else {
            return;
        }
    }
    int npi = nearestPoint(p);
    if (npi != -1 && points[npi].len2(p) < sq(near_treshold)) {
        if (mouse->button == 1) {
            moving_point_idx = npi;
            D(INFO, "starting move\n");
        } else if (mouse->button == 3) {
            display_ww(npi);
        }
    } else {
        points.push_back(p);
        D(INFO, "adding point (%lf, %lf)\n", p.x, p.y);
    }
    recalcFocus(p);
}

// function to handle key press events
void handleKeyPress(SDL_keysym *keysym) {
    switch (keysym->sym) {
    case SDLK_ESCAPE:
        // ESC key was pressed
        Quit(0);
        break;
    // case SDLK_b:
    //     // 'b' key was pressed
    //      * this toggles blending
    //     
    //     blend = !blend;
    //     if (blend)
    //     {
    //         glEnable(GL_BLEND);
    //         glDisable(GL_DEPTH_TEST);
    //     }
    //     else
    //     {
    //         glDisable(GL_BLEND);
    //         glEnable(GL_DEPTH_TEST);
    //     }
    //     break;
    case SDLK_F1:
        // 'f' key was pressed
        // this toggles fullscreen mode
        
        SDL_WM_ToggleFullScreen(surface);
        break;
    default:
        break;
    }

    return;
}

// general OpenGL initialization function
bool initGL() {
    // set the background to black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // disables depth testing
    glDisable(GL_DEPTH_TEST);

    glPointSize(4.0f);

    return true;
}

// here goes our drawing code
bool drawGLScene() {
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // reset the view
    glLoadIdentity();

    glColor4f(0.4f, 0.4f, 0.4f, 0.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i < (int) points.size(); ++i) {
            points[i].draw();
        }
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
        for (int i = 0; i < (int) points.size(); ++i) {
            if (focus_pnt_idx == i)
                glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
            points[i].draw();
            if (focus_pnt_idx == i)
                glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
        }
    glEnd();

    vector<pnt> curve = rational_bezier(points);
    glColor4f(0.2f, 0.8f, 0.2f, 0.0f);
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i < (int) curve.size(); ++i) {
            curve[i].draw();
        }
    glEnd();

    if (weight_point_idx != -1) {
        draw_ww();
    }

    // draw all to the screen
    SDL_GL_SwapBuffers();

    return true;
}

int main(int argc, char **argv) {
    // flags to pass to SDL_SetVideoMode
    int videoFlags;
    // main loop variable
    bool done = false;
    // used to collect events
    SDL_Event event;
    // this holds some info about our display
    const SDL_VideoInfo *videoInfo;
    // whether or not the window is active
    bool isActive = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Video initialization failed: %s\n",
             SDL_GetError());
        Quit(1);
    }

    // fetch the video info
    videoInfo = SDL_GetVideoInfo();

    if (!videoInfo) {
        fprintf(stderr, "Video query failed: %s\n",
             SDL_GetError());
        Quit(1);
    }

    // the flags to pass to SDL_SetVideoMode
    videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
    videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
    videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware
    videoFlags |= SDL_RESIZABLE;       // Enable window resizing

    // check if surfaces can be stored in memory
    if (videoInfo->hw_available)
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    // check if hardware blits can be done
    if (videoInfo->blit_hw)
       videoFlags |= SDL_HWACCEL;

    // set up OpenGL double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // get an SDL surface
    surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                videoFlags);

    // verify there is a surface
    if (!surface) {
        fprintf(stderr,  "Video mode set failed: %s\n", SDL_GetError());
        Quit(1);
    }

    // initialize OpenGL
    initGL();

    // resize the initial window
    resizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    debug_lvl = WARN;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-debug") == 0) {
            sscanf(argv[i+1], "%d", (int *)&debug_lvl);
            ++i;
        } else {
            printf("unrecognized option: %s\n", argv[i]);
        }
    }

    // wait for events
    while (!done) {
        if (isActive)
            drawGLScene();

        // handle the events in the queue
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_ACTIVEEVENT:
                // something's happend with the focus
                // if we lost focus or we are iconified, we
                // shouldn't draw the screen
                
                if (event.active.gain == 0)
                isActive = false;
                else
                isActive = true;
                break;                
            case SDL_VIDEORESIZE:
                // handle resize event
                surface = SDL_SetVideoMode(event.resize.w,
                            event.resize.h,
                            16, videoFlags);
                if (!surface) {
                    fprintf(stderr, "Could not get a surface after resize: %s\n", SDL_GetError());
                    Quit(1);
                }
                resizeWindow(event.resize.w, event.resize.h);
                break;
            case SDL_KEYDOWN:
                // handle key presses
                handleKeyPress(&event.key.keysym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                handleMouse(&event.button, 1);
                break;
            case SDL_MOUSEBUTTONUP:
                handleMouse(&event.button, 0);
                break;
            case SDL_MOUSEMOTION:
                handleMouseMove(&event.motion);
                break;
            case SDL_QUIT:
                // handle quit requests
                done = true;
                break;
            default:
                break;
            }
        }

    }

    // clean up and exit
    Quit(0);

    // Should never get here
    return 0;
}
