#pragma once
#include <queue>
#include <mutex>
#include <atomic>
#include <windows.h>
#include "Gazdik_Message.h"

class Gazdik_Session {
private:
    std::queue<Gazdik_Message> queue;
    std::mutex                 mtx;
    HANDLE                     eventHandle;
    std::atomic<bool>          hasMessages;  // быстрая проверка без захвата мьютекса

public:
    int sessionId;

    Gazdik_Session(int id);
    ~Gazdik_Session();

    void pushMessage(const Gazdik_Message& msg);
    bool pullMessage(Gazdik_Message& msg);
};
