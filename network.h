// In a header file, let's say "network.h"
#ifndef NETWORK_H
#define NETWORK_H

typedef char * HostAddr;
typedef char * Method;
typedef char * RequestBody;
typedef char * ResponseBody;
typedef char * Header;
typedef struct{
    HostAddr hostAddr;
    Method method;
}Target;
typedef struct{
    RequestBody body;
    Header header;
}HttpReqeust;

#endif // NETWORK_H
