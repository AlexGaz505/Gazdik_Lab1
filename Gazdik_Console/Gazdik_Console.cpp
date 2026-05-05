#include <iostream>
#include <windows.h>
#include "Gazdik_ThreadManager.h"

// ============================================================
// Gazdik_Console.cpp — точка входа консольного приложения
//
// Что делает эта программа?
// Это "сервер потоков". Она запускается как отдельный процесс
// из C# GUI и управляется через именованные Win32 Events.
//
// Представь: это рабочий цех. C# GUI — это диспетчер.
// Диспетчер звонит в цех (устанавливает Event), говорит:
//   "Открой новый станок" (Start)
//   "Закрой последний станок" (Stop)
//   "Закрывай цех совсем" (Exit)
// Цех отвечает: "Сделано" (Confirm)
//
// Именованные события — это как телефон между процессами.
// Одно и то же имя → один и тот же объект в ядре Windows.
// C# и C++ пишут разные языки, но звонят на один номер.
// ============================================================

int main() {
    // Устанавливаем русскую локаль чтобы cout корректно выводил кириллицу
    setlocale(LC_ALL, "Russian");

    // --------------------------------------------------------
    // Создаём 4 именованных события
    //
    // CreateEventW параметры:
    //   NULL  — атрибуты безопасности (по умолчанию)
    //   FALSE — auto-reset: событие сбрасывается само после
    //           того как WaitForMultipleObjects его поймает
    //           (нам не нужно вручную ResetEvent — само)
    //   FALSE — изначально не сигнальное (не активировано)
    //   L"имя"— имя события в ядре Windows (L = wide string = Unicode)
    //
    // Имена должны СОВПАДАТЬ с теми, что C# пишет в EventWaitHandle!
    // --------------------------------------------------------
    HANDLE startEvt   = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Start");
    HANDLE stopEvt    = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Stop");
    HANDLE exitEvt    = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Exit");
    HANDLE confirmEvt = CreateEventW(NULL, FALSE, FALSE, L"Gazdik_Evt_Confirm");

    // Массив ожидаемых событий (только 3 — confirmEvt мы сами устанавливаем)
    HANDLE waitHandles[] = { startEvt, stopEvt, exitEvt };

    int sessionCounter = 0;  // счётчик для генерации уникальных id потоков
    bool isAlive = true;     // флаг: работаем ли ещё

    std::cout << "Gazdik_Console запущена. Ожидаю команды..." << std::endl;

    // --------------------------------------------------------
    // Главный цикл ожидания команд
    // --------------------------------------------------------
    while (isAlive) {
        // WaitForMultipleObjects — засыпаем до получения ЛЮБОГО из 3 событий
        //   3            — количество событий
        //   waitHandles  — массив событий
        //   FALSE        — ждать ЛЮБОЕ (не все сразу)
        //   INFINITE     — без таймаута
        //
        // Возвращает: WAIT_OBJECT_0 + индекс сработавшего события
        // Вычитаем WAIT_OBJECT_0 → получаем индекс: 0, 1 или 2
        DWORD signal = WaitForMultipleObjects(3, waitHandles, FALSE, INFINITE)
                       - WAIT_OBJECT_0;

        if (signal == 0) {
            // --- Пришёл сигнал START ---
            // C# GUI просит создать новый рабочий поток
            std::cout << "Команда: Start (создаю поток " << sessionCounter << ")" << std::endl;
            Gazdik_ThreadManager::createWorker(sessionCounter);
            sessionCounter++;
            // Подтверждаем C# что всё сделано
            SetEvent(confirmEvt);
        }
        else if (signal == 1) {
            // --- Пришёл сигнал STOP ---
            // C# GUI просит остановить последний поток
            std::cout << "Команда: Stop" << std::endl;
            if (Gazdik_ThreadManager::terminateLast()) {
                // Поток был — уменьшаем счётчик
                sessionCounter--;
            }
            else {
                // Потоков не было вообще — останавливаем и цех
                std::cout << "Нет активных потоков — завершаю работу." << std::endl;
                isAlive = false;
            }
            // В любом случае подтверждаем
            SetEvent(confirmEvt);
        }
        else if (signal == 2) {
            // --- Пришёл сигнал EXIT ---
            // C# GUI закрывается и просит нас тоже закрыться
            std::cout << "Команда: Exit — завершаю работу." << std::endl;
            isAlive = false;
            // confirmEvt установим ПОСЛЕ deinit (см. ниже)
        }
    }

    // --------------------------------------------------------
    // Завершение: корректно останавливаем все оставшиеся потоки
    // --------------------------------------------------------
    Gazdik_ThreadManager::deinit();

    // Говорим C# что мы завершились
    SetEvent(confirmEvt);

    // Освобождаем все handles (возвращаем ресурсы ОС)
    CloseHandle(startEvt);
    CloseHandle(stopEvt);
    CloseHandle(exitEvt);
    CloseHandle(confirmEvt);

    std::cout << "Gazdik_Console завершена." << std::endl;
    return 0;
}
