#include <stdio.h>
#include <iostream>
#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include "xfiber.h"


XFiber::XFiber() {
    curr_fiber_ = nullptr;
}

XFiber::~XFiber() {
}

ucontext_t *XFiber::SchedCtx() {
    return &sched_ctx_;
}

void XFiber::AddReadyTask(Fiber *fiber) {
    ready_fibers_.push_back(fiber);
}

void XFiber::AddTask(std::function<void ()> run, size_t stack_size, std::string fiber_name) {
    if (stack_size == 0) {
        stack_size = 131072;
    }
    Fiber *fiber = new Fiber(run, stack_size, fiber_name);
    fiber->SetXFiber(this);
    AddReadyTask(fiber);
}

void XFiber::Dispatch() {
    while (true) {
        if (ready_fibers_.size() > 0) {
            std::deque<Fiber *> ready = std::move(ready_fibers_);
            ready_fibers_.clear();

            for (auto iter = ready.begin(); iter != ready.end(); iter++) {
                Fiber *fiber = *iter;
                curr_fiber_ = fiber;
                assert(swapcontext(SchedCtx(), fiber->Ctx()) == 0);
                curr_fiber_ = nullptr;

                if (fiber->IsFinished()) {
                    delete fiber;
                }
            }
            ready.clear();
        }
    }
}

void XFiber::Yield() {
    assert(curr_fiber_ != nullptr);
    AddReadyTask(curr_fiber_);
    assert(swapcontext(curr_fiber_->Ctx(), SchedCtx()) == 0);
}


Fiber::Fiber(std::function<void ()> run, size_t stack_size, std::string fiber_name) {
    run_ = run;
    fiber_name_ = fiber_name;
    stack_size_ = stack_size;
    stack_ptr_ = new uint8_t[stack_size_];
}

Fiber::~Fiber() {

}

void Fiber::SetXFiber(XFiber *xfiber) {
    assert(getcontext(&ctx_) == 0);

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

