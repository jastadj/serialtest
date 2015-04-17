#include "serialport.hpp"

SerialPort::SerialPort(int portnum, int baud)
{
    //create comm name handle
    std::stringstream comname;
    comname << "\\\\.\\COM" << portnum;

    //init DCB
    m_DCBConfig = {0};

    //set timeouts
    m_CommTimeouts.ReadIntervalTimeout = 2;
    m_CommTimeouts.ReadTotalTimeoutConstant = 2;
    m_CommTimeouts.ReadTotalTimeoutMultiplier = 2;
    m_CommTimeouts.WriteTotalTimeoutConstant = 2;
    m_CommTimeouts.WriteTotalTimeoutMultiplier = 2;

    //set mask events
    m_CommEvents =EV_RXCHAR | EV_CTS;

    m_hComm = CreateFile( comname.str().c_str(),
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    //able to open comm port?
    if (m_hComm != INVALID_HANDLE_VALUE)
    {
        //com port found
        std::cout << "Com port found at COM" << portnum << std::endl;

        //reset dcb
        //FillMemory(&myconfig, sizeof(DCB),0);
        m_DCBConfig.DCBlength = sizeof(DCB);

        //comm timeouts
        if(SetCommTimeouts(m_hComm, &m_CommTimeouts) )
       {
           std::cout << "Set comm timeouts.\n";
       }
       else std::cout << "Error setting comm timeouts.\n";

       if(SetCommMask(m_hComm, m_CommEvents ))
       {
           std::cout << "Set comm mask\n";
       }
       else std::cout << "Error setting comm mask\n";

        //build comms
        std::stringstream comconstructor;
        comconstructor << baud << ",n,8,1";
        if(BuildCommDCB(LPCSTR(comconstructor.str().c_str()), &m_DCBConfig))
        {
            std::cout << "Built com port successfully.\n";
        }
        else std::cout << "Failed to build com port.\n";

        //set baud
        m_DCBConfig.BaudRate = baud;
        m_DCBConfig.ByteSize= 8;
        m_DCBConfig.StopBits=ONESTOPBIT;
        m_DCBConfig.Parity=NOPARITY;

        if(SetCommState(m_hComm, &m_DCBConfig))
        {
            std::cout << "Set comm state successfully.\n";
        }

        if(GetCommState(m_hComm, &m_DCBConfig) )
        {
            std::cout << "Com port is ready for use:\n";

        }
        else
        {
            std::cout << "Unable to get comm state.  Error code:" << GetLastError() << "\n";
        }
    }


}

SerialPort::~SerialPort()
{
    //close port
    CloseHandle(m_hComm);
}

DWORD SerialPort::readFromSerialPort(uint8_t *buffer, int buffersize)
{
    DWORD dwBytesRead = 0;
    if(!ReadFile(m_hComm, buffer, buffersize, &dwBytesRead, NULL))
    {
        //handle error
    }
    return dwBytesRead;
}


DWORD SerialPort::writeToSerialPort(uint8_t *data, int buffersize)
{

	DWORD dwBytesRead = 0;
	if(!WriteFile(m_hComm, data, buffersize, &dwBytesRead, NULL))
    {
		//error
	}
	return dwBytesRead;

}
