#include <stdio.h>
#include <iostream>
#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include "xfiber.h"


XFiber::XFiber() {
}

XFiber::~XFiber() {
}

ucontext_t *XFiber::SchedCtx() {
    return &sched_ctx_;
}

void XFiber::AddTask(Fiber *fiber) {
    ready_.push_back(fiber);
}

void XFiber::AddTask(std::function<void ()> run, size_t stack_size, std::string fiber_name) {
    if (stack_size == 0) {
        stack_size = 131072;
    }
    Fiber *fiber = new Fiber(run, stack_size, fiber_name);
    fiber->SetXFiber(this);
    AddTask(fiber);
}

void XFiber::Dispatch() {
    while (true) {
        if (ready_.size() > 0) {
            for (auto iter = ready_.begin(); iter != ready_.end(); iter++) {
                Fiber *fiber = *iter;
                swapcontext((ucontext_t *)SchedCtx(), fiber->Ctx());

                if (fiber->IsFinished()) {
                    delete fiber;
                }
            }
            ready_.clear();
        }
    }
}

// void XFiber::Yield() {
//     xfiber_->AddTask(this);
//     swapcontext(&ctx_, xfiber_->SchedCtx());
// }


Fiber::Fiber(std::function<void ()> run, size_t stack_size, std::string fiber_name) {
    run_ = run;
    fiber_name_ = fiber_name;
    stack_size_ = stack_size;
    stack_ptr_ = new uint8_t[stack_size_];
}

Fiber::~Fiber() {

}

void Fiber::SetXFiber(XFiber *xfiber) {
    getcontext(&ctx_);

    ctx_.uc_stack.ss_sp = stack_ptr_;
    ctx_.uc_stack.ss_size = stack_size_;
    ctx_.uc_link = xfiber->SchedCtx();
    makecontext(&ctx_, (void (*)())Fiber::Start, 1, this);
}

ucontext_t *Fiber::Ctx() {
    return &ctx_;
}

void Fiber::Start(Fiber *fiber) {
    fiber->run_();
    fiber->status_ = FiberStatus::FINISHED;
}

std::string Fiber::Name() {
    return fiber_name_;
}

bool Fiber::IsFinished() {
    return status_ == FiberStatus::FINISHED;
}

