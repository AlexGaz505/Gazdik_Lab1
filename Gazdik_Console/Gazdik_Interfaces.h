#pragma once

struct Gazdik_Message;

class Gazdik_ISender {
public:
    virtual void send(Gazdik_Message& msg) const = 0;
    virtual ~Gazdik_ISender() = default;
};

class Gazdik_IReceiver {
public:
    virtual void receive(Gazdik_Message& msg) const = 0;
    virtual ~Gazdik_IReceiver() = default;
};
