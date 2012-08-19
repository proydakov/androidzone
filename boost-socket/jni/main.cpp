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

#include <async_tcp_server.h>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

//-----------------------------------------------------------------------------

#define APP_STRING_NAME "boost-socket"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, APP_STRING_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, APP_STRING_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_WARN, APP_STRING_NAME, __VA_ARGS__))

static const std::string HOST = std::string("127.0.0.1");
static const int PORT         = 41007;

static const int SERVER_START_TIMEOUT = 450;
static const int SERVER_STOP_TIMEOUT  = 350;
static const int CLIENT_CYCLE_TIMEOUT = 100;

//-----------------------------------------------------------------------------
// server
//-----------------------------------------------------------------------------

std::string make_daytime_string()
{
    std::string time = boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time());
    std::string message(std::string("server time : ") + time);

    return message;
}

static void server_thread_fun(const bool& run, boost::asio::io_service& io_service)
{
    LOGW("boost-socket : start server_thread_fun");

    try {
        async_tcp_server server(io_service, PORT, make_daytime_string);
        io_service.run();
    }
    catch (std::exception& e) {
        LOGE("server_thread_fun : %s", e.what());
    }
    catch (...) {
        LOGE("server_thread_fun : unknown exception");
    }

    LOGW("boost-socket : stop server_thread_fun");
}

//-----------------------------------------------------------------------------
// client
//-----------------------------------------------------------------------------

static void client_cycle_fun(boost::asio::io_service& io_service)
{
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(HOST, boost::lexical_cast<std::string>(PORT));
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;

    boost::asio::ip::tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;

    while (error && endpoint_iterator != end) {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }

    if (error) {
        throw boost::system::system_error(error);
    }

    const size_t size = 128;
    boost::array<char, size> buf;
    memset(&buf, 0, size);

    socket.read_some(boost::asio::buffer(buf), error);

    LOGI("%s", buf.elems);
}

static void client_thread_fun(const bool& run, boost::asio::io_service& io_service)
{
    LOGW("boost-socket : start client_thread_fun");

    try {
        while(run) {
            client_cycle_fun(io_service);
            boost::this_thread::sleep(boost::posix_time::milliseconds(CLIENT_CYCLE_TIMEOUT));
        }
    }
    catch (std::exception& e) {
        LOGE("client_thread_fun : %s", e.what());
    }
    catch (...) {
        LOGE("client_thread_fun : unknown exception");
    }

    LOGW("boost-socket : stop client_thread_fun");
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

    LOGW("boost-socket : entered main");

    state->userData = NULL;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    bool run = true;
    boost::asio::io_service server_io_service;
    boost::thread server_thread(boost::bind(server_thread_fun, boost::cref(run), boost::ref(server_io_service)));

    boost::this_thread::sleep(boost::posix_time::milliseconds(SERVER_START_TIMEOUT));

    boost::asio::io_service client_io_service;
    boost::thread client_thread(boost::bind(client_thread_fun, boost::cref(run), boost::ref(client_io_service)));

    while (true) {
        int ident;
        int events;
        android_poll_source* source;

        while ((ident = ALooper_pollAll(-1, NULL, &events, (void**) &source)) >= 0) {
            if (source != NULL) {
                source->process(state, source);
            }

            if (state->destroyRequested != 0) {

                client_io_service.stop();
                server_io_service.stop();

                run = false;

                boost::this_thread::sleep(boost::posix_time::milliseconds(SERVER_STOP_TIMEOUT));

                LOGW("boost-socket : exited main");
                return;
            }
        }
    }
}
