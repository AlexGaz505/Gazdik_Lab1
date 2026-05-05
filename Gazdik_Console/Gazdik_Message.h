#pragma once
#include <string>
#include "Gazdik_Interfaces.h"

enum Gazdik_MessageTypes {
    MT_CLOSE
};

struct Gazdik_MessageHeader {
    int messageType;
    int to;
};

struct Gazdik_Message {
    Gazdik_MessageHeader header = { 0 };
    std::wstring data; // полезная нагрузка

    Gazdik_Message() = default;
    Gazdik_Message(Gazdik_MessageTypes type, std::wstring payload = L"");

    static void sendMessage(const Gazdik_ISender& sender, int to,
                            Gazdik_MessageTypes type, std::wstring payload = L"");
    static Gazdik_Message receiveMessage(const Gazdik_IReceiver& receiver);
};
