/**
* @file cuserial.h
*/

#ifndef CUSERIAL_H
#define CUSERIAL_H

#if defined(_WIN32)
	#include <windows.h>
	#include <stdlib.h>
	typedef HANDLE __cuserial_native_handle;
#elif defined(__linux__)
	#include <unistd.h>
	#include <fcntl.h>
	#include <termios.h>
#include <string.h>
	typedef int __cuserial_native_handle;
#else
#error Unsupported platform
#endif

#define CUSERIAL_STRING_MAX 64
#define __CUSERIAL_SET_STATUS(s) if (status != NULL) *status = s;

/**
* @brief Baud rates aliases for safety, prefer using this instead of raw integer number.
*/
enum cuserial_baudrate_t
{
	CUSERIAL_BAUDRATE_300 = 300, //!< 300 bauds
	CUSERIAL_BAUDRATE_600 = 600, //!< 600 bauds
	CUSERIAL_BAUDRATE_1200 = 1200, //!< 1200 bauds
	CUSERIAL_BAUDRATE_2400 = 2400, //!< 2400 bauds
	CUSERIAL_BAUDRATE_4800 = 4800, //!< 4800 bauds
	CUSERIAL_BAUDRATE_9600 = 9600, //!< 9600 bauds
	CUSERIAL_BAUDRATE_19200 = 19200, //!< 19200 bauds
	CUSERIAL_BAUDRATE_38400 = 38400, //!< 38400 bauds
	CUSERIAL_BAUDRATE_57600 = 57600, //!< 57600 bauds
	CUSERIAL_BAUDRATE_115200 = 115200, //!< 115200 bauds
};

/**
* @brief Status/error codes
*/
enum cuserial_status_t
{
	CUSERIAL_NONE, //!< Actually, not handled error
	CUSERIAL_NULL_ARGUMENT, //!< If you pass null cuserial_t
	//CUSERIAL_INVALID_BAUDRATE, //!< Baudrate error
	CUSERIAL_INVALID_PORT, //!< Cannot connect to the serial port
	CUSERIAL_OK //!< No error
};


/**
* @brief Serial port connection handle.
* It's self-contained and have all it needs without any global state.
* Configure it **before** calling cuserial_connect().
* Then it will contain information about connection representing connection handle.
*/
struct cuserial_t
{
	int baudrate; //!< Prefer using cuserial_baudrate_t enumerations to not to make a mistake.
	int timeout; //!< Input/output timeout in milliseconds.
	char port[CUSERIAL_STRING_MAX]; //!< Connection file name (e.g. COM1, COM3).
	__cuserial_native_handle _handle; //!< Native handle, better not to use it yourself.
};

/**
* @brief Establish serial port connection.
*
* @param serial Pointer to configured cuserail handle
* @return 0 if success, otherwise -1
*/
int cuserial_connect(struct cuserial_t* serial, enum cuserial_status_t* status);

/**
* @brief Disconnect serial port.
*/
void cuserial_disconnect(struct cuserial_t* serial);

/**
* @brief Write data to serial port.
*
* @param serial Connection handle
* @param bytes Data to send
* @param size Size of data in bytes
* @return Count of sent bytes if succes, otherwise -1
*/
int cuserial_write(struct cuserial_t* serial, const void* bytes, int size);

/**
* @brief Read data from serial port.
*
* @param serial Connection handle
* @param bytes Output bytes ptr, need to be pre-allocated.
* @param size Size of data-to-read in bytes
* @return Count of actually read bytes if success, otherwise -1
*/
int cuserial_read(struct cuserial_t* serial, void* bytes, int size);

/**
* @brief Checks if baudrate is supported (one of the cuserial_baudrate_t enum)
* @return 0 if success, otherwise -1
*/
int cuserial_check_baudrate(int baudrate);

/**
* @brief Checks if specified port is avaliable to connect.
* @return 0 if success, otherwise -1
*/
int cuserial_check_port(char port[CUSERIAL_STRING_MAX]);

/// @private
int __cuserial_connect_internal(struct cuserial_t* serial, enum cuserial_status_t* status);


int cuserial_connect(struct cuserial_t* serial, enum cuserial_status_t* status)
{
	__CUSERIAL_SET_STATUS(CUSERIAL_NONE);
	if (serial == NULL)
	{
		__CUSERIAL_SET_STATUS(CUSERIAL_NULL_ARGUMENT);
		return -1;
	}

	// This check is limiting
	/*if (cuserial_check_baudrate(serial->baudrate))
	{
		__CUSERIAL_SET_STATUS(CUSERIAL_INVALID_BAUDRATE);
		return -1;
	}*/

	if (cuserial_check_port(serial->port))
	{
		__CUSERIAL_SET_STATUS(CUSERIAL_INVALID_PORT);
		return -1;
	}

	if (__cuserial_connect_internal(serial, status))
	{
		return -1;
	}

	__CUSERIAL_SET_STATUS(CUSERIAL_OK);
	return 0;
}

