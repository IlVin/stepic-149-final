#include <string>
#include <cstring>
#include "request.h"
#include "buffer.h"


bool is(char ** p, char ch) {
    if (**p == ch) {
        (*p)++;
        return true;
    }
    return false;
}

bool isnt(char ** p, char ch) {
    if (**p != ch) {
        (*p)++;
        return true;
    }
    return false;
}

bool is(char ** p, std::string ch_str) {
    const char * ch = ch_str.c_str();
    for (;*ch != '\0'; ch++) {
        if (is(p, *ch)) return true;
    }
    return false;
}

bool isnt(char ** p, std::string ch_str) {
    const char * ch = ch_str.c_str();
    for (;*ch != '\0'; ch++) {
        if (**p == *ch) return false;
    }
    (*p)++;
    return true;
}

bool is_token(char ** p, std::string tok_str) {
    const char * tok = tok_str.c_str();
    char * ptr = *p;
    for (;*tok != '\0'; tok++) {
        if (!is(&ptr, *tok)) return false;
    }
    *p = ptr;
    return true;
}

bool skip(char ** p, std::string ch_str) {
    const char * ch = ch_str.c_str();
    bool result = false;
    while (is(p, ch)) result = true;
    return result;
}

bool TRequest::parse(TBuffer * rb){
    char * p = rb->c_str();

    if (is_token(&p, "GET") && skip(&p, " ")) {
        if (is_token(&p, "http://")) while(isnt(&p, '/'));
        if (!is(&p, '/')) return false;
        path = p - 1;
        while(isnt(&p, "? \n\r"));
        *p = '\0';
        return true;
    }
    return false;
};
