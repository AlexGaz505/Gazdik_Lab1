#include "Gazdik_Session.h"

Gazdik_Session::Gazdik_Session(int id) : sessionId(id), hasMessages(false) {
    eventHandle = CreateEventW(NULL, TRUE, FALSE, NULL);
}

Gazdik_Session::~Gazdik_Session() {
    CloseHandle(eventHandle);
}

void Gazdik_Session::pushMessage(const Gazdik_Message& msg) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(msg);
        hasMessages = true;
    }
    SetEvent(eventHandle);
}

bool Gazdik_Session::pullMessage(Gazdik_Message& msg) {
    while (true) {
        WaitForSingleObject(eventHandle, INFINITE);

        std::lock_guard<std::mutex> lock(mtx);
        if (!queue.empty()) {
            msg = queue.front();
            queue.pop();
            if (queue.empty()) {
                hasMessages = false;
                ResetEvent(eventHandle);
            }
            return true;
        }
        ResetEvent(eventHandle);
        hasMessages = false;
    }
}
