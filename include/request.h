#pragma once

#include <iostream>
#include <string>
#include "buffer.h"


class TRequest {
    public:
        int status;
        char * path;

        TRequest(TBuffer * rb) {
            path = nullptr;
            if( parse(rb) ) {
                status = 0;
            } else {
                status = -1;
            }
        }
        ~TRequest() {
        }

    bool parse(TBuffer * rb);
};
