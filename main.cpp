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
#define DPRINT(fmt, args...) fprintf(stderr, "[D][%s %d] " fmt"\n", __FILE__, __LINE__, ##args);


int main() {
    signal(SIGINT, sigint_action);
    Schedule *schedule = Schedule::coroutineManager();
    schedule->CreateCoroutine([&]{
        Server server = Server::ListenTCP(7000);
        while (true) {
			shared_ptr<Client> conn1 = server.Accept();

            schedule->CreateCoroutine([conn1] {
                while (true) {
                    char recv_buf[512];
                    int n = conn1->Read(recv_buf, 512, 5000);
                    if (n <= 0) {
                        break;
                    }

                 #if 1
                    /*conn2->Write(recv_buf, n);
                    char rsp[1024];
                    int rsp_len = conn2->Read(rsp, 1024);
                    cout << "recv from remote: " << rsp << endl;
                    conn1->Write(rsp, rsp_len);*/
                    if(conn1->Write(recv_buf, n, 1000) <= 0) {
                        break;
                    }
                #else
                    if (conn1->Write("+OK\r\n", 5, 1000) <= 0) {
                        break;
                    }
                #endif
                }
            }, 0, "server");
        }
    });
    
    schedule->Dispatch();

    return 0;
}
