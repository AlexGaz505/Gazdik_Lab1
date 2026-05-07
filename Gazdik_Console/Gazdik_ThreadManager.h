#pragma once
#include <windows.h>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include "Gazdik_Session.h"
#include "Gazdik_Interfaces.h"

class Gazdik_ThreadManager : public Gazdik_ISender, public Gazdik_IReceiver {
private:
    int targetId;

    static std::map<int, std::shared_ptr<Gazdik_Session>> sessions;
    static std::vector<HANDLE> threadHandles;
    static std::vector<int> activeIds;
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
