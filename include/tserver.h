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

#define THREADS 4

class TServer;
#define WIO2SRV(wio) ((TServer*)((char*)wio - offsetof(TServer, w_io)))
#define WSIG2SRV(wsig) ((TServer*)((char*)wsig - offsetof(TServer, w_sig)))

struct thread_ctx {
    int id;
    pthread_t           thread;
    struct ev_loop *    loop;
    std::vector<int>    fd_queue;
    pthread_spinlock_t  spin;

    thread_ctx(int id): id(id) {
        loop = ev_loop_new(0);
    }
    ~thread_ctx() {
        ev_loop_destroy(loop);
    }
};

class TServer {
public:
    std::string host;
    std::string port;
    std::string folder;

    int                 current_thread;
    thread_ctx *        threads[THREADS];

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
        thread_ctx * ctx = (thread_ctx *) arg;
        ev_loop(ctx->loop, 0);
        std::cout << "pthread["<< ctx->id <<"] ended looping" << std::endl;
        return arg;
    }

    static void sig_cb(struct ev_loop *loop, struct ev_signal *w, int revent) {
        ev_signal_stop(loop, w);
        ev_unloop(loop, EVUNLOOP_ALL);
    }

    static void io_accept_cb(struct ev_loop *loop, struct ev_io *w, int revent) {
        TServer * srv = WIO2SRV(w);
        int client_sd = accept(w->fd, NULL, NULL);

        pthread_spin_lock (&(srv->threads[srv->current_thread]->spin));
        (srv->threads[srv->current_thread]->fd_queue).push_back(client_sd);
        pthread_spin_unlock (&(srv->threads[srv->current_thread]->spin));
        (srv->current_thread)++;

        std::cerr << "ACCEPT: client_sd = " << client_sd << "; folder = " << srv->folder << std::endl;
        new HTTPHandler(loop, client_sd, srv->folder);
    }


};


