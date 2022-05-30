#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <cstring>
#include <memory>

#include "Coroutine.h"
#include "SockCoroutine.h"

using namespace std;


void sigint_action(int sig) {
    cout << "exit..." << std::endl;
    exit(0);    
}

int main() {
    signal(SIGINT, sigint_action);
    Schedule *schedule = Schedule::coroutineManager();
    schedule->CreateCoroutine([&] {
        Server server = Server::ListenTCP(8888);
        while(true) {
            Connection* conn = server.Accept(); //如果Accept不到客户端立刻放弃cpu，单线程是阻塞在这里，多线程会浪费cpu在这，直到Accept到客户端
            schedule->CreateCoroutine([conn] {
                while(true) {
                    char recv_buf[512], write_buf[512] = "+";
                    int n = conn->Read(recv_buf, 512, 2000);
                    if(n <= 0)
                        break;
                    strcat(write_buf, recv_buf);
                    if(conn->Write(write_buf, n + 1, 2000) <= 0)
                        break;
                }
            }, 0, "server");
        }
    });
    schedule->Dispatch();

    return 0;
}
