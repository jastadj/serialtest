#include <cstdlib>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

DWORD readFromSerialPort(HANDLE hSerial, uint8_t * buffer, int buffersize)
{
    DWORD dwBytesRead = 0;
    if(!ReadFile(hSerial, buffer, buffersize, &dwBytesRead, NULL))
    {
        //handle error
    }
    return dwBytesRead;
}

DWORD writeToSerialPort(HANDLE hSerial, uint8_t * data, int length)
{

	DWORD dwBytesRead = 0;
	if(!WriteFile(hSerial, data, length, &dwBytesRead, NULL))
    {
		//error
	}
	return dwBytesRead;

}

int main(int argc, char *argv[])
{
    //enumerate com ports
    for(int i = 0; i < 10; i++)
    {
        std::stringstream comname;
        comname << "\\\\.\\COM" << i;
        DCB myconfig = {0};

        HANDLE hComm;
        COMMTIMEOUTS commtimeouts;
        commtimeouts.ReadIntervalTimeout = 1000;
        commtimeouts.ReadTotalTimeoutConstant = 1000;
        commtimeouts.ReadTotalTimeoutMultiplier = 1000;
        commtimeouts.WriteTotalTimeoutConstant = 1000;
        commtimeouts.WriteTotalTimeoutMultiplier = 1000;

        DWORD commevents =EV_RXCHAR | EV_CTS;

        hComm = CreateFileA( comname.str().c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);
        if (hComm != INVALID_HANDLE_VALUE)
        {
            std::cout << "Com port found at COM" << i << std::endl;

            //reset dcb
            //FillMemory(&myconfig, sizeof(DCB),0);
            myconfig.DCBlength = sizeof(DCB);

            //comm timeouts
            if(SetCommTimeouts(hComm, &commtimeouts) )
           {
               std::cout << "Set comm timeouts.\n";
           }
           else std::cout << "Error setting comm timeouts.\n";

           if(SetCommMask(hComm, commevents ))
           {
               std::cout << "Set comm mask\n";
           }
           else std::cout << "Error setting comm mask\n";

            //build comms
            if(BuildCommDCB(LPCSTR("19200,n,8,1"), &myconfig))
            {
                std::cout << "Built com port successfully.\n";
            }
            else std::cout << "Failed to build com port.\n";

            //set baud
            myconfig.BaudRate = 19200;
            myconfig.ByteSize=8;
            myconfig.StopBits=ONESTOPBIT;
            myconfig.Parity=NOPARITY;

            if(SetCommState(hComm, &myconfig))
            {
                std::cout << "Set comm state successfully.\n";
            }

            if(GetCommState(comname, &myconfig))
            {
                std::cout << "Com port is ready for use:\n";

            }
            else
            {
                std::cout << "Unable to get comm state.  Error code:" << GetLastError() << "\n";
            }





        }
        else
        {
            //std::cout << "No com " << i << std::endl;
        }

        CloseHandle(hComm);

    }



    return 0;

}
