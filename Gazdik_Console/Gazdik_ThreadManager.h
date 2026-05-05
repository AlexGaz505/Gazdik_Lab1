#pragma once
#include <windows.h>
#include <map>
#include <memory>
#include <mutex>
#include "Gazdik_Session.h"
#include "Gazdik_Interfaces.h"

// связанные данные потока в одном месте
struct Gazdik_ThreadInfo {
    HANDLE                          handle;
    std::shared_ptr<Gazdik_Session> session;
};

class Gazdik_ThreadManager : public Gazdik_ISender, public Gazdik_IReceiver {
private:
    int targetId;

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
