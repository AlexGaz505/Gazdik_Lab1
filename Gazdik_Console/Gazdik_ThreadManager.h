#pragma once
#include <windows.h>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include "Gazdik_Session.h"
#include "Gazdik_Interfaces.h"

// ============================================================
// Gazdik_ThreadManager.h
//
// Что это такое?
// ThreadManager — это "начальник почты".
// Он знает все ящики (Sessions), создаёт рабочих (потоки),
// умеет отправлять письма в нужный ящик и получать из своего.
//
// Он реализует ОБА интерфейса:
//   ISender   → умеет ОТПРАВЛЯТЬ сообщения (в нужную Session)
//   IReceiver → умеет ПОЛУЧАТЬ сообщения (из своей Session)
//
// Хитрость: объект ThreadManager(id) означает
//   "я работаю от имени потока id"
// ThreadManager() без аргумента (id = -1) означает
//   "я просто отправляю, смотри в заголовке кому"
//
// Статические члены (static) — одни на всю программу,
// не на каждый объект. Это как общий список сотрудников
// в отделе кадров — один на всю компанию.
// ============================================================

class Gazdik_ThreadManager : public Gazdik_ISender, public Gazdik_IReceiver {
private:
    int targetId;  // id потока, от имени которого работает этот объект

    // --- Статические данные (общие для всех объектов класса) ---
    static std::map<int, std::shared_ptr<Gazdik_Session>> sessions;
    //   map: словарь id → Session ("ящик №3 принадлежит потоку 3")
    //   shared_ptr: умный указатель — сам удалит Session когда она не нужна

    static std::vector<HANDLE> threadHandles;
    //   список HANDLE'ов потоков — нужны чтобы дождаться их завершения

    static std::vector<int> activeIds;
    //   список id живых потоков (в порядке создания)

    static std::mutex mx;
    //   мьютекс для защиты всех статических данных выше

    // Функция, которую выполняет каждый рабочий поток
    // WINAPI — специальное соглашение о вызове для Win32 потоков
    // LPVOID param — единственный аргумент (передаём туда id)
    static DWORD WINAPI threadFunc(LPVOID param);

public:
    // Конструктор: создать "представителя" для потока id
    // Если id = -1, смотрим адрес в заголовке сообщения
    Gazdik_ThreadManager(int id = -1);

    // Реализация ISender: положить msg в ящик нужного потока
    void send(Gazdik_Message& msg) const override;

    // Реализация IReceiver: вынуть msg из ящика потока targetId
    void receive(Gazdik_Message& msg) const override;

    // --- Статические методы управления ---

    // Создать новый рабочий поток с данным id
    static void createWorker(int id);

    // Остановить последний созданный поток (через сообщение MT_CLOSE)
    // Возвращает true если поток был, false если потоков не осталось
    static bool terminateLast();

    // Остановить все потоки (вызывается при завершении программы)
    static void deinit();
};
