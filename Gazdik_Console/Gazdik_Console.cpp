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

    std::cout << "Gazdik_Console запущена. Ожидаю команды..." << std::endl;

    while (isAlive) {
        DWORD idx = WaitForMultipleObjects(3, waitHandles, FALSE, INFINITE)
                    - WAIT_OBJECT_0;

        switch (idx) {
        case 0: // Start — создать новый поток
            std::cout << "Команда: Start (поток " << sessionCounter << ")" << std::endl;
            Gazdik_ThreadManager::createWorker(sessionCounter++);
            SetEvent(confirmEvt);
            break;

        case 1: // Stop — завершить последний поток
            std::cout << "Команда: Stop" << std::endl;
            if (Gazdik_ThreadManager::terminateLast()) {
                sessionCounter--;
            } else {
                std::cout << "Нет активных потоков — завершаю работу." << std::endl;
                isAlive = false;
            }
            SetEvent(confirmEvt);
            break;

        case 2: // Exit — завершить всё
            std::cout << "Команда: Exit" << std::endl;
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

    std::cout << "Gazdik_Console завершена." << std::endl;
    return 0;
}
