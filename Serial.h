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

#if defined(WIN32) || defined(WIN64)
	#include "./Windows/Windows.inl"
#elif defined(__linux)
	#include "./Linux/Linux.inl"
#endif

inline SerialPort::SerialPort() : pImpl{ std::make_unique<Impl>() } {}
inline bool SerialPort::IsOK() const { return pImpl->IsOK(); }
inline bool SerialPort::Connect(std::string Port, int Baudrate) { return pImpl->Connect(Port, Baudrate); }
inline void SerialPort::Disconnect() { pImpl->Disconnect(); }

inline bool SerialPort::Write(std::vector<unsigned char>& Data) { return pImpl->Write(Data); }
inline bool SerialPort::Write(std::vector<unsigned char>&& Data) { return pImpl->Write(Data); }
inline bool SerialPort::Write(std::string& Data) { return pImpl->Write(Data); }
inline bool SerialPort::Write(std::string&& Data) { return pImpl->Write(Data); }
inline bool SerialPort::Write(const char* Data, uint32_t Size) { return pImpl->Write(Data, Size); }

inline bool SerialPort::Read(std::vector<unsigned char>& Data) { return pImpl->Read(Data); }
inline bool SerialPort::Read(std::string& Data, uint32_t Size) { return pImpl->Read(Data, Size); }
inline bool SerialPort::Read(char* Data, uint32_t Size) { return pImpl->Read(Data, Size); }






