/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *
 * This library implements the Modbus protocol.
 * http://libmodbus.org/
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "./../libmodbus-3.1.4/src/config.h"

#include "./../libmodbus-3.1.4/src/modbus.h"
#include "./../libmodbus-3.1.4/src/modbus-rtu.h"
#include "./../libmodbus-3.1.4/src/modbus-private.h"



int xrt_make_socket_non_blocking (modbus_t *ctx) {
  int flags, s;

  flags = fcntl (ctx->s, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (ctx->s, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}



static char dummy[64];

                
 int modbus_data_available(modbus_t *ctx) {
    int nread = 0, nread2 = 0;
    int rc = ioctl(ctx->s, FIONREAD, &nread);
    if (rc < 0) {
        perror("modbus_data_available():ioctl:");
        return 0;
    } else {
        if (nread > 0) {
            /*
            nread2 = recv(ctx->s, dummy, sizeof(dummy), MSG_PEEK | MSG_DONTWAIT);         
            if (nread != nread2) {
                perror("modbus_data_available");
                fprintf(stderr, "[nread:%d/%d]", nread, nread2); fflush(stderr);
                return nread2;
            }
            */
        }
    }
    return nread;
 }

int modbus_purge_comm(modbus_t *ctx) {
    int nread = 1, bytesRead = 0;
    char str[32];
    
    while (nread>0) {
        // usleep(1000*10); // OK
        usleep(500);
        ioctl(ctx->s, FIONREAD, &nread);
        if (nread) {
            if (ctx->debug) {
                fprintf(stderr, "[read:%d]", nread); fflush(stderr);        
            }
        }
        if (nread > 0) {
            for (int i=0; i<nread; i++) {
                if (read(ctx->s, str, 1) < 0) {
                    return bytesRead;
                } else {
                    // printf("[%2x]", (int)str[0]);
                }
                bytesRead++;
            }
        bytesRead = bytesRead;
        }
    }
    return bytesRead;
}




/* Reads the data from a remove device and put that data into an array */
int xrt_modbus_read_registers_send(modbus_t *ctx, int addr, int nb) {
    int rc;
    int req_length;
    uint8_t req[_MIN_REQ_LENGTH];

    if (nb > MODBUS_MAX_READ_REGISTERS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Too many registers requested (%d > %d)\n",
                    nb, MODBUS_MAX_READ_REGISTERS);
        }
        errno = EMBMDATA;
        return -1;
    }

    ctx->lastReqLen = ctx->backend->build_request_basis(ctx, MODBUS_FC_READ_HOLDING_REGISTERS, addr, nb, ctx->lastReq);

    rc = send_msg(ctx, ctx->lastReq, ctx->lastReqLen);
    if (rc > 0) {
    }

    return rc;
}

/*
int offset;
int i;

rc = _modbus_receive_msg(ctx, rsp, MSG_CONFIRMATION);
if (rc == -1)
    return -1;

rc = check_confirmation(ctx, req, rsp, rc);
if (rc == -1)
    return -1;

offset = ctx->backend->header_length;

for (i = 0; i < rc; i++) {
    // shift reg hi_byte to temp OR with lo_byte
    dest[i] = (rsp[offset + 2 + (i << 1)] << 8) | rsp[offset + 3 + (i << 1)];
}
*/

/* Reads the data from a remove device and put that data into an array */
int xrt_modbus_read_registers_recv(modbus_t *ctx, uint16_t *dest) {
    int rc;
    int offset;
    int i;

    rc = _modbus_receive_msg(ctx, ctx->lastRes, MSG_CONFIRMATION);
    if (rc == -1)
        return -1;

    rc = check_confirmation(ctx, ctx->lastReq, ctx->lastRes, rc);
    if (rc == -1)
        return -1;

    offset = ctx->backend->header_length;

    for (i = 0; i < rc; i++) {
        /* shift reg hi_byte to temp OR with lo_byte */
        dest[i] = (ctx->lastRes[offset + 2 + (i << 1)] << 8) | ctx->lastRes[offset + 3 + (i << 1)];
    }

    return rc;
}

int8_t *xrt_modbus_get_last_req(modbus_t *ctx) {
    return (char*)ctx->lastReq;
}
int xrt_modbus_get_last_req_len(modbus_t *ctx) {
    return (int)ctx->lastReqLen;
}
int8_t *xrt_modbus_get_last_res(modbus_t *ctx) {
    return (char*)ctx->lastRes;
}





int xrt_modbus_write_register_send(modbus_t *ctx, int addr, int value) {
    int rc;
    int req_length;

    req_length = ctx->backend->build_request_basis(ctx, MODBUS_FC_WRITE_SINGLE_REGISTER, addr, value, ctx->lastReq);

    rc = send_msg(ctx, ctx->lastReq, req_length);
    if (rc > 0) {
        /*
        uint8_t rsp[_MIN_REQ_LENGTH];

        rc = _modbus_receive_msg(ctx, rsp, MSG_CONFIRMATION);
        if (rc == -1)
            return -1;

        rc = check_confirmation(ctx, req, rsp, rc);
        */
    }

    return rc;
}



/* Reads the data from a remove device and put that data into an array */
int xrt_modbus_write_register_recv(modbus_t *ctx, uint16_t *dest) {
    int rc;
    int offset;
    int i;

    rc = _modbus_receive_msg(ctx, (uint8_t *)ctx->lastRes, MSG_CONFIRMATION);
    if (rc == -1)
        return -1;

    rc = check_confirmation(ctx, ctx->lastReq, ctx->lastRes, rc);
    if (rc == -1)
        return -1;

    for (i = 0; i < rc; i++) {
        dest[i] = ctx->lastRes[i];
    }

    /*
    offset = ctx->backend->header_length;
    for (i = 0; i < rc; i++) {
        // shift reg hi_byte to temp OR with lo_byte
        dest[i] = (ctx->lastRes[offset + 2 + (i << 1)] << 8) | ctx->lastRes[offset + 3 + (i << 1)];
    }
    */

    return rc;
}