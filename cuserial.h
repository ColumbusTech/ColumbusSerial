#ifndef CUSERIAL_H
#define CUSERIAL_H

#if defined(_WIN32)
	#include <windows.h>
	#include <stdlib.h>
	typedef HANDLE __cuserial_native_handle;
#elif defined(__linux__)
	typedef int __cuserial_native_handle;
#endif

#define __CUSERIAL_SET_STATUS(s) if (status != NULL) *status = s;
#define CUSERIAL_STRING_MAX 64

enum cuserial_baudrate_t
{
	CUSERIAL_BAUDRATE_1200 = 1200,
	CUSERIAL_BAUDRATE_2400 = 2400,
	CUSERIAL_BAUDRATE_4800 = 4800,
	CUSERIAL_BAUDRATE_9600 = 9600,
	CUSERIAL_BAUDRATE_19200 = 19200,
	CUSERIAL_BAUDRATE_38400 = 38400,
	CUSERIAL_BAUDRATE_57600 = 57600,
	CUSERIAL_BAUDRATE_115200 = 115200,
};

enum cuserial_status_t
{
	CUSERIAL_NONE,
	CUSERIAL_NULL_ARGUMENT,
	CUSERIAL_INVALID_BAUDRATE,
	CUSERIAL_INVALID_PORT,
	CUSERIAL_OK
};

struct cuserial_t
{
	int baudrate;
	int timeout;
	char port[CUSERIAL_STRING_MAX];
	__cuserial_native_handle _handle;
};

int cuserial_connect(struct cuserial_t* serial, enum cuserial_status_t* status);
void cuserial_disconnect(struct cuserial_t* serial);
int cuserial_write(struct cuserial_t* serial, const void* bytes, int size);
int cuserial_read(struct cuserial_t* serial, void* bytes, int size);

int cuserial_check_baudrate(int baudrate);
int cuserial_check_port(char port[CUSERIAL_STRING_MAX]);

int __cuserial_connect_internal(struct cuserial_t* serial, enum cuserial_status_t* status);


int cuserial_connect(struct cuserial_t* serial, enum cuserial_status_t* status)
{
	__CUSERIAL_SET_STATUS(CUSERIAL_NONE);
	if (serial == NULL)
	{
		__CUSERIAL_SET_STATUS(CUSERIAL_NULL_ARGUMENT);
		return 0;
	}

	if (cuserial_check_baudrate(serial->baudrate) == 0)
	{
		__CUSERIAL_SET_STATUS(CUSERIAL_INVALID_BAUDRATE);
		return 0;
	}

	if (cuserial_check_port(serial->port) == 0)
	{
		__CUSERIAL_SET_STATUS(CUSERIAL_INVALID_PORT);
		return 0;
	}

	if (__cuserial_connect_internal(serial, status) == 0)
	{
		return 0;
	}

	__CUSERIAL_SET_STATUS(CUSERIAL_OK);
	return 1;
}

#if defined(_WIN32)
	int __cuserial_connect_internal(struct cuserial_t* serial, enum cuserial_status_t* status)
	{
		serial->_handle = CreateFile(serial->port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (serial->_handle == INVALID_HANDLE_VALUE)
		{
			return 0;
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
			return 0;
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
			return 0;
		}

		return 1;
	}

	void cuserial_disconnect(struct cuserial_t* serial)
	{
		CloseHandle(serial->_handle);
		serial->_handle = INVALID_HANDLE_VALUE;
	}

	int cuserial_check_baudrate(int baudrate)
	{
		int baud_rates[] =
		{
			1200,
			2400,
			4800,
			9600,
			19200,
			38400,
			57600,
			115200
		};

		int len = sizeof(baud_rates) / sizeof(baud_rates[0]);
		for (int i = 0; i < len; i++)
		{
			if (baudrate == baud_rates[i])
				return 1;
		}

		return 0;
	}

	int cuserial_check_port(char port[CUSERIAL_STRING_MAX])
	{
		return 1;
	}

	int cuserial_write(struct cuserial_t* serial, const void* bytes, int size)
	{
		DWORD written;
		BOOL res = WriteFile(serial->_handle, bytes, size, &written, 0);
		return res == TRUE ? written : 0;
	}

	int cuserial_read(struct cuserial_t* serial, void* bytes, int size)
	{
		DWORD read;
		BOOL res = ReadFile(serial->_handle, bytes, size, &read, 0);
		return res == TRUE ? read : 0;
	}
#endif

#endif /* !CUSERIAL_H */

