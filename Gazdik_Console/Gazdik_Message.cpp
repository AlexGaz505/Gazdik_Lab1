#include "Gazdik_Message.h"

// ============================================================
// Gazdik_Message.cpp
//
// Здесь — реализация методов, объявленных в .h файле.
// Правило: в .h пишем ЧТО есть, в .cpp пишем КАК работает.
// ============================================================

// Конструктор с типом:
// Создаём сообщение и записываем в заголовок его тип.
// Например: Gazdik_Message m(MT_CLOSE) — сообщение "закройся"
Gazdik_Message::Gazdik_Message(Gazdik_MessageTypes type) {
    header.messageType = type;
}

// sendMessage — статическая вспомогательная функция.
// Создаёт сообщение, записывает получателя и отправляет через sender.
//
// Пример использования:
//   Gazdik_Message::sendMessage(manager, threadId, MT_CLOSE);
//   → создаст сообщение MT_CLOSE с адресом threadId
//   → и передаст в manager.send(msg)
void Gazdik_Message::sendMessage(const Gazdik_ISender& sender, int to, Gazdik_MessageTypes type) {
    Gazdik_Message m(type);   // создали сообщение нужного типа
    m.header.to = to;         // записали получателя
    sender.send(m);           // отправили (вызов виртуального метода)
}

// receiveMessage — получить сообщение через receiver.
// Блокирует поток до тех пор, пока сообщение не придёт.
//
// Пример использования:
//   Gazdik_Message msg = Gazdik_Message::receiveMessage(manager);
//   → вызовет manager.receive(msg) и вернёт заполненное сообщение
Gazdik_Message Gazdik_Message::receiveMessage(const Gazdik_IReceiver& receiver) {
    Gazdik_Message m;         // пустое сообщение-контейнер
    receiver.receive(m);      // receiver заполнит его
    return m;
}
