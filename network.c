#include <stdio.h>
#include "network.h"
Target target;

void setTarget(HostAddr hostAddr, Method method){
    target.hostAddr = hostAddr;
    target.method = method;
}

void setMethodOfTarget(Method method){
    target.method = method;
}
void setHostAddrOfTarget(HostAddr hostAddr){
    target.hostAddr = hostAddr;
}

void initTarget(){
    //target.hostAddr = "http://localhost:8080/a?param1=value1&param2=value2";
    //target.hostAddr = "http://localhost:8080/a";
    //target.hostAddr = "http://43.201.18.187:8000/a";
    target.hostAddr = "http://43.201.18.187:8000/b?a=1&b=3";
    //target.hostAddr = "http://43.201.18.187:8000/e?a=13";
    target.method = "GET";
}
Target getTarget(){
    return target;
}
