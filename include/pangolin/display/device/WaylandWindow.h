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

#pragma once

#include <pangolin/platform.h>
#include <pangolin/display/display_internal.h>

#include <EGL/egl.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <GLES2/gl2.h>
#include <stdexcept>
#include <string>
#include <linux/input.h>


namespace pangolin
{

struct wl_display *wdisplay = NULL;
struct wl_registry *wregistry;
struct wl_compositor *wcompositor = NULL;
struct wl_surface *wsurface;
struct wl_egl_window *egl_window;
struct wl_region *wregion;
struct wl_shell *wshell;
struct wl_shell_surface *wshell_surface;

struct wl_seat *wseat;
struct wl_keyboard *wkeyboard;
struct wl_pointer *pointer;
struct wl_touch *touch;

EGLint w, h, format;
EGLint numConfigs, count;
EGLConfig egl_config;
EGLConfig egl_configs[10];
EGLSurface egl_surface;
EGLContext egl_context;
EGLDisplay egl_display;


    EGLint major, minor, n, size, width, height;
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
    };

    EGLint const attrib_list[] = {
#ifdef HAVE_GLES_2
        EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
        EGL_NONE
    };


bool presedLeft = false;
bool presedRight = false;
bool presedMiddle = false;
wl_fixed_t lastx=0;
wl_fixed_t lasty=0;

static void
pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
    fprintf(stderr, "pointer enter\n");

    lastx=sx/256;
    lasty=sy/256;
/*    struct display *display = data;
    struct wl_buffer *buffer;
    struct wl_cursor *cursor = display->default_cursor;
    struct wl_cursor_image *image;

    if (display->window->fullscreen)
        wl_pointer_set_cursor(pointer, serial, NULL, 0, 0);
    else if (cursor) {
        image = display->default_cursor->images[0];
        buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
            return;
        wl_pointer_set_cursor(pointer, serial,
                      display->cursor_surface,
                      image->hotspot_x,
                      image->hotspot_y);
        wl_surface_attach(display->cursor_surface, buffer, 0, 0);
        wl_surface_damage(display->cursor_surface, 0, 0,
                  image->width, image->height);
        wl_surface_commit(display->cursor_surface);
    }*/
}

static void
pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
    // fprintf(stderr, "pointer leave\n");
}

static void
pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    // fprintf(stderr, "pointer motion \n");
    lastx=sx/256;
    lasty=sy/256;
    if (presedLeft || presedRight || presedMiddle)
        pangolin::process::MouseMotion(lastx, lasty);
    else
        pangolin::process::PassiveMouseMotion(lastx, lasty);
}

static void
pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    // fprintf(stderr, "pointer button\n");
    enum MouseButton ebut;
    if (button == BTN_LEFT )
    {
        if(state == WL_POINTER_BUTTON_STATE_PRESSED)
            presedLeft = true;
        else
            presedLeft = false;
        // fprintf(stderr, "LEFT\n");
        ebut = MouseButtonLeft;
    }
    if (button == BTN_RIGHT)
    {
        if(state == WL_POINTER_BUTTON_STATE_PRESSED)
            presedRight = true;
        else
            presedRight = false;
        // fprintf(stderr, "RIGHT\n");
        ebut = MouseButtonRight;
    }
    if (button == BTN_MIDDLE)
    {
        if(state == WL_POINTER_BUTTON_STATE_PRESSED)
            presedMiddle = true;
        else
            presedMiddle = false;
        // fprintf(stderr, "Middle\n");
        ebut = MouseButtonMiddle;
    }
    pangolin::process::Mouse(ebut, state==WL_POINTER_BUTTON_STATE_PRESSED, lastx, lasty);
    // struct display *display = data;

    // if (!display->window->xdg_toplevel)
    //     return;

    // if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED)
        // zxdg_toplevel_v6_move(display->window->xdg_toplevel, display->seat, serial);
}

static void
pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    // fprintf(stderr, "pointer axis\n");
}

static const struct wl_pointer_listener pointer_listener = {
    pointer_handle_enter,
    pointer_handle_leave,
    pointer_handle_motion,
    pointer_handle_button,
    pointer_handle_axis,
};

static void
touch_handle_down(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
    // struct display *d = (struct display *)data;
    // if (!d->shell)
    //     return;
    // zxdg_toplevel_v6_move(d->window->xdg_toplevel, d->seat, serial);
}

static void
touch_handle_up(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, int32_t id)
{
}

static void
touch_handle_motion(void *data, struct wl_touch *wl_touch, uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
}

static void
touch_handle_frame(void *data, struct wl_touch *wl_touch)
{
}

static void
touch_handle_cancel(void *data, struct wl_touch *wl_touch)
{
}

static const struct wl_touch_listener touch_listener = {
    touch_handle_down,
    touch_handle_up,
    touch_handle_motion,
    touch_handle_frame,
    touch_handle_cancel,
};

static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
    
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface,
                      struct wl_array *keys)
{
    fprintf(stderr, "Keyboard gained focus\n");
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface)
{
    fprintf(stderr, "Keyboard lost focus\n");
}

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    fprintf(stderr, "Key is %d state is %d\n", key, state);
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    fprintf(stderr, "Modifiers depressed %d, latched %d, locked %d, group %d\n",
        mods_depressed, mods_latched, mods_locked, group);
}

