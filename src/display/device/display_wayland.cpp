/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


// Code based on public domain sample at
// https://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_%28GLX%29

#include <pangolin/platform.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/gl/glglut.h>
#include <pangolin/display/display.h>
#include <pangolin/display/display_internal.h>
#include <pangolin/display/window.h>

#include <pangolin/display/device/WaylandWindow.h>

#include <mutex>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include <GL/glx.h>

namespace pangolin
{
extern __thread PangolinGl* context;

std::mutex window_mutex;
std::weak_ptr<WaylandGlContext> global_gl_context;

const long EVENT_MASKS = ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|ButtonMotionMask|PointerMotionMask|KeyPressMask|KeyReleaseMask|FocusChangeMask;

#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef EGLContext (*glXCreateContextAttribsARBProc)(::Display*, ::EGLContext, Bool, const int*);


WaylandGlContext::WaylandGlContext(std::shared_ptr<WaylandDisplay>& d, std::shared_ptr<WaylandGlContext> shared_context)
    : display(d), shared_context(shared_context)
{
    // prevent chained sharing
    while(shared_context && shared_context->shared_context) {
        shared_context = shared_context->shared_context;
    }

    // Contexts can't be shared across different displays.
    if(shared_context && shared_context->display != d) {
        shared_context.reset();
    }

    eglInitialize(egl_display, &major, &minor);

    eglGetConfigs(egl_display, NULL, 0, &count);
    // printf("EGL has %d configs\n", count);

    eglChooseConfig(egl_display, attribs, egl_configs, count, &numConfigs);

    struct wl_egl_window *egl_window = wl_egl_window_create(wsurface, width, height);
    if (egl_window == EGL_NO_SURFACE) {
        fprintf(stderr, "Can't create egl window\n");
        exit(1);
    }

    egl_surface = eglCreateWindowSurface(egl_display, egl_configs[0], (EGLNativeWindowType)egl_window, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Can't create egl surface\n");
        // exit(1);
    }

    egl_context = eglCreateContext(egl_display, egl_configs[0], EGL_NO_CONTEXT, attrib_list);
    eglcontext = egl_context;
}

WaylandGlContext::~WaylandGlContext()
{
    // glXDestroyContext( display->display, glcontext );
}

WaylandWindow::WaylandWindow(
    const std::string& title, int w, int h,
    std::shared_ptr<WaylandDisplay>& display
)
{
    width=w;
    height=h;
    PangolinGl::windowed_size[0] = width;
    PangolinGl::windowed_size[1] = height;
}

WaylandWindow::~WaylandWindow()
{
}

void WaylandWindow::MakeCurrent(EGLContext ctx)
{
    eglMakeCurrent(egl_display, egl_surface, egl_surface, ctx);
    context = this;
}

void WaylandWindow::MakeCurrent()
{
    MakeCurrent(eglcontext ? eglcontext->eglcontext : global_gl_context.lock()->eglcontext);
}

void WaylandWindow::ToggleFullscreen()
{
}

void WaylandWindow::Move(int x, int y)
{
}

void WaylandWindow::Resize(unsigned int w, unsigned int h)
{
}

void WaylandWindow::ProcessEvents()
{
   /* XEvent ev;
    while(!pangolin::ShouldQuit() && XPending(egl_display) > 0)
    {
        XNextEvent(egl_display, &ev);

        switch(ev.type){
        case ConfigureNotify:
            pangolin::process::Resize(ev.xconfigure.width, ev.xconfigure.height);
            break;
        case ClientMessage:
            // We've only registered to receive WM_DELETE_WINDOW, so no further checks needed.
            pangolin::Quit();
            break;
        case ButtonPress:
        case ButtonRelease:
        {
            const int button = ev.xbutton.button-1;
            const int mask = Button1Mask << button;
            pangolin::process::Mouse(
                button,
                ev.xbutton.state & mask,
                ev.xbutton.x, ev.xbutton.y
            );
            break;
        }
        case FocusOut:
            pangolin::context->mouse_state = 0;
            break;
        case MotionNotify:
            if(ev.xmotion.state & (Button1Mask|Button2Mask|Button3Mask) ) {
                pangolin::process::MouseMotion(ev.xmotion.x, ev.xmotion.y);
            }else{
                pangolin::process::PassiveMouseMotion(ev.xmotion.x, ev.xmotion.y);
            }
            break;
        case KeyPress:
        case KeyRelease:
            int key;
            char ch;
            KeySym sym;

            if( XLookupString(&ev.xkey,&ch,1,&sym,0) == 0) {
                switch (sym) {
                case XK_F1:        key = PANGO_SPECIAL + PANGO_KEY_F1         ; break;
                case XK_F2:        key = PANGO_SPECIAL + PANGO_KEY_F2         ; break;
                case XK_F3:        key = PANGO_SPECIAL + PANGO_KEY_F3         ; break;
                case XK_F4:        key = PANGO_SPECIAL + PANGO_KEY_F4         ; break;
                case XK_F5:        key = PANGO_SPECIAL + PANGO_KEY_F5         ; break;
                case XK_F6:        key = PANGO_SPECIAL + PANGO_KEY_F6         ; break;
                case XK_F7:        key = PANGO_SPECIAL + PANGO_KEY_F7         ; break;
                case XK_F8:        key = PANGO_SPECIAL + PANGO_KEY_F8         ; break;
                case XK_F9:        key = PANGO_SPECIAL + PANGO_KEY_F9         ; break;
                case XK_F10:       key = PANGO_SPECIAL + PANGO_KEY_F10        ; break;
                case XK_F11:       key = PANGO_SPECIAL + PANGO_KEY_F11        ; break;
                case XK_F12:       key = PANGO_SPECIAL + PANGO_KEY_F12        ; break;
                case XK_Left:      key = PANGO_SPECIAL + PANGO_KEY_LEFT       ; break;
                case XK_Up:        key = PANGO_SPECIAL + PANGO_KEY_UP         ; break;
                case XK_Right:     key = PANGO_SPECIAL + PANGO_KEY_RIGHT      ; break;
                case XK_Down:      key = PANGO_SPECIAL + PANGO_KEY_DOWN       ; break;
                case XK_Page_Up:   key = PANGO_SPECIAL + PANGO_KEY_PAGE_UP    ; break;
                case XK_Page_Down: key = PANGO_SPECIAL + PANGO_KEY_PAGE_DOWN  ; break;
                case XK_Home:      key = PANGO_SPECIAL + PANGO_KEY_HOME       ; break;
                case XK_End:       key = PANGO_SPECIAL + PANGO_KEY_END        ; break;
                case XK_Insert:    key = PANGO_SPECIAL + PANGO_KEY_INSERT     ; break;
                case XK_Shift_L:
                case XK_Shift_R:
                    key = -1;
                    if(ev.type==KeyPress) {
                        pangolin::context->mouse_state |=  pangolin::KeyModifierShift;
                    }else{
                        pangolin::context->mouse_state &= ~pangolin::KeyModifierShift;
                    }
                    break;
                case XK_Control_L:
                case XK_Control_R:
                    key = -1;
                    if(ev.type==KeyPress) {
                        pangolin::context->mouse_state |=  pangolin::KeyModifierCtrl;
                    }else{
                        pangolin::context->mouse_state &= ~pangolin::KeyModifierCtrl;
                    }
                    break;
                case XK_Alt_L:
                case XK_Alt_R:
                    key = -1;
                    if(ev.type==KeyPress) {
                        pangolin::context->mouse_state |=  pangolin::KeyModifierAlt;
                    }else{
                        pangolin::context->mouse_state &= ~pangolin::KeyModifierAlt;
                    }
                    break;
                case XK_Super_L:
                case XK_Super_R:
                    key = -1;
                    if(ev.type==KeyPress) {
                        pangolin::context->mouse_state |=  pangolin::KeyModifierCmd;
                    }else{
                        pangolin::context->mouse_state &= ~pangolin::KeyModifierCmd;
                    }
                    break;
                default: key = -1; break;
                }
            }else{
                key = ch;
            }

            if(key >=0) {
                if(ev.type == KeyPress) {
                    pangolin::process::Keyboard(key, ev.xkey.x, ev.xkey.y);
                }else{
                    pangolin::process::KeyboardUp(key, ev.xkey.x, ev.xkey.y);
                }
            }

            break;
        }
    }*/
}

void WaylandWindow::SwapBuffers() {
    eglSwapBuffers(egl_display,egl_surface);
    wl_display_roundtrip(wdisplay);
}


WindowInterface& CreateWindowAndBind(std::string window_title, int w, int h, const Params &params)
{
    width=w;
    height=h;
    const std::string display_name = params.Get(PARAM_DISPLAYNAME, std::string());

    std::shared_ptr<WaylandDisplay> newdisplay = std::make_shared<WaylandDisplay>(display_name.empty() ? NULL : display_name.c_str() );
    if (!newdisplay) {
        throw std::runtime_error("Pangolin Wayland: Failed to open display");
    }

    window_mutex.lock();
    std::shared_ptr<WaylandGlContext> newglcontext = std::make_shared<WaylandGlContext>(
        newdisplay, global_gl_context.lock()
    );

    if(!global_gl_context.lock()) {
        global_gl_context = newglcontext;
    }
    window_mutex.unlock();

    WaylandWindow* win = new WaylandWindow(window_title, w, h, newdisplay);
    win->eglcontext = newglcontext;
    fprintf(stderr, "Created Wayland window\n");
    // Add to context map
    AddNewContext(window_title, std::shared_ptr<PangolinGl>(win) );
    BindToContext(window_title);
    // Process window events
    context->ProcessEvents();

#ifndef HAVE_GLES_2
    fprintf(stderr, "Don't HAVE_GLES_2\n");
#endif

#ifndef HAVE_GLES
    fprintf(stderr, "Don't HAVE_GLES\n");
#endif

    return *context;
}


}

