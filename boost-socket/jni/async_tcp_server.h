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

#ifndef _ASYNC_TCP_SERVER_H_
#define _ASYNC_TCP_SERVER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#define UNUSED(x) (void)x;

typedef std::string (*p_make_string)();

class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;
    typedef boost::asio::ip::tcp::socket      socket;

    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new tcp_connection(io_service));
    }

    socket& get_socket()
    {
        return m_socket;
    }

    void start(p_make_string fun)
    {
        m_message = fun();

        boost::asio::async_write(m_socket, boost::asio::buffer(m_message),
            boost::bind(&tcp_connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

private:
    tcp_connection(boost::asio::io_service& io_service)
        : m_socket(io_service)
    {
    }

    void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
    {
    }

private:
    socket      m_socket;
    std::string m_message;
};

class async_tcp_server
{
public:
    typedef boost::asio::ip::tcp::endpoint endpoint;

    async_tcp_server(boost::asio::io_service& io_service, unsigned short port, p_make_string fun) :
        m_acceptor(io_service, endpoint(boost::asio::ip::tcp::v4(), port)),
        m_port(port),
        m_fun(fun)
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection = tcp_connection::create(m_acceptor.get_io_service());

        m_acceptor.async_accept(new_connection->get_socket(),
            boost::bind(&async_tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
    {
        if (!error) {
            new_connection->start(m_fun);
            new_connection->get_socket().close();
        }
        start_accept();
    }

private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    unsigned short m_port;
    p_make_string  m_fun;
};

#endif // _ASYNC_TCP_SERVER_H_
