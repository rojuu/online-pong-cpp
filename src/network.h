#pragma once

struct Packet
{
    void* message;
    Uint32 size;
};

struct ServerMessage
{
    int i;
};

struct ClientMessage
{
    Uint32 i;
};
