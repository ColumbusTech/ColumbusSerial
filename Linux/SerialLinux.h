#pragma once

#include "../Serial.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

namespace
{
	static bool WriteSerialData(int TTY, const char* Data, uint32_t Size)
	{
		if (TTY < 0)
		{
			return false;
		}

		uint32_t N = write(TTY, Data, Size);
		return N != -1;
	}

	static bool ReadSerialData(int TTY, char* Data, uint32_t Size)
	{
		if (TTY < 0 || Data == NULL || Size == 0)
		{
			return false;
		}

		uint32_t N = read(TTY, Data, Size);
		return N != -1;
	}
}

class SerialPortLinux : public SerialPort
{
private:
	int TTY;
public:
	SerialPortLinux() :
		TTY(-1)
		{ }

	bool IsOK() const override { return TTY >= 0; }

	bool Connect(std::string Port, int Baudrate) override
	{
		Disconnect();
		TTY = open(Port.c_str(), O_RDWR| O_NONBLOCK | O_NDELAY);
		if (TTY < 0)
		{
			return false;
		}

		termios Config;
		memset(&Config, 0, sizeof(Config));

		if (tcgetattr(TTY, &Config) != 0)
		{
			return false;
		}

		cfsetospeed(&Config, B9600);
		cfsetispeed(&Config, B9600);

		Config.c_cflag &= ~PARENB;
		Config.c_cflag &= ~CSTOPB;
		Config.c_cflag &= ~CSIZE;
		Config.c_cflag |= CS8;
		Config.c_cflag &= ~CRTSCTS;
		Config.c_lflag = 0;
		Config.c_oflag = 0;
		Config.c_cc[VMIN] = 0;
		Config.c_cc[VTIME] = 5;

		Config.c_cflag |= CREAD | CLOCAL;
		Config.c_iflag &= ~(IXON | IXOFF | IXANY);
		Config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		Config.c_oflag &= ~OPOST;

		tcflush(TTY, TCIFLUSH);

		if (tcsetattr(TTY, TCSANOW, &Config) != 0)
		{
			return false;
		}

		return true;
	}

	void Disconnect() override
	{
		if (TTY >= 0)
		{
			close(TTY);
			TTY = -1;
		}
	}

	bool Write(std::vector<unsigned char>& Data) override
	{
		return WriteSerialData(TTY, (const char*)Data.data(), Data.size());
	}

	bool Write(std::vector<unsigned char>&& Data) override
	{
		return WriteSerialData(TTY, (const char*)Data.data(), Data.size());
	}

	bool Write(std::string& Data) override
	{
		return WriteSerialData(TTY, Data.c_str(), Data.size());
	}

	bool Write(std::string&& Data) override
	{
		return WriteSerialData(TTY, Data.c_str(), Data.size());
	}

	bool Write(const char* Data, uint32_t Size) override
	{
		return WriteSerialData(TTY, Data, Size);
	}

	bool Read(std::vector<unsigned char>& Data) override
	{
		if (Data.size() == 0)
		{
			return false;
		}

		return ReadSerialData(TTY, (char*)&Data[0], Data.size());
	}

	bool Read(std::string& Data, uint32_t Size) override
	{
		if (Data.size() == 0)
		{
			return false;
		}

		return ReadSerialData(TTY, &Data[0], Data.size());
	}

	bool Read(char* Data, uint32_t Size) override
	{
		return ReadSerialData(TTY, Data, Size);
	}

	~SerialPortLinux() override { Disconnect(); }
};


















