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

// #define DEBUG_ENABLE
//#define DPRINT(fmt, args...) fprintf(stderr, "[D][%s %d] " fmt"\n", __FILE__, __LINE__, ##args);


int main() {
    signal(SIGINT, sigint_action);
    Schedule *schedule = Schedule::coroutineManager();
    schedule->CreateCoroutine([&]{
        Server server = Server::ListenTCP(7000);
        while (true) {
			shared_ptr<Client> client = server.Accept();
            schedule->CreateCoroutine([client] {
                while (true) {
                    char recv_buf[512];
                    int n = client->Read(recv_buf, 512, 5000);
                    if (n <= 0) {
                        break;
                    }

                    if (client->Write("+OK\r\n", 5, 1000) <= 0) {
                        break;
                    }
                }
            }, 0, "server");
        }
    });
    schedule->Dispatch();

    return 0;
}
