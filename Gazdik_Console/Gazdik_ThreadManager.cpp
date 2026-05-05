#include "Gazdik_ThreadManager.h"
#include <iostream>

// ============================================================
// Gazdik_ThreadManager.cpp — реализация "начальника почты"
// ============================================================

// Инициализация статических членов (обязательно делать в .cpp)
// Это как "выделить место под общий список" — делается один раз
std::map<int, std::shared_ptr<Gazdik_Session>> Gazdik_ThreadManager::sessions;
std::vector<HANDLE> Gazdik_ThreadManager::threadHandles;
std::vector<int> Gazdik_ThreadManager::activeIds;
std::mutex Gazdik_ThreadManager::mx;

// Конструктор — запоминаем, чьим представителем является этот объект
Gazdik_ThreadManager::Gazdik_ThreadManager(int id) : targetId(id) {}

// -----------------------------------------------------------------
// send() — отправить сообщение в нужный ящик
// Реализует Gazdik_ISender
// -----------------------------------------------------------------
void Gazdik_ThreadManager::send(Gazdik_Message& msg) const {
    std::lock_guard<std::mutex> lock(mx);

    // Определяем получателя:
    // Если targetId < 0 — смотрим адрес в заголовке сообщения
    // Если targetId задан — отправляем конкретному потоку
    int dest = (targetId < 0) ? msg.header.to : targetId;

    // Проверяем что такой ящик существует и кладём письмо
    if (sessions.count(dest)) {
        sessions[dest]->pushMessage(msg);
    }
}

// -----------------------------------------------------------------
// receive() — получить сообщение из своего ящика
// Реализует Gazdik_IReceiver
// -----------------------------------------------------------------
void Gazdik_ThreadManager::receive(Gazdik_Message& msg) const {
    std::shared_ptr<Gazdik_Session> sess;

    {
        // Захватываем мьютекс только чтобы взять shared_ptr на Session
        // Потом отпускаем — нельзя держать мьютекс во время ожидания!
        // Иначе получим ДЕДЛОК: поток ждёт письма, держа замок,
        // а отправитель не может положить письмо — замок занят.
        std::lock_guard<std::mutex> lock(mx);
        if (sessions.count(targetId)) {
            sess = sessions[targetId];  // берём shared_ptr
        }
    }  // <-- мьютекс отпускается здесь

    if (sess) {
        // pullMessage может блокировать НАДОЛГО — поэтому мьютекс уже не держим
        sess->pullMessage(msg);
    }
}

// -----------------------------------------------------------------
// threadFunc — функция, которую выполняет каждый рабочий поток
// -----------------------------------------------------------------
DWORD WINAPI Gazdik_ThreadManager::threadFunc(LPVOID param) {
    // Достаём id из аргумента (передали как intptr_t → LPVOID)
    int id = static_cast<int>(reinterpret_cast<intptr_t>(param));

    // Сообщаем о запуске (захватываем mx чтобы cout не перемешивался)
    {
        std::lock_guard<std::mutex> lock(mx);
        std::cout << "Поток " << id << " запущен" << std::endl;
    }

    // Главный цикл потока: ждём сообщений и обрабатываем
    bool canContinue = true;
    while (canContinue) {
        // Создаём временный объект ThreadManager(id) как получателя
        // и блокируемся в receiveMessage пока не придёт письмо
        Gazdik_Message msg = Gazdik_Message::receiveMessage(Gazdik_ThreadManager(id));

        switch (msg.header.messageType) {
        case MT_CLOSE:
            // Получили команду "закройся" — выходим из цикла
            canContinue = false;
            break;

        default:
            // Неизвестный тип — игнорируем (в будущих лабах добавим обработку)
            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(mx);
        std::cout << "Поток " << id << " остановлен" << std::endl;
    }

    return 0;  // поток завершён успешно
}

// -----------------------------------------------------------------
// createWorker — создать и запустить новый рабочий поток
// -----------------------------------------------------------------
void Gazdik_ThreadManager::createWorker(int id) {
    std::lock_guard<std::mutex> lock(mx);

    // Создаём почтовый ящик для этого потока
    auto session = std::make_shared<Gazdik_Session>(id);
    sessions[id] = session;  // регистрируем в словаре

    // Запускаем поток:
    //   NULL         — атрибуты по умолчанию
    //   0            — размер стека по умолчанию
    //   threadFunc   — функция потока
    //   (LPVOID)id   — передаём id как аргумент (через LPVOID)
    //   0            — сразу запустить (не создавать приостановленным)
    //   NULL         — нам не нужен числовой id потока
    HANDLE hThread = CreateThread(
        NULL, 0,
        threadFunc,
        reinterpret_cast<LPVOID>(static_cast<intptr_t>(id)),
        0, NULL
    );

    if (hThread) {
        threadHandles.push_back(hThread);  // сохраняем handle
        activeIds.push_back(id);           // запоминаем id
    }
}

// -----------------------------------------------------------------
// terminateLast — корректно завершить последний запущенный поток
// -----------------------------------------------------------------
bool Gazdik_ThreadManager::terminateLast() {
    HANDLE hWait = NULL;
    int tId = -1;

    {
        std::lock_guard<std::mutex> lock(mx);

        if (!activeIds.empty()) {
            // Берём последний (самый свежий) поток
            tId = activeIds.back();
            hWait = threadHandles.back();
            // Убираем из списков ДО того как мьютекс отпустили
            activeIds.pop_back();
            threadHandles.pop_back();
        }
    }  // <-- ВАЖНО: отпускаем мьютекс ДО WaitForSingleObject
       // Иначе дедлок: мы ждём поток, держа mx,
       // а поток при печати в cout пытается захватить mx — и висим.

    if (hWait && tId != -1) {
        // Отправляем потоку команду "закройся"
        // ThreadManager() без аргумента = id=-1, смотрит в msg.header.to
        Gazdik_Message::sendMessage(Gazdik_ThreadManager(), tId, MT_CLOSE);

        // Ждём пока поток РЕАЛЬНО завершится (не просто получил команду)
        WaitForSingleObject(hWait, INFINITE);
        CloseHandle(hWait);  // освобождаем handle

        // Удаляем ящик
        std::lock_guard<std::mutex> lock(mx);
        sessions.erase(tId);

        return true;  // поток был, завершили
    }

    return false;  // потоков не было
}

// -----------------------------------------------------------------
// deinit — завершить абсолютно все потоки
// Вызывается при выходе из программы
// -----------------------------------------------------------------
void Gazdik_ThreadManager::deinit() {
    // terminateLast() завершает по одному, пока не вернёт false
    while (terminateLast()) {}
}
