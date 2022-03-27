#include <signal.h>
#include <iostream>
#include <string>

#include "xfiber.h"

using namespace std;


void sigint_action(int sig) {
    std::cout << "exit..." << std::endl;
    exit(0);    
}

int main() {
    signal(SIGINT, sigint_action);

    XFiber xfiber;

    xfiber.AddTask([&]() {
        cout << "hello world 11" << endl;
        cout << "hello world 12" << endl;
        cout << "hello world 13" << endl;
        // xfiber.Yield();
        cout << "hello world 14" << endl;
        cout << "hello world 15" << endl;

    }, 0, "f1");

    xfiber.AddTask([]() {
        cout << "hello world 2" << endl;
    }, 0, "f2");

    
    xfiber.Dispatch();

    return 0;
}
