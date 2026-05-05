#include <iostream>
#include <windows.h>
#include "Gazdik_ThreadManager.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF-8");

    HANDLE startEvt   = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Start");
    HANDLE stopEvt    = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Stop");
    HANDLE exitEvt    = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Exit");
    HANDLE confirmEvt = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Confirm");

    HANDLE waitHandles[] = { startEvt, stopEvt, exitEvt };

    int  sessionCounter = 0;
    bool isAlive        = true;

    std::cout << "Gazdik_Console запущена" << std::endl;

    while (isAlive) {
        DWORD idx = WaitForMultipleObjects(3, waitHandles, FALSE, INFINITE)
                    - WAIT_OBJECT_0;

        switch (idx) {
        case 0:
            Gazdik_ThreadManager::createWorker(sessionCounter++);
            SetEvent(confirmEvt);
            break;
        case 1:
            if (!Gazdik_ThreadManager::terminateLast()) {
                isAlive = false;
            }
            SetEvent(confirmEvt);
            break;
        case 2:
            isAlive = false;
            break;
        }
    }

    Gazdik_ThreadManager::deinit();
    SetEvent(confirmEvt);

    CloseHandle(startEvt);
    CloseHandle(stopEvt);
    CloseHandle(exitEvt);
    CloseHandle(confirmEvt);

    return 0;
}
