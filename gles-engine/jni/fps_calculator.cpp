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

#include <boost/date_time.hpp>

#include <utils/macro.h>

#include <fps_calculator.h>

//-----------------------------------------------------------------------------

const long FPS_CALCULATOR_CYCLE_TIME = 1000; //ms

//-----------------------------------------------------------------------------

struct fps_calculator::d
{
    d()
    {
        last_time =  boost::posix_time::microsec_clock::local_time();
        frames = 0;
    }

    ~d()
    {
        frames = 0;
        fps = 0;
    }

    typedef boost::posix_time::ptime ptime;

    ptime last_time;
    long  frames;

    fps_calculator::fps_t fps;
};

fps_calculator::fps_calculator() :
        m_d(NULL)
{
    m_d = new d;
}

fps_calculator::~fps_calculator()
{
    SAFE_DELETE(m_d);
}

fps_calculator::fps_t fps_calculator::calc_fps()
{
    m_d->frames++;

    boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
    long delta = (current_time - m_d->last_time).total_milliseconds();

    if(delta > FPS_CALCULATOR_CYCLE_TIME) {
        m_d->last_time = current_time;
        m_d->fps = m_d->frames * 1000 / delta;
        m_d->frames = 0;
    }

    return m_d->fps;
}

fps_calculator::fps_t fps_calculator::get_fps() const
{
    return m_d->fps;
}
