#pragma once

#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class SerialPort
{
private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
public:
	SerialPort();

	bool IsOK() const;

	bool Connect(std::string Port, int Baudrate);
	void Disconnect();

	bool Write(std::vector<unsigned char>& Data);
	bool Write(std::vector<unsigned char>&& Data);
	bool Write(std::string& Data);
	bool Write(std::string&& Data);
	bool Write(const char* Data, uint32_t Size);

	bool Read(std::vector<unsigned char>& Data);
	bool Read(std::string& Data, uint32_t Size);
	bool Read(char* Data, uint32_t Size);

	~SerialPort() = default;
};

#if defined(__linux)
	#include "./Linux/Linux.inl"
#endif






