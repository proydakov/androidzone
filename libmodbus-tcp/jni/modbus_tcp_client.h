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

#ifndef _MODBUS_TCP_SERVER_H_
#define _MODBUS_TCP_SERVER_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <modbus/modbus.h>

#include <modbus_logger.h>

const int LOOP          = 1;
const int ADDRESS_START = 0;
const int ADDRESS_END   = 99;

int modbus_tcp_client_fun(const std::string& host, unsigned int port, modbus_logger& logger)
{
    modbus_t* ctx = modbus_new_tcp(host.c_str(), port);

    if (modbus_connect(ctx) == -1) {
        logger.log(std::string("Connection failed :") + std::string(modbus_strerror(errno)));
        modbus_free(ctx);
        return -1;
     }

     int nb = ADDRESS_END - ADDRESS_START;

     uint8_t* tab_rq_bits = new uint8_t[nb];
     memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

     uint8_t* tab_rp_bits = new uint8_t[nb];
     memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

     uint16_t* tab_rq_registers = new uint16_t[nb];
     memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

     uint16_t* tab_rp_registers = new uint16_t[nb];
     memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

     uint16_t* tab_rw_rq_registers = new uint16_t[nb];
     memset(tab_rw_rq_registers, 0, nb * sizeof(uint16_t));

     int rc = 0;
     int addr = 0;
     int nb_fail = 0;
     int nb_loop = 0;

     while (nb_loop++ < LOOP) {
         for (addr = ADDRESS_START; addr <= ADDRESS_END; addr++) {

             for (int i = 0; i < nb; i++) {
                 tab_rq_registers[i] = (uint16_t) (65535.0 * rand() / (RAND_MAX + 1.0));
                 tab_rw_rq_registers[i] = ~tab_rq_registers[i];
                 tab_rq_bits[i] = tab_rq_registers[i] % 2;
             }

             nb = ADDRESS_END - addr;

             // BIT
             rc = modbus_write_bit(ctx, addr, tab_rq_bits[0]);
             if (rc != 1) {
                 logger.log("BIT TEST : write error");
                 nb_fail++;
             }
             else {
                 rc = modbus_read_bits(ctx, addr, 1, tab_rp_bits);
                 if (rc != 1 || tab_rq_bits[0] != tab_rp_bits[0]) {
                     logger.log("BIT TEST : read error");
                     nb_fail++;
                 }
             }

             // MULTIPLE BITS
             rc = modbus_write_bits(ctx, addr, nb, tab_rq_bits);
             if (rc != nb) {
                 logger.log("MULTIPLE BITS TEST : write error");
                 nb_fail++;
             }
             else {
                 rc = modbus_read_bits(ctx, addr, nb, tab_rp_bits);
                 if (rc != nb) {
                     logger.log("MULTIPLE BITS TEST : read error");
                     nb_fail++;
                 }
                 else {
                     for (int i = 0; i < nb; i++) {
                         if (tab_rp_bits[i] != tab_rq_bits[i]) {
                             logger.log("MULTIPLE BITS TEST : compare error");
                             nb_fail++;
                         }
                     }
                 }
             }

             // SINGLE REGISTER
             rc = modbus_write_register(ctx, addr, tab_rq_registers[0]);
             if (rc != 1) {
                 logger.log("SINGLE REGISTER TEST : write error");
                 nb_fail++;
             }
             else {
                 rc = modbus_read_registers(ctx, addr, 1, tab_rp_registers);
                 if (rc != 1) {
                     logger.log("SINGLE REGISTER TEST : read error");
                     nb_fail++;
                 }
                 else {
                     for (int i = 0; i < nb; i++) {
                         if (tab_rp_bits[i] != tab_rq_bits[i]) {
                             logger.log("SINGLE REGISTER TEST : compare error");
                             nb_fail++;
                         }
                     }
                 }
             }

             // MULTIPLE REGISTERS
             rc = modbus_write_registers(ctx, addr, nb, tab_rq_registers);
             if (rc != nb) {
                 logger.log("MULTIPLE REGISTER TEST : write error");
                 nb_fail++;
             }
             else {
                 rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
                 if (rc != nb) {
                     logger.log("MULTIPLE REGISTER TEST : read error");
                     nb_fail++;
                 }
                 else {
                     for (int i = 0; i < nb; i++) {
                         if (tab_rp_bits[i] != tab_rq_bits[i]) {
                             logger.log("MULTIPLE REGISTER TEST : compare error");
                             nb_fail++;
                         }
                     }
                 }
             }

             // R/W MULTIPLE REGISTERS
             rc = modbus_write_and_read_registers(ctx,
                                                  addr, nb, tab_rw_rq_registers,
                                                  addr, nb, tab_rp_registers);
             if (rc != nb) {
                 logger.log("R/W MULTIPLE REGISTER TEST : write error");
                 nb_fail++;
             }
             else {
                 for (int i = 0; i < nb; i++) {
                     if (tab_rp_registers[i] != tab_rw_rq_registers[i]) {
                         logger.log("R/W MULTIPLE REGISTER TEST : compare error");
                         nb_fail++;
                     }
                 }

                 rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
                 if (rc != nb) {
                     logger.log("R/W MULTIPLE REGISTER TEST : read error");
                     nb_fail++;
                 }
                 else {
                     for (int i = 0; i < nb; i++) {
                         if (tab_rw_rq_registers[i] != tab_rp_registers[i]) {
                             logger.log("R/W MULTIPLE REGISTER TEST : compare error");
                             nb_fail++;
                         }
                     }
                 }
             }
         }
     }

     delete [] tab_rq_bits;
     delete [] tab_rp_bits;
     delete [] tab_rq_registers;
     delete [] tab_rp_registers;
     delete [] tab_rw_rq_registers;

     modbus_close(ctx);
     modbus_free(ctx);

     return nb_fail;
}

#endif // _MODBUS_TCP_SERVER_H_
