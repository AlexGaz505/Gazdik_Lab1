#include "Gazdik_Session.h"

// ============================================================
// Gazdik_Session.cpp — реализация почтового ящика
// ============================================================

// Конструктор: создаём ящик для потока id
// Создаём Win32 Event с параметрами:
//   NULL  — атрибуты по умолчанию
//   TRUE  — manual-reset: звонок НЕ сбрасывается автоматически,
//            мы сами решаем когда его выключить (ResetEvent)
//            Это важно! Если в очереди 3 письма — звонок звонит непрерывно
//            пока все не заберут. AutoReset сбросился бы после первого.
//   FALSE — изначально не звонит (ящик пуст)
//   NULL  — без имени (ящик приватный, не виден другим процессам)
Gazdik_Session::Gazdik_Session(int id) : sessionId(id) {
    eventHandle = CreateEventW(NULL, TRUE, FALSE, NULL);
}

// Деструктор: освобождаем handle от ОС
// Это как вернуть ключ от ящика — важно делать,
// иначе ресурс ОС "утечёт"
Gazdik_Session::~Gazdik_Session() {
    CloseHandle(eventHandle);
}

// pushMessage — положить письмо в ящик
// Вызывается из ЛЮБОГО потока (поэтому нужен мьютекс)
void Gazdik_Session::pushMessage(const Gazdik_Message& msg) {
    // lock_guard — автоматический замок:
    // при входе в блок {} захватывает мьютекс,
    // при выходе — отпускает. Даже если упадёт исключение.
    std::lock_guard<std::mutex> lock(mtx);

    queue.push(msg);        // кладём письмо в очередь
    SetEvent(eventHandle);  // "звоним в звонок" — будим ожидающий поток
}

// pullMessage — вынуть письмо из ящика
// Вызывается только из потока-владельца
// БЛОКИРУЕТ поток пока ящик пуст
bool Gazdik_Session::pullMessage(Gazdik_Message& msg) {
    // Ждём у ящика бесконечно (INFINITE) пока не позвонят
    // Поток в это время НЕ тратит процессор — он "спит"
    WaitForSingleObject(eventHandle, INFINITE);

    std::lock_guard<std::mutex> lock(mtx);  // захватываем замок

    // Проверяем: вдруг событие сработало, но очередь уже пустая
    // (может случиться в хитрых многопоточных ситуациях)
    if (queue.empty()) {
        ResetEvent(eventHandle);  // выключаем звонок
        return false;
    }

    msg = queue.front();  // берём первое письмо
    queue.pop();          // удаляем его из очереди

    // Если больше писем нет — выключаем звонок
    // Если ещё есть — оставляем звонок активным, чтобы
    // поток сразу же вошёл снова и взял следующее
    if (queue.empty()) {
        ResetEvent(eventHandle);
    }

    return true;
}
