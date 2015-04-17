#ifndef CLASS_SERIALPORT
#define CLASS_SERIALPORT

#include <cstdlib>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
//#include <iomanip>
//#include <fstream>
#include <vector>

class SerialPort
{
private:

    DCB m_DCBConfig;
    COMMTIMEOUTS m_CommTimeouts;
    DWORD m_CommEvents;

    HANDLE m_hComm;

public:
    SerialPort(int portnum, int baud = 19200);
    ~SerialPort();

    DWORD readFromSerialPort(uint8_t *buffer, int buffersize);
    DWORD writeToSerialPort(uint8_t *buffer, int buffersize);

    bool isValid() { if(m_hComm != INVALID_HANDLE_VALUE) return true; else return false;}

    void debugCage();

};

#endif // CLASS_SERIALPORT
