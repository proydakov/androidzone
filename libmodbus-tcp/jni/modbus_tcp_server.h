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

#ifndef _MODBUS_TCP_CLIENT_H_
#define _MODBUS_TCP_CLIENT_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <modbus/modbus.h>

#include <modbus_logger.h>

int modbus_tcp_server_fun(unsigned int port, modbus_logger& logger)
{
    int socket;
    modbus_t* ctx;
    modbus_mapping_t* mb_mapping;

    ctx = modbus_new_tcp("127.0.0.1", port);

    mb_mapping = modbus_mapping_new(500, 500, 500, 500);
    if (mb_mapping == NULL) {
        logger.log(std::string("Failed to allocate the mapping : ") + std::string(modbus_strerror(errno)));
        modbus_free(ctx);
        return -1;
    }

    socket = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &socket);

    while(true) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc;

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            // rc is the query size
            modbus_reply(ctx, query, rc, mb_mapping);
        }
        else if (rc == -1) {
            // Connection closed by the client or error
            break;
        }
    }

    logger.log(std::string("Quit the loop : ") + std::string(modbus_strerror(errno)));

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}

#endif // _MODBUS_TCP_CLIENT_H_
