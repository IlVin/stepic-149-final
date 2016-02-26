#pragma once

#include <ev.h>
#include "buffer.h"
#include "cgi.h"
#include "request.h"

class TCGI {
    private:
        std::string folder;

    public:
        TCGI(std::string f): folder(f) {
        }
        ~TCGI() {
        }

        void start(TBuffer * rb, TBuffer * wb);
};
