#include "Gazdik_ThreadManager.h"
#include <iostream>

std::map<int, Gazdik_ThreadInfo> Gazdik_ThreadManager::workers;
std::mutex                       Gazdik_ThreadManager::mx;

Gazdik_ThreadManager::Gazdik_ThreadManager(int id) : targetId(id) {}

// --- Отправка сообщения в очередь нужного потока ---
void Gazdik_ThreadManager::send(Gazdik_Message& msg) const {
    std::lock_guard<std::mutex> lock(mx);

    int dest = (targetId < 0) ? msg.header.to : targetId;
    auto found = workers.find(dest);

    if (found != workers.end()) {
        found->second.session->pushMessage(msg);
    }
}

// --- Получение сообщения из очереди своего потока ---
void Gazdik_ThreadManager::receive(Gazdik_Message& msg) const {
    std::shared_ptr<Gazdik_Session> target;

    {
        std::lock_guard<std::mutex> lock(mx);
        auto found = workers.find(targetId);
        if (found != workers.end()) {
            target = found->second.session;
        }
    }

    if (target != nullptr) {
        target->pullMessage(msg);
    }
}

// --- Тело рабочего потока ---
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

// --- Создание нового рабочего потока ---
void Gazdik_ThreadManager::createWorker(int id) {
    std::lock_guard<std::mutex> lock(mx);

    Gazdik_ThreadInfo info;
    info.session = std::make_shared<Gazdik_Session>(id);
    info.handle  = CreateThread(
        NULL, 0,
        threadFunc,
        reinterpret_cast<LPVOID>(static_cast<intptr_t>(id)),
        0, NULL
    );

    if (info.handle != NULL) {
        workers.emplace(id, std::move(info));
    }
}

// --- Остановка последнего потока ---
bool Gazdik_ThreadManager::terminateLast() {
    HANDLE hWait = NULL;
    int    tId   = -1;

    {
        std::lock_guard<std::mutex> lock(mx);
        if (!workers.empty()) {
            auto last = std::prev(workers.end());
            tId   = last->first;
            hWait = last->second.handle;
            // НЕ удаляем из workers здесь — send() должен найти сессию
        }
    }

    if (tId == -1) return false;

    // Отправляем MT_CLOSE — send() найдёт сессию в workers
    Gazdik_Message::sendMessage(Gazdik_ThreadManager(), tId, MT_CLOSE);

    // Ждём реального завершения потока
    WaitForSingleObject(hWait, INFINITE);
    CloseHandle(hWait);

    // Только теперь удаляем — поток уже завершился
    {
        std::lock_guard<std::mutex> lock(mx);
        workers.erase(tId);
    }

    return true;
}

// --- Остановка всех потоков ---
void Gazdik_ThreadManager::deinit() {
    while (terminateLast()) {}
}
