#include "sender_thread.h"

#include <iostream>

#include <sys/socket.h>

#include "player_connection.h"

void SenderThread::run() {
    try {
        // while (should_keep_running()) {
        //     Event event = send_queue.pop();
        //     protocol.send_event(event);
        // }
    } catch (...) {}
}

void SenderThread::stop() {
    Thread::stop();
    // ...
}
