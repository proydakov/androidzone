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

#include <string>
#include <fstream>

#include <android/log.h>
#include <android_native_app_glue.h>

//-----------------------------------------------------------------------------

#define APP_STRING_NAME "std-streams"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  APP_STRING_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  APP_STRING_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, APP_STRING_NAME, __VA_ARGS__))

//-----------------------------------------------------------------------------

bool read_from_file(const std::string& file_name)
{
    std::ifstream stream(file_name.c_str(), std::ios::in);
    if (!stream.is_open()) {
        LOGE("std::ifstream : open error");
        return false;
    }
    std::string line;

    while(true) {
        std::getline(stream, line);
        if (stream.fail()) {
            break;
        }
        LOGI("std::ifstream : read : %s", line.c_str());
    }
    return true;
}

bool write_to_file(const std::string& file_name, const std::string& text)
{
    bool res = false;

    std::ofstream stream(file_name.c_str(), std::ios::out);
    stream.clear();
    if (!stream.fail()) {
        res = true;
        stream << text.c_str() << std::endl;
    }
    else {
        LOGE("std::ofstream : wtire error");
    }
    stream.close();

    return res;
}

//-----------------------------------------------------------------------------

static int32_t engine_handle_input(android_app* app, AInputEvent* event)
{
    return 0;
}

static void engine_handle_cmd(android_app* app, int32_t cmd)
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

void android_main(android_app* state)
{
    app_dummy();

    LOGW("entered main");

    state->userData = NULL;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    // sdcard example

    std::string file("/sdcard/text.txt");
    LOGW("test file : %s", file.c_str());

    write_to_file(file, "simple text");
    read_from_file(file);

    // device memory example
    /// @todo : fix error

    file = std::string("std_text.txt");
    LOGW("test file : %s", file.c_str());

    write_to_file(file, "simple text");
    read_from_file(file);

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
