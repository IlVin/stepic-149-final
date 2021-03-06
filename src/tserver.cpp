#include "tserver.h"

#include <string>
#include <list>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ev.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>

void TServer::start() {

    sock = socket(PF_INET, SOCK_STREAM, 0);

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(std::stoi(port));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        perror("bind");

    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

    listen(sock, SOMAXCONN);

    ev_init(w_accept(), io_accept_cb);
    ev_io_set(w_accept(), sock, EV_READ);
    ev_io_start(loop, w_accept());

    ev_signal_init(w_signal(), sig_cb, SIGINT);
    ev_signal_start(loop, w_signal());

    // Start threads
    for (int i = 0; i < THREADS; i++) {
        threads[i] = new TCTX(i, folder);
        int result = pthread_create(&(threads[i]->thread), NULL, threadFunc, (void*) threads[i]);
        if(result != 0) {
            perror("Creating thread error");
            exit(1);
        }
    }

    ev_loop(loop, 0);

    // Join threads
    for (int i = 0; i < THREADS; i++) {
        TCTX * ctx = nullptr;
        int result = pthread_join(threads[i]->thread, (void **) &ctx);
        assert(ctx == threads[i]);
        delete threads[i];
    }
    return;
}


