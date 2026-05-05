#include "Gazdik_ThreadManager.h"
#include <iostream>

std::map<int, Gazdik_ThreadInfo> Gazdik_ThreadManager::workers;
std::mutex                       Gazdik_ThreadManager::mx;

Gazdik_ThreadManager::Gazdik_ThreadManager(int id) : targetId(id) {}

void Gazdik_ThreadManager::send(Gazdik_Message& msg) const {
    std::lock_guard<std::mutex> lock(mx);
    int dest = (targetId < 0) ? msg.header.to : targetId;
    auto it  = workers.find(dest);
    if (it != workers.end()) {
        it->second.session->pushMessage(msg);
    }
}

void Gazdik_ThreadManager::receive(Gazdik_Message& msg) const {
    std::shared_ptr<Gazdik_Session> sess;
    {
        std::lock_guard<std::mutex> lock(mx);
        auto it = workers.find(targetId);
        if (it != workers.end()) {
            sess = it->second.session;
        }
    }
    if (sess) {
        sess->pullMessage(msg);
    }
}

DWORD WINAPI Gazdik_ThreadManager::threadFunc(LPVOID param) {
    int id = static_cast<int>(reinterpret_cast<intptr_t>(param));

    {
        std::lock_guard<std::mutex> lock(mx);
        std::cout << "Поток " << id << " запущен" << std::endl;
    }

    bool running = true;
    while (running) {
        Gazdik_Message msg = Gazdik_Message::receiveMessage(Gazdik_ThreadManager(id));

        switch (msg.header.messageType) {
        case MT_CLOSE:
            running = false;
            break;
        default:
            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(mx);
        std::cout << "Поток " << id << " остановлен" << std::endl;
    }

    return 0;
}

void Gazdik_ThreadManager::createWorker(int id) {
    std::lock_guard<std::mutex> lock(mx);

    auto session = std::make_shared<Gazdik_Session>(id);
    HANDLE hThread = CreateThread(
        NULL, 0,
        threadFunc,
        reinterpret_cast<LPVOID>(static_cast<intptr_t>(id)),
        0, NULL
    );

    if (hThread) {
        workers.emplace(id, Gazdik_ThreadInfo{ hThread, session });
    }
}

bool Gazdik_ThreadManager::terminateLast() {
    HANDLE hWait = NULL;
    int    tId   = -1;

    {
        std::lock_guard<std::mutex> lock(mx);
        if (!workers.empty()) {
            auto last = std::prev(workers.end());
            tId   = last->first;
            hWait = last->second.handle;
            workers.erase(last);
        }
    }

    if (tId == -1) return false;

    Gazdik_Message::sendMessage(Gazdik_ThreadManager(), tId, MT_CLOSE);
    WaitForSingleObject(hWait, INFINITE);
    CloseHandle(hWait);
    return true;
}

void Gazdik_ThreadManager::deinit() {
    while (terminateLast()) {}
}
