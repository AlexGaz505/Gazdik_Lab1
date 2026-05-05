#include "Gazdik_ThreadManager.h"
#include <iostream>

std::map<int, Gazdik_ThreadInfo> Gazdik_ThreadManager::workers;
std::mutex Gazdik_ThreadManager::mx;

Gazdik_ThreadManager::Gazdik_ThreadManager(int id) : targetId(id) {}

void Gazdik_ThreadManager::send(Gazdik_Message& msg) const {
    std::lock_guard<std::mutex> lock(mx);
    int dest = (targetId < 0) ? msg.header.to : targetId;
    auto it = workers.find(dest);
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

    bool canContinue = true;
    while (canContinue) {
        Gazdik_Message msg = Gazdik_Message::receiveMessage(Gazdik_ThreadManager(id));

        switch (msg.header.messageType) {
        case MT_CLOSE:
            canContinue = false;
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

    Gazdik_ThreadInfo info;
    info.session = std::make_shared<Gazdik_Session>(id);

    HANDLE hThread = CreateThread(
        NULL, 0,
        threadFunc,
        reinterpret_cast<LPVOID>(static_cast<intptr_t>(id)),
        0, NULL
    );

    if (hThread) {
        info.handle = hThread;
        workers[id] = std::move(info);
    }
}

bool Gazdik_ThreadManager::terminateLast() {
    HANDLE hWait = NULL;
    int tId = -1;

    {
        std::lock_guard<std::mutex> lock(mx);
        if (!workers.empty()) {
            // map отсортирован по ключу — берём последний (наибольший id)
            auto it = std::prev(workers.end());
            tId  = it->first;
            hWait = it->second.handle;
            workers.erase(it);
        }
    }

    if (hWait && tId != -1) {
        Gazdik_Message::sendMessage(Gazdik_ThreadManager(), tId, MT_CLOSE);
        WaitForSingleObject(hWait, INFINITE);
        CloseHandle(hWait);
        return true;
    }
    return false;
}

void Gazdik_ThreadManager::deinit() {
    while (terminateLast()) {}
}
