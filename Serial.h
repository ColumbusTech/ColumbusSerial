#pragma once


class SerialPort
{
public:
    SerialPort() {}

    virtual bool IsOK() const { return false; }

    virtual bool Connect(const std::string& Port, int Baudrate) { return false; }
    virtual void Disconnect() {}

    virtual bool Write(std::vector<unsigned char>& Data) { return false; }
    virtual bool Write(std::vector<unsigned char>&& Data) { return false; }
    virtual bool Write(std::string& Data) { return false; }
    virtual bool Write(std::string&& Data) { return false; }
    virtual bool Write(const char* Data) { return false; }

    virtual bool Read(std::vector<unsigned char>& Data) { return false; }
    virtual bool Read(std::string& Data) { return false; }

    virtual ~SerialPort() {}
};





