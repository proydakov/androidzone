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

#include <android/log.h>
#include <android_native_app_glue.h>

#include <boost/thread.hpp>

#include <modbus_tcp_server.h>
#include <modbus_tcp_client.h>

//-----------------------------------------------------------------------------

#define APP_STRING_NAME "libmodbus-tcp"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, APP_STRING_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, APP_STRING_NAME, __VA_ARGS__))

static const std::string HOST = std::string("127.0.0.1");
static const int PORT         = 41007;

static const int SERVER_START_TIMEOUT = 500;

//-----------------------------------------------------------------------------

void print_log(const std::string& message, const modbus_logger& logger)
{
    string_list log;
    logger.get_log(log);

    LOGW("%s start", message.c_str());

    string_list::const_iterator end_it = log.end();
    for(string_list::const_iterator it = log.begin(); it != end_it; ++it) {
        std::string line(*it);
        LOGI("%s", line.c_str());
    }

    LOGW("%s end", message.c_str());
}

//-----------------------------------------------------------------------------
// server
//-----------------------------------------------------------------------------

void server_thread_fun()
{
    modbus_logger logger;

    int code = modbus_tcp_server_fun(PORT, logger);

    if(code < 0) {
        LOGW("server : start failed");
    }
    else {
        LOGW("server : test OK");
    }

    print_log(std::string("server work log"), logger);
}

//-----------------------------------------------------------------------------
// client
//-----------------------------------------------------------------------------

void client_thread_fun()
{
    modbus_logger logger;

    int code = modbus_tcp_client_fun(HOST, PORT, logger);

    if(code < 0) {
        LOGW("client : start failed");
    }
    else if(code) {
        int error_count = code;
        LOGW("client : test failed count : %d", error_count);
    }
    else {
        LOGW("client : test OK");
    }

    print_log(std::string("client work log"), logger);
}

//-----------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------

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
    app_dummy();

    LOGW("entered main");

    state->userData = NULL;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    boost::thread_group group;

    group.add_thread(new class boost::thread(boost::bind(&server_thread_fun)));

    boost::this_thread::sleep(boost::posix_time::milliseconds(SERVER_START_TIMEOUT));

    group.add_thread(new class boost::thread(boost::bind(&client_thread_fun)));

    //group.join_all();

    while (true) {
        int ident;
        int events;
        android_poll_source* source;

        while ((ident = ALooper_pollAll(-1, NULL, &events, (void**) &source)) >= 0) {
            if (source != NULL) {
                source->process(state, source);
            }

            if (state->destroyRequested != 0) {
                LOGW("exited main");
                return;
            }
        }
    }
}
