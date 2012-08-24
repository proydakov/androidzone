/*
 *  Copyright (c) 2012 Evgeny Proydakov <lord.tiran@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include <stdexcept>

#include <EGL/egl.h>
#include <android_native_app_glue.h>

#include <utils/macro.h>

#include <egl_engine.h>
#include <message_logger.h>

struct egl_engine::d
{
    d() :
        app(NULL),
        logger(NULL),
        display(NULL),
        surface(NULL),
        context(NULL),
        width(0),
        height(0)
    {
    }

    ~d()
    {
        app    = NULL;
        logger = NULL;

        display = NULL;
        surface = NULL;
        context = NULL;

        width  = 0;
        height = 0;
    }

    android_app* app;
    message_logger* logger;

    EGLDisplay   display;
    EGLSurface   surface;
    EGLContext   context;

    EGLint       width;
    EGLint       height;
};

egl_engine::egl_engine(android_app* app, message_logger* logger) :
        m_d(NULL)
{
    m_d = new d;

    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };

    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        std::runtime_error error("Unable to eglMakeCurrent");
        throw error;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    m_d->app = app;
    m_d->logger = logger;
    m_d->display = display;
    m_d->context = context;
    m_d->surface = surface;
    m_d->width  = w;
    m_d->height = h;
}

egl_engine::~egl_engine()
{
    if (m_d->display != EGL_NO_DISPLAY) {

        eglMakeCurrent(m_d->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (m_d->context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_d->display, m_d->context);
        }
        if (m_d->surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_d->display, m_d->surface);
        }
        eglTerminate(m_d->display);
    }

    m_d->display = EGL_NO_DISPLAY;
    m_d->context = EGL_NO_CONTEXT;
    m_d->surface = EGL_NO_SURFACE;

    SAFE_DELETE(m_d);
}

int egl_engine::get_width() const
{
    return m_d->width;
}

int egl_engine::get_height() const
{
    return m_d->height;
}

bool egl_engine::swap_buffers()
{
    return  eglSwapBuffers(m_d->display, m_d->surface);
}
