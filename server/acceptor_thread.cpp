#include "acceptor_thread.h"

#include <sys/socket.h>


//...

void AcceptorThread::reap() noexcept {
    //...
}

void AcceptorThread::clear() noexcept {
    //...
}

AcceptorThread::~AcceptorThread() {
    clear();
}

void AcceptorThread::run() {
    //...
}

void AcceptorThread::stop() {
    Thread::stop();
    try {
        listener.shutdown(SHUT_RDWR);
        listener.close();
    } catch (const std::exception& e) {
        std::cerr << "[ACCEPTOR] Error stopping: " << e.what() << "\n";
    }
}
