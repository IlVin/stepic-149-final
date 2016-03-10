#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <ev.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>

#include <pthread.h>

#include "http_handler.h"
#include "ctx.h"

#define THREADS 16

class TServer;

#define WIO2SRV(wio) ((TServer*)((char*)wio - offsetof(TServer, w_io)))
#define WSIG2SRV(wsig) ((TServer*)((char*)wsig - offsetof(TServer, w_sig)))

class TServer {
public:
    std::string host;
    std::string port;
    std::string folder;

    int                 current_thread;
    TCTX *              threads[THREADS];

    struct ev_io        w_io;
    struct ev_signal    w_sig;

    int                 sock;

    struct ev_loop *    loop;

public:
    struct ev_io * w_accept() {
        return (struct ev_io *)((char*)this + (offsetof(TServer, w_io)));
    }

    struct ev_signal * w_signal() {
        return (struct ev_signal *)((char*)this + (offsetof(TServer, w_sig)));
    }


    TServer(const std::string &h, const std::string &p, const std::string &f)
        : host(h)
        , port(p)
        , folder(f)
        , current_thread(0)
        , loop(EV_DEFAULT)
    {
    }

    ~TServer() {
    }

    void start();

    static void * threadFunc(void *arg) {
        TCTX * ctx = (TCTX *) arg;
        while (true) {
            ctx->step();
        }
        return arg;
    }

    static void sig_cb(struct ev_loop *loop, struct ev_signal *w, int revent) {
        ev_signal_stop(loop, w);
        ev_unloop(loop, EVUNLOOP_ALL);
    }


    static void io_accept_cb(struct ev_loop *loop, struct ev_io *w, int revent) {
        TServer * srv = WIO2SRV(w);
        int client_sd = accept(w->fd, NULL, NULL);
        (srv->threads[srv->current_thread])->accept(client_sd);

        srv->current_thread = (srv->current_thread + 1) % THREADS;

    }


};


