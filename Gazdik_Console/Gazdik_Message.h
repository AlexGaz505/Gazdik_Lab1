#pragma once
#include "Gazdik_Interfaces.h"

// ============================================================
// Gazdik_Message.h
//
// Что это такое?
// Описание "письма" — сообщения, которое потоки передают
// друг другу. Как конверт: снаружи — заголовок (кому, тип),
// внутри — содержимое (пока только тип команды).
//
// Из чего состоит сообщение:
//   [Gazdik_MessageHeader] — шапка (тип + получатель)
//   [Gazdik_Message]       — само сообщение (содержит шапку)
// ============================================================

// Все возможные типы сообщений.
// Пока только одна команда — "закройся".
// В следующих лабораторных здесь добавятся новые типы.
enum Gazdik_MessageTypes {
    MT_CLOSE   // команда потоку: завершить работу
};

// Заголовок сообщения — "адрес на конверте"
struct Gazdik_MessageHeader {
    int messageType;  // тип из enum выше (что делать)
    int to;           // id потока-получателя (кому)
};

// Само сообщение
struct Gazdik_Message {
    Gazdik_MessageHeader header = { 0 };  // заголовок, по умолчанию всё 0

    // Конструктор по умолчанию — пустое сообщение
    Gazdik_Message() = default;

    // Конструктор с типом — создаём сообщение нужного типа
    Gazdik_Message(Gazdik_MessageTypes type);

    // Статический метод — удобная обёртка:
    // "создай сообщение и отправь его через sender потоку to"
    // static = можно вызывать без объекта: Gazdik_Message::sendMessage(...)
    static void sendMessage(const Gazdik_ISender& sender, int to, Gazdik_MessageTypes type);

    // Статический метод — получить сообщение через receiver
    static Gazdik_Message receiveMessage(const Gazdik_IReceiver& receiver);
};
