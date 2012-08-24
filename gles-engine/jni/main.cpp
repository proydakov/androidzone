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

#include <android_native_app_glue.h>

#include <utils/macro.h>

#include <gles_engine.h>
#include <message_logger.h>

//-----------------------------------------------------------------------------

#define LOGGER_TAG "gles-engine-logger"

//-----------------------------------------------------------------------------

struct application_data
{
    application_data() :
        logger(NULL),
        engine(NULL)
    {
    }

    ~application_data()
    {
        engine = NULL;
        logger = NULL;
    }

    message_logger* logger;
    gles_engine* engine;
};

//-----------------------------------------------------------------------------

static int32_t engine_handle_input(android_app* app, AInputEvent* event)
{
    return 0;
}

static void engine_handle_cmd(android_app* app, int32_t cmd)
{
    application_data* app_data = (application_data*)app->userData;

    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;

        case APP_CMD_INIT_WINDOW:
            app_data->engine->init_display();
            app_data->engine->enable();
            break;

        case APP_CMD_TERM_WINDOW:
            app_data->engine->disable();
            app_data->engine->destroy_display();
            break;

        case APP_CMD_GAINED_FOCUS:
            app_data->logger->log_warning("APP_CMD_GAINED_FOCUS");
            app_data->engine->enable();
            break;

        case APP_CMD_LOST_FOCUS:
            app_data->logger->log_warning("APP_CMD_LOST_FOCUS");
            app_data->engine->disable();
            break;
    }
}

void android_main(android_app* app)
{
    app_dummy();

    application_data app_data;
    app_data.logger = new message_logger(LOGGER_TAG);
    app_data.engine = new gles_engine(app, app_data.logger);

    app_data.logger->log_warning("entered main");

    app->userData = &app_data;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;

    while (true) {
        int ident;
        int events;
        android_poll_source* source;

        while ((ident = ALooper_pollAll(app_data.engine->is_enabled() ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {

            if (source != NULL) {
                source->process(app, source);
            }

            if (app->destroyRequested != 0) {

                SAFE_DELETE(app_data.engine);

                app_data.logger->log_warning("exited main");

                SAFE_DELETE(app_data.logger);

                return;
            }
        }
    }
}
