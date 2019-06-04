#ifndef _XOP_LOG_H
#define _XOP_LOG_H

#include <cstdio>

#define LOG(format, ...)  	\
{								\
    fprintf(stderr, "[DEBUG] [%s:%s:%d] " format "", \
    __FILE__, __FUNCTION__ , __LINE__, ##__VA_ARGS__);     \
}


#endif
