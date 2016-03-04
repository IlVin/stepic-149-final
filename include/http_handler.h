#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <stddef.h>
#include <ev.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include "buffer.h"
#include "cgi.h"

class HTTPHandler;

#define WIO2HND(wio) ((HTTPHandler*)((char*)wio - offsetof(HTTPHandler, w_io)))

class HTTPHandler {
    public:
        std::string     folder;
        int             sock;

        struct ev_io    w_io;
        struct ev_loop  * loop;

        TBuffer         *rb;
        TBuffer         *wb;

    public:
        HTTPHandler(struct ev_loop * l, int s, std::string f)
            : folder(f)
            , sock(s)
            , loop(l)
        {
            rb = new TBuffer();
            wb = new TBuffer();

            ev_init(wio(), r_cb);
            ev_io_set(wio(), sock, EV_READ);
            ev_io_start(loop, wio());
        };

        ~HTTPHandler() {
            shutdown(sock, SHUT_RDWR);
            close(sock);
            delete rb;
            delete wb;
        }

        struct ev_io * wio() {
            return (struct ev_io *)((char*)this + (offsetof(HTTPHandler, w_io)));
        }

        static void w_cb(struct ev_loop *loop, struct ev_io *w, int revent) {
            TBuffer *wb = WIO2HND(w)->wb;
            int r = wb->snd(w->fd);
            if (r == 0 || r == -1) {
                ev_io_stop(loop, w);
                shutdown(w->fd, SHUT_RDWR);
                close(w->fd);
                delete WIO2HND(w);
            }
            return;
        }

        static void r_cb(struct ev_loop *loop, struct ev_io *w, int revent) {
            ev_io_stop(loop, w);
            HTTPHandler * hndl = WIO2HND(w);
            TBuffer *rb = hndl->rb;
            TBuffer *wb = hndl->wb;
            int r = rb->rcv(w->fd, 1024);

            if (r > 0){
                char ** p = rb->head();
                // No multithreading
                if (strstr(*p, "\n\n") != nullptr || strstr(*p, "\n\r\n") != nullptr) {
                    ev_io_stop(loop, w);
                    TCGI * cgi = new TCGI(hndl->folder);
                    cgi->start(rb, wb);
                    delete cgi;
                    //response
                    ev_init(w, w_cb);
                    ev_io_set(w, w->fd, EV_WRITE);
                } 
                ev_io_start(loop, w);
            }
            return;
        }

};
