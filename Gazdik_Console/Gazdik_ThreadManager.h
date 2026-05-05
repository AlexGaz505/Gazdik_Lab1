#pragma once
#include <windows.h>
#include <map>
#include <memory>
#include <mutex>
#include "Gazdik_Session.h"
#include "Gazdik_Interfaces.h"

// Хранит все данные одного рабочего потока в одном месте.
// Раньше HANDLE и Session хранились в отдельных параллельных
// векторах — это неудобно и легко рассинхронизировать.
// Теперь всё что относится к потоку лежит в одной структуре.
struct Gazdik_ThreadInfo {
    HANDLE                          handle;   // дескриптор потока
    std::shared_ptr<Gazdik_Session> session;  // почтовый ящик потока
};

class Gazdik_ThreadManager : public Gazdik_ISender, public Gazdik_IReceiver {
private:
    int targetId;

    // Один словарь id → ThreadInfo вместо двух параллельных векторов.
    // Порядок создания отслеживаем через отдельный счётчик lastId.
    static std::map<int, Gazdik_ThreadInfo> workers;
    static std::mutex mx;

    static DWORD WINAPI threadFunc(LPVOID param);

public:
    Gazdik_ThreadManager(int id = -1);

    void send(Gazdik_Message& msg) const override;
    void receive(Gazdik_Message& msg) const override;

    static void createWorker(int id);
    static bool terminateLast();
    static void deinit();
};
