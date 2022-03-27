#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>

#include <ucontext.h>


typedef enum {
    INIT = 0,
    READYING = 1,
    WAITING = 2,
    FINISHED = 3
}FiberStatus;

class Fiber;
class XFiber {
public:
    XFiber();

    ~XFiber();

    void AddTask(std::function<void()> run, size_t stack_size = 0, std::string fiber_name="");

    void Dispatch();

    ucontext_t *SchedCtx();

private:
    std::vector<Fiber *> ready_;
};


class Fiber
{
public:
    Fiber(std::function<void ()> run, size_t stack_size, std::string fiber_name);

    ~Fiber();

    ucontext_t* Ctx();

    std::string Name();

    bool IsFinished();

    void SetXFiber(XFiber *xfiber);
    
    void Run();

    static void Start(Fiber *fiber);

private:
    XFiber *xfiber_;
    std::string fiber_name_;
    FiberStatus status_;
    ucontext_t ctx_;
    uint8_t *stack_ptr_;
    size_t stack_size_;
    std::function<void ()> run_;
};

