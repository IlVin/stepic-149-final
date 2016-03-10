#include <iostream>
#include <string>
#include <vector>

#include <pthread.h>
#include <ev.h>
#include <ctime>


struct TCTX;
#define WSIG2CTX(sig) ((TCTX*)((char*)sig - offsetof(TCTX, w_accept_sig)))


struct TCTX {
    int                 id;
    std::string         folder;
    pthread_t           thread;
    struct ev_loop *    loop;

    std::vector<int>    fd_queue;
    pthread_spinlock_t  spin;

    clock_t sleep_time = 50;

    TCTX(int id, std::string folder): id(id), folder(folder) {
        loop = ev_loop_new(EVBACKEND_EPOLL);
        pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE);
    }

    ~TCTX() {
        ev_unloop(loop, EVUNLOOP_ALL);
        ev_loop_destroy(loop);
    }

    void accept(int client_sd) {
        pthread_spin_lock (&spin);
        fd_queue.push_back(client_sd);
        pthread_spin_unlock (&spin);
    }

    void step() {
        clock_t start = std::clock();
        pthread_spin_lock (&spin);
        while (fd_queue.size() > 0) {
            new HTTPHandler(loop, fd_queue.back(), folder);
            fd_queue.pop_back();
        }
        pthread_spin_unlock (&spin);

        ev_loop(loop, EVLOOP_NONBLOCK);
        clock_t diff = std::clock() - start;

        if (diff < sleep_time) usleep (sleep_time - diff);
    }


};