static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
};


static void
seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t caps1)
{
    enum wl_seat_capability caps;
    caps = (enum wl_seat_capability)caps1;
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        wkeyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(wkeyboard, &keyboard_listener, NULL);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
        wl_keyboard_destroy(wkeyboard);
        wkeyboard = NULL;
    }
    if (caps & WL_SEAT_CAPABILITY_POINTER) {
        pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointer_listener, NULL);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER)) {
        wl_pointer_destroy(pointer);
        pointer = NULL;
    }
    /*if (caps & WL_SEAT_CAPABILITY_TOUCH) {
        touch = wl_seat_get_touch(seat);
        // wl_touch_set_user_data(touch, NULL);
        wl_touch_add_listener(touch, &touch_listener, NULL);
    } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH)) {
        wl_touch_destroy(touch);
        touch = NULL;
    }*/

}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
};

static void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
    // printf("Got a registry event for %s id %d\n", interface, id);
    if (strcmp(interface, "wl_compositor") == 0) {
        wcompositor = reinterpret_cast<wl_compositor*> (wl_registry_bind(registry, id, &wl_compositor_interface, 1));
    } else if (strcmp(interface, "wl_shell") == 0) {
        wshell = reinterpret_cast<wl_shell*> (wl_registry_bind(registry, id, &wl_shell_interface, 1));
    } else if (strcmp(interface, "wl_seat") == 0) {
        wseat = reinterpret_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1));
        wl_seat_add_listener(wseat, &seat_listener, NULL);
    }
}

static void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
    printf("Got a registry losing event for %d\n", id);
}

static const struct wl_registry_listener wregistry_listener = {
    global_registry_handler,
    global_registry_remover
};

static void
handle_ping(void *data, struct wl_shell_surface *shell_surface,
        uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void
handle_configure(void *data, struct wl_shell_surface *shell_surface,
         uint32_t edges, int32_t width, int32_t height)
{

}

static void
handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
    handle_ping,
    handle_configure,
    handle_popup_done
};

struct WaylandDisplay
{
    WaylandDisplay(const char* name = 0) {


    wdisplay = wl_display_connect(NULL);
    if (wdisplay == NULL) {
    // fprintf(stderr, "Can't connect to display\n");
    throw 107;
    }
    // printf("connected to display\n");

    wregistry = wl_display_get_registry(wdisplay);
    wl_registry_add_listener(wregistry, &wregistry_listener, NULL);

    // wl_display_dispatch(wdisplay);
    wl_display_roundtrip(wdisplay);

    if (wcompositor == NULL || wshell == NULL) {
    // fprintf(stderr, "Can't find compositor or shell\n");
    throw 104;
    }

    wsurface = wl_compositor_create_surface(wcompositor);
    wshell_surface = wl_shell_get_shell_surface(wshell, wsurface);
    wl_shell_surface_add_listener(wshell_surface, &shell_surface_listener, NULL);
    // wl_shell_surface_set_toplevel(wshell_surface);
    wl_shell_surface_set_maximized(wshell_surface, NULL);

        egl_display = eglGetDisplay ((EGLNativeDisplayType) wdisplay);
        if (!egl_display) {
            throw std::runtime_error("Pangolin Wayland: Failed to open display");
        }
        EGLint major, minor, n;
        EGLBoolean result;
        result = eglInitialize (egl_display, &major, &minor);
        // assert (result == EGL_TRUE);

        // result = eglBindAPI (EGL_OPENGL_ES_API);
        result = eglBindAPI (EGL_OPENGL_API);
        // assert (result == EGL_TRUE);

    }

    ~WaylandDisplay() {
        wl_display_disconnect(wdisplay);
    }

    // Owns the display
    ::Display* display;
};

struct WaylandGlContext : public GlContextInterface
{
    WaylandGlContext(std::shared_ptr<WaylandDisplay> &d, std::shared_ptr<WaylandGlContext> shared_context = std::shared_ptr<WaylandGlContext>() );
    ~WaylandGlContext();

    std::shared_ptr<WaylandDisplay> display;

    std::shared_ptr<WaylandGlContext> shared_context;

    // Owns the OpenGl Context
    ::EGLContext eglcontext;
};

struct WaylandWindow : public PangolinGl
{
    WaylandWindow(
        const std::string& title, int width, int height,
        std::shared_ptr<WaylandDisplay>& display
    );

    ~WaylandWindow();

    void ToggleFullscreen() override;

    void Move(int x, int y) override;

    void Resize(unsigned int w, unsigned int h) override;

    void MakeCurrent(EGLContext ctx);

    void MakeCurrent() override;

    void SwapBuffers() override;

    void ProcessEvents() override;

    // References the Wayland display and context.
    std::shared_ptr<WaylandDisplay> display;
    std::shared_ptr<WaylandGlContext> eglcontext;

    // Owns the Wayland Window and Colourmap
    ::Window win;
    ::Colormap cmap;

    Atom delete_message;
};

}
