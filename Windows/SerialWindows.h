#pragma once

#include "../Serial.h"
#include <windows.h>
#include <cstdlib>

namespace
{
    typedef uint32_t uint32;
    static constexpr int TIMEOUT = 1000;
}

static bool WriteSerialData(HANDLE Handle, const char* Data, uint32 Size)
{
    if(Handle == INVALID_HANDLE_VALUE || !Data || Size == 0)
    {
        return false;
    }

    DWORD feedback;
    if(!WriteFile(Handle, Data, (DWORD)Size, &feedback, 0) || feedback != (DWORD)Size)
    {
        CloseHandle(Handle);
        Handle = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

class SerialPortWindows : public SerialPort
{
private:
    HANDLE Handle;
public:
    SerialPortWindows() :
        Handle(INVALID_HANDLE_VALUE)
        { }

    bool IsOK() const override
    {
        return Handle != INVALID_HANDLE_VALUE;
    }

    bool Connect(const std::string& Port, int Baudrate) override
    {
        Disconnect();

        Handle = CreateFile(Port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if(Handle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        SetCommMask(Handle, EV_RXCHAR);
        SetupComm(Handle, 1500, 1500);

        COMMTIMEOUTS CommTimeOuts;
        CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
        CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
        CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
        CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
        CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;

        if(!SetCommTimeouts(Handle, &CommTimeOuts))
        {
            CloseHandle(Handle);
            Handle = INVALID_HANDLE_VALUE;
            return false;
        }

        DCB ComDCM;

        memset(&ComDCM,0,sizeof(ComDCM));
        ComDCM.DCBlength = sizeof(DCB);
        GetCommState(Handle, &ComDCM);
        ComDCM.BaudRate = DWORD(Baudrate);
        ComDCM.ByteSize = 8;
        ComDCM.Parity = NOPARITY;
        ComDCM.StopBits = ONESTOPBIT;
        ComDCM.fAbortOnError = TRUE;
        ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
        ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
        ComDCM.fBinary = TRUE;
        ComDCM.fParity = FALSE;
        ComDCM.fInX = FALSE;
        ComDCM.fOutX = FALSE;
        ComDCM.XonChar = 0;
        ComDCM.XoffChar = (unsigned char)0xFF;
        ComDCM.fErrorChar = FALSE;
        ComDCM.fNull = FALSE;
        ComDCM.fOutxCtsFlow = FALSE;
        ComDCM.fOutxDsrFlow = FALSE;
        ComDCM.XonLim = 128;
        ComDCM.XoffLim = 128;

        if(!SetCommState(Handle, &ComDCM))
        {
            CloseHandle(Handle);
            Handle = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    void Disconnect() override
    {
        if(Handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(Handle);
            Handle = INVALID_HANDLE_VALUE;
        }
    }

    bool Write(std::vector<unsigned char>& Data) override
    {
        WriteSerialData(Handle, (const char*)Data.data(), Data.size());
    }

    bool Write(std::vector<unsigned char>&& Data) override
    {
        WriteSerialData(Handle, (const char*)Data.data(), Data.size());
    }

    bool Write(std::string& Data) override
    {
        WriteSerialData(Handle, Data.c_str(), Data.size());
    }

    bool Write(std::string&& Data) override
    {
        WriteSerialData(Handle, Data.c_str(), Data.size());
    }

    bool Write(const char* Data) override
    {
        WriteSerialData(Handle, Data, strlen(Data));
    }

    bool Read(std::vector<unsigned char>& Data) override
    {
        if(Handle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        DWORD begin = GetTickCount();
        DWORD feedback = 0;

        unsigned char* buf = &Data[0];
        DWORD len = (DWORD)Data.size();

        int attempts = 3;
        while(len && (attempts || (GetTickCount() - begin) < (DWORD)TIMEOUT / 3))
        {
            if(attempts) attempts--;

            if(!ReadFile(Handle, buf, len, &feedback, NULL))
            {
                CloseHandle(Handle);
                Handle = INVALID_HANDLE_VALUE;
                return false;
            }

            assert(feedback <= len);
            len -= feedback;
            buf += feedback;

        }

        if(len)
        {
            CloseHandle(Handle);
            Handle = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    bool Read(std::string& Data, uint32_t Size)
    {
        return Read((unsigned char*)&Data[0], Size);
    }

    bool Read(unsigned char* Data, uint32_t Size) override
    {
        if(Handle == INVALID_HANDLE_VALUE || !Data || Size == 0)
        {
            return false;
        }

        DWORD begin = GetTickCount();
        DWORD feedback = 0;

        DWORD len = (DWORD)Size;

        int attempts = 3;
        while(len && (attempts || (GetTickCount() - begin) < (DWORD)TIMEOUT / 3))
        {
            if(attempts) attempts--;

            if(!ReadFile(Handle, Data, len, &feedback, NULL))
            {
                CloseHandle(Handle);
                Handle = INVALID_HANDLE_VALUE;
                return false;
            }

            assert(feedback <= len);
            len -= feedback;
            Data += feedback;
        }

        if(len)
        {
            CloseHandle(Handle);
            Handle = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    ~SerialPortWindows() override
    {
        Disconnect();
    }
};



