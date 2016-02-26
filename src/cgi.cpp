#include "cgi.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>

std::string tbad = "400 Bad Request";
std::string t200 = "HTTP/1.0 200 OK\r\nContent-length: %d\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s";
std::string t404 = "HTTP/1.0 404 NOT FOUND\r\nContent-Type: text/html\r\nContent-length: 9\r\nConnection: close\r\n\r\nNOT_FOUND";

void TCGI::start(TBuffer * rb, TBuffer * wb) {
    TRequest * r = new TRequest(rb);
    if (r->status < 0) {
        wb->append(tbad);
    } else {
        std::string fpath = folder + r->path;
        struct stat st;
        int r = stat(fpath.c_str(), &st);
        if (r == -1 || !(st.st_mode & S_IFREG)) {
            wb->append(t404);
        } else {
            std::ifstream * f = new std::ifstream(fpath, std::ios::in );
            std::string content;
            getline(*f, content, '\0');
            f->close();
            std::stringstream ss;
            ss  << "HTTP/1.0 200 OK\r\nContent-length: " << content.size()
                << "\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n" << content;
            wb->append(ss.str());
            delete f;
        }

    }
    delete r;
    return;
}
