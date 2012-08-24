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

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

#include <GLES/gl.h>
#include <android_native_app_glue.h>

#include <utils/macro.h>

#include <egl_engine.h>
#include <gles_engine.h>
#include <message_logger.h>
#include <fps_calculator.h>

// ----------------------------------------------------------------------------

const int RENDER_THREAD_CYCLE_WAIT_TIMEOUT = 50;  // ms
const int RENDER_THREAD_STOP_TIMEOUT       = 250; // ms

const int DATA_THREAD_CYCLE_WAIT_TIMEOUT   = 50;  // ms
const int DATA_THREAD_STOP_TIMEOUT         = 250; // ms

const GLfloat ROTATE_DELTA = 0.5;

// ----------------------------------------------------------------------------

struct gles_engine::d
{
    d() :
        app(NULL),
        logger(NULL),
        calculator(NULL),
        display_engine(NULL),
        render_thread(NULL),
        render_thread_animated(false),
        render_thread_worked(false),
        render_thread_initialized(false),
        data_thread_worked(false),
        data_thread_updated(false),
        m_rotate(0)
    {
    }

    ~d()
    {
        app = NULL;
        logger = NULL;
        calculator = NULL;

        display_engine = NULL;

        render_thread = NULL;
        data_thread = NULL;

        render_thread_worked = false;
        render_thread_animated = false;
        render_thread_initialized = false;

        data_thread_worked = false;
        data_thread_updated = false;

        m_rotate = 0;
    }

    // engine data

    android_app*    app;
    message_logger* logger;
    fps_calculator* calculator;

    egl_engine* display_engine;

    boost::thread* render_thread;
    boost::thread* data_thread;

    boost::mutex render_mutex;

    bool render_thread_worked;
    bool render_thread_animated;
    bool render_thread_initialized;

    bool data_thread_worked;
    bool data_thread_updated;

    // user data

    GLfloat m_rotate;
};

//-----------------------------------------------------------------------------

gles_engine::gles_engine(android_app* app, message_logger* logger) :
    m_d(NULL)
{
    m_d = new d;

    m_d->app = app;
    m_d->logger = logger;
    m_d->calculator = new fps_calculator;

    m_d->render_thread_worked = true;
    m_d->render_thread = new class boost::thread(boost::bind(&gles_engine::render, this));

    m_d->data_thread_worked = true;
    m_d->data_thread_updated = true;
    m_d->data_thread = new class boost::thread(boost::bind(&gles_engine::update, this));
}

gles_engine::~gles_engine()
{
    m_d->render_thread_worked = false;
    m_d->render_thread_animated = false;
    m_d->render_thread_initialized = false;

    boost::this_thread::sleep(boost::posix_time::milliseconds(RENDER_THREAD_STOP_TIMEOUT));
    SAFE_DELETE(m_d->render_thread);

    m_d->data_thread_worked = false;
    m_d->data_thread_updated = false;

    boost::this_thread::sleep(boost::posix_time::milliseconds(DATA_THREAD_STOP_TIMEOUT));
    SAFE_DELETE(m_d->data_thread);

    SAFE_DELETE(m_d->calculator);

    SAFE_DELETE(m_d);
}

void gles_engine::init_display()
{
    m_d->render_thread_initialized = true;
}

void gles_engine::destroy_display()
{
    m_d->render_thread_initialized = false;
    destroy_display_impl();
}

void gles_engine::enable()
{
    m_d->render_thread_animated = true;
}

void gles_engine::disable()
{
    m_d->render_thread_animated = false;
}

bool gles_engine::is_enabled()
{
    return m_d->render_thread_animated;
}

void gles_engine::init_display_impl()
{
    if(m_d->display_engine) {
        return;
    }

    m_d->display_engine = new egl_engine(m_d->app, m_d->logger);

    if(m_d->display_engine) {

        int w = m_d->display_engine->get_width();
        int h = m_d->display_engine->get_height();

        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_SMOOTH);

        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrthof(-2, 2, -2 * (GLfloat) h / (GLfloat) w, 2 * (GLfloat) h / (GLfloat) w, -10.0, 10.0);

        glFlush();
    }
}

void gles_engine::destroy_display_impl()
{
    boost::lock_guard<boost::mutex> lock_guard(m_d->render_mutex);
    SAFE_DELETE(m_d->display_engine);
}

void gles_engine::render()
{
    m_d->logger->log_warning("START RENDER THREAD");

    while(m_d->render_thread_worked) {

        if(m_d->render_thread_initialized && m_d->display_engine == 0) {

            boost::lock_guard<boost::mutex> lock_guard(m_d->render_mutex);

            init_display_impl();
        }

        if(m_d->render_thread_animated && m_d->display_engine) {

            boost::lock_guard<boost::mutex> lock_guard(m_d->render_mutex);

            glClearColor(0.0, 0.0, 1.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glRotatef(m_d->m_rotate, 0.0, 0.0, 1.0);

            GLfloat vertices[] = { -1, -1, 0,   1, -1, 0,   -1, 1, 0,   1, 1, 0 };

            glColor4f(1.0, 0.0, 0.0, 1.0);

            glVertexPointer(3, GL_FLOAT, 0, vertices);
            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glFlush();

            fps_calculator::fps_t fps = m_d->calculator->calc_fps();

            std::stringstream stream;
            stream << "render fps : " << fps;

            m_d->logger->log_info(stream.str());

            m_d->display_engine->swap_buffers();
        }
        else {
            m_d->logger->log_info("render sleep");

            boost::this_thread::sleep(boost::posix_time::milliseconds(RENDER_THREAD_CYCLE_WAIT_TIMEOUT));
        }
    }
    destroy_display();

    m_d->logger->log_warning("STOP RENDER THREAD");
}

void gles_engine::update()
{
    m_d->logger->log_warning("START UPDATE THREAD");

    while(m_d->data_thread_worked) {

        if(m_d->data_thread_updated) {

            m_d->logger->log_info("update");

            m_d->m_rotate += ROTATE_DELTA;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(DATA_THREAD_CYCLE_WAIT_TIMEOUT));
    }

    m_d->logger->log_warning("STOP UPDATE THREAD");
}
