#include "Gazdik_ThreadManager.h"
#include <iostream>

std::map<int, std::shared_ptr<Gazdik_Session>> Gazdik_ThreadManager::sessions;
std::vector<HANDLE> Gazdik_ThreadManager::threadHandles;
std::vector<int>    Gazdik_ThreadManager::activeIds;
std::mutex          Gazdik_ThreadManager::mx;

Gazdik_ThreadManager::Gazdik_ThreadManager(int id) : targetId(id) {}

void Gazdik_ThreadManager::send(Gazdik_Message& msg) const {
    std::lock_guard<std::mutex> lock(mx);
    int dest = (targetId < 0) ? msg.header.to : targetId;
    if (sessions.count(dest)) {
        sessions[dest]->pushMessage(msg);
    }
}

void Gazdik_ThreadManager::receive(Gazdik_Message& msg) const {
    std::shared_ptr<Gazdik_Session> sess;
    {
        std::lock_guard<std::mutex> lock(mx);
        if (sessions.count(targetId)) {
            sess = sessions[targetId];
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

    for (bool running = true; running;) {
        Gazdik_Message incoming =
            Gazdik_Message::receiveMessage(Gazdik_ThreadManager(id));
        switch (incoming.header.messageType) {
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
    sessions[id] = session;

    HANDLE hThread = CreateThread(
        NULL, 0,
        threadFunc,
        reinterpret_cast<LPVOID>(static_cast<intptr_t>(id)),
        0, NULL
    );

    if (hThread) {
        threadHandles.push_back(hThread);
        activeIds.push_back(id);
    }
}

bool Gazdik_ThreadManager::terminateLast() {
    HANDLE hWait = NULL;
    int tId = -1;

    {
        std::lock_guard<std::mutex> lock(mx);
        if (!activeIds.empty()) {
            tId   = activeIds.back();
            hWait = threadHandles.back();
            activeIds.pop_back();
            threadHandles.pop_back();
        }
    }

    if (tId == -1) return false;

    Gazdik_Message::sendMessage(Gazdik_ThreadManager(), tId, MT_CLOSE);
    WaitForSingleObject(hWait, INFINITE);
    CloseHandle(hWait);

    {
        std::lock_guard<std::mutex> lock(mx);
        sessions.erase(tId);
    }
    return true;
}

void Gazdik_ThreadManager::deinit() {
    while (terminateLast()) {}
}
