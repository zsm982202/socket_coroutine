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
    std::cout << "exit..." << std::endl;
    exit(0);    
}

int main() {
    signal(SIGINT, sigint_action);
    Schedule *schedule = Schedule::coroutineManager();
    schedule->CreateCoroutine([&]{
        Server server = Server::ListenTCP(7000);
        while (true) {
			Connection* conn = server.Accept();
            schedule->CreateCoroutine([conn] {
                while (true) {
                    char recv_buf[512];
                    int n = conn->Read(recv_buf, 512, 5000);
                    if (n <= 0) {
                        break;
                    }

                    if (conn->Write("+OK\r\n", 5, 1000) <= 0) {
                        break;
                    }
                }
            }, 0, "server");
        }
    });
    schedule->Dispatch();

    return 0;
}
