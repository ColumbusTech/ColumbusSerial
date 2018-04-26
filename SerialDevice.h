#pragma once

#if defined(WIN32) || defined(WIN64)
	#include "./Windows/SerialWindows.h"
#elif defined(__linux)
	#include "./Linux/SerialLinux.h"
#endif

struct SerialDevice
{
	static SerialPort* CreateSerialPort()
	{
		#if defined(WIN32) || defined(WIN64)
			return new SerialPortWindows();
		#elif defined(__linux)
			return new SerialPortLinux();
		#endif

		return nullptr;
	}
};









