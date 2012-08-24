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

#include <message_logger.h>

android_LogPriority level_type_convertor(message_logger::message_level level)
{
    android_LogPriority priority = ANDROID_LOG_UNKNOWN;

    switch(level) {
        case message_logger::info :
            priority = ANDROID_LOG_INFO;
            break;

        case message_logger::warning:
            priority = ANDROID_LOG_WARN;
            break;

        case message_logger::error:
            priority = ANDROID_LOG_ERROR;
            break;
    }
    return priority;
}

message_logger::message_logger(const std::string& default_tag) :
    m_default_tag(default_tag)
{
}

message_logger::~message_logger()
{
}

//void message_logger::set_tag(const std::string& new_tag)
//{
//    m_default_tag = new_tag;
//}

//const std::string& message_logger::get_tag() const
//{
//    return m_default_tag;
//}

void message_logger::log(const std::string& message, message_level level) const
{
    log(message, level, m_default_tag);
}

void message_logger::log(const std::string& message, message_level level, const std::string& tag) const
{
    __android_log_print(level_type_convertor(level), tag.c_str(), message.c_str());
}

void message_logger::log_info(const std::string& message) const
{
    log(message, info, m_default_tag);
}

void message_logger::log_warning(const std::string& message) const
{
    log(message, warning, m_default_tag);
}

void message_logger::log_error(const std::string& message) const
{
    log(message, error, m_default_tag);
}