#if defined(_WIN32)
	int __cuserial_connect_internal(struct cuserial_t* serial, enum cuserial_status_t* status)
	{
		serial->_handle = CreateFile(serial->port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (serial->_handle == INVALID_HANDLE_VALUE)
		{
			return -1;
		}

		SetCommMask(serial->_handle, EV_RXCHAR);
		SetupComm(serial->_handle, 1500, 1500);

		COMMTIMEOUTS CommTimeOuts;
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = serial->timeout;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = serial->timeout;

		if (SetCommTimeouts(serial->_handle, &CommTimeOuts) == FALSE)
		{
			CloseHandle(serial->_handle);
			serial->_handle = INVALID_HANDLE_VALUE;
			return -1;
		}

		DCB ComDCM;
		ZeroMemory(&ComDCM, sizeof(DCB));

		ComDCM.DCBlength = sizeof(DCB);
		GetCommState(serial->_handle, &ComDCM);
		ComDCM.BaudRate = (DWORD)serial->baudrate;
		ComDCM.ByteSize = 8;
		ComDCM.Parity = PARITY_NONE;
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

		if (!SetCommState(serial->_handle, &ComDCM))
		{
			CloseHandle(serial->_handle);
			serial->_handle = INVALID_HANDLE_VALUE;
			return -1;
		}

		return 0;
	}

	void cuserial_disconnect(struct cuserial_t* serial)
	{
		CloseHandle(serial->_handle);
		serial->_handle = INVALID_HANDLE_VALUE;
	}

	int cuserial_write(struct cuserial_t* serial, const void* bytes, int size)
	{
		DWORD written;
		BOOL res = WriteFile(serial->_handle, bytes, size, &written, 0);
		return res == TRUE ? written : -1;
	}

	int cuserial_read(struct cuserial_t* serial, void* bytes, int size)
	{
		DWORD read;
		BOOL res = ReadFile(serial->_handle, bytes, size, &read, 0);
		return res == TRUE ? read : -1;
	}

	int cuserial_check_baudrate(int baudrate)
	{
		int baud_rates[] =
		{
			CUSERIAL_BAUDRATE_300,
			CUSERIAL_BAUDRATE_600,
			CUSERIAL_BAUDRATE_1200,
			CUSERIAL_BAUDRATE_2400,
			CUSERIAL_BAUDRATE_4800,
			CUSERIAL_BAUDRATE_9600,
			CUSERIAL_BAUDRATE_19200,
			CUSERIAL_BAUDRATE_38400,
			CUSERIAL_BAUDRATE_57600,
			CUSERIAL_BAUDRATE_115200
		};

		int len = sizeof(baud_rates) / sizeof(baud_rates[0]);
		for (int i = 0; i < len; i++)
		{
			if (baudrate == baud_rates[i])
				return 0;
		}

		return -1;
	}

	int cuserial_check_port(char port[CUSERIAL_STRING_MAX])
	{
		HANDLE tmp = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (tmp == INVALID_HANDLE_VALUE)
			return -1;

		CloseHandle(tmp);
		return 0;
	}

#elif defined(__linux__)
	int __cuserial_convert_baud_internal_unix(int baudrate)
	{
		switch (baudrate)
		{
		case CUSERIAL_BAUDRATE_300: return B300;
		case CUSERIAL_BAUDRATE_600: return B600;
		case CUSERIAL_BAUDRATE_1200: return B1200;
		case CUSERIAL_BAUDRATE_2400: return B2400;
		case CUSERIAL_BAUDRATE_4800: return B4800;
		case CUSERIAL_BAUDRATE_9600: return B9600;
		case CUSERIAL_BAUDRATE_19200: return B19200;
		case CUSERIAL_BAUDRATE_38400: return B38400;
		case CUSERIAL_BAUDRATE_57600: return B57600;
		case CUSERIAL_BAUDRATE_115200: return B115200;
		}

		return -1;
	}

	int __cuserial_connect_internal(struct cuserial_t* serial, enum cuserial_status_t* status)
	{
		serial->_handle = open(serial->port, O_RDWR | O_NONBLOCK | O_NDELAY);
		if (serial->_handle < 0)
			return -1;

		struct termios config;
		memset(&config, 0, sizeof(config));

		if (tcgetattr(serial->_handle, &config) != 0)
		{
			cuserial_disconnect(serial);
			return -1;
		}

		int baudrate = __cuserial_convert_baud_internal_unix(serial->baudrate);

		cfsetospeed(&config, baudrate);
		cfsetispeed(&config, baudrate);

		config.c_cflag &= ~PARENB;
		config.c_cflag &= ~CSTOPB;
		config.c_cflag &= ~CSIZE;
		config.c_cflag |= CS8;
		config.c_cflag &= ~CRTSCTS;
		config.c_lflag = 0;
		config.c_oflag = 0;
		config.c_cc[VMIN] = 0;
		config.c_cc[VTIME] = 5;

		config.c_cflag |= CREAD | CLOCAL;
		config.c_iflag &= ~(IXON | IXOFF | IXANY);
		config.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		config.c_oflag &= ~OPOST;

		tcflush(serial->_handle, TCIFLUSH);

		if (tcsetattr(serial->_handle, TCSANOW, &config) != 0)
		{
			cuserial_disconnect(serial);
			return -1;
		}

		return 0;
	}

	void cuserial_disconnect(struct cuserial_t* serial)
	{
		close(serial->_handle);
		serial->_handle = -1;
	}

	int cuserial_write(struct cuserial_t* serial, const void* bytes, int size)
	{
		if (serial->_handle < 0)
			return -1;

		int written = write(serial->_handle, bytes, size);
		return written;
	}

	int cuserial_read(struct cuserial_t* serial, void* bytes, int size)
	{
		if (serial->_handle < 0)
			return -1;

		int rd = read(serial->_handle, bytes, size);
		return rd;
	}

	int cuserial_check_baudrate(int baudrate)
	{
		return __cuserial_convert_baud_internal_unix(baudrate) == -1 ? -1 : 0;
	}

	int cuserial_check_port(char port[CUSERIAL_STRING_MAX])
	{
		return 0;
	}
#endif

#endif /* !CUSERIAL_H */

