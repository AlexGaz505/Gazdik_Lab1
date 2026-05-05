#include "Gazdik_Message.h"

Gazdik_Message::Gazdik_Message(Gazdik_MessageTypes type, std::wstring payload)
    : data(std::move(payload))
{
    header.messageType = type;
    header.to          = -1;
}

void Gazdik_Message::sendMessage(const Gazdik_ISender& sender, int to,
                                  Gazdik_MessageTypes type, std::wstring payload)
{
    Gazdik_Message m;
    m.header.messageType = type;
    m.header.to          = to;
    m.data               = std::move(payload);
    sender.send(m);
}

Gazdik_Message Gazdik_Message::receiveMessage(const Gazdik_IReceiver& receiver) {
    Gazdik_Message m;
    receiver.receive(m);
    return m;
}
