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

#include <jni.h>
#include <errno.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <boost/thread.hpp>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define APP_STRING_NAME "boost-thread"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, APP_STRING_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, APP_STRING_NAME, __VA_ARGS__))

static const int TIMEOUT           = 750;
static const int ITERATION         = 50;
static const int NUMBER_OF_THREADS = 7;

void thread_fun(int id)
{
    for(int i = 0; i < ITERATION; ++i) {
        LOGI("boost-thread id %d : %d", id, i);
        boost::this_thread::sleep(boost::posix_time::milliseconds(TIMEOUT));
    }
}

static int32_t engine_handle_input(struct android_app* app,  AInputEvent* event)
{
    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;

        case APP_CMD_INIT_WINDOW:
            break;

        case APP_CMD_TERM_WINDOW:
            break;

        case APP_CMD_GAINED_FOCUS:
            break;

        case APP_CMD_LOST_FOCUS:
            break;
    }
}

void android_main(struct android_app* state)
{
    boost::thread_group group;

    app_dummy();

    LOGW( "boost-thread : entered main" );

    state->userData = &group;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    for(int i = 0; i < NUMBER_OF_THREADS; ++i) {
        group.add_thread(new class boost::thread(boost::bind(&thread_fun, i)));
    }

    //group.join_all();

    while (true) {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollAll(-1, NULL, &events, (void**) &source)) >= 0) {
            if (source != NULL) {
                source->process(state, source);
            }

            if (state->destroyRequested != 0) {
                LOGW( "boost-thread : exited main" );
                return;
            }
        }
    }
}
