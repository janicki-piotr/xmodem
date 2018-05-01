#pragma once
const char SOH = 0x01;
const char NAK = 0x15;
const char CAN = 0x18;
const char ACK = 0x06;
const char EOT = 0x04;

HANDLE   portHandle;            // Handle for a port
LPCTSTR  portName;              // A pointer to a constant null-terminated string of 8-bit Windows (ANSI) characters, port name
DCB      controlSettings;       // Defines the control setting for a serial communications device.
COMSTAT	 commDeviceInfo;           // Contains information about a communications device. This structure is filled by the ClearCommError function
DWORD    Error;                         	 
COMMTIMEOUTS timeParameters;	// Contains the time-out parameters for a communications device. 
USHORT   tmpCRC;				// CRC

