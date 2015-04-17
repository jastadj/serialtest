#ifndef CLASS_STA
#define CLASS_STA

#include <cstdlib>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>

#include <SFML\Graphics.hpp>
#include "serialport.hpp"

#define DLE 0x10
#define ETX 0x03

class SerialTypeA
{
private:

    //screen
    sf::RenderWindow *screen;

    std::vector<SerialPort*> ports;
    SerialPort *activePort;
    std::ofstream ofile; //log file output

    //init
    void init();
    void enumPorts();

    //threads
    sf::Thread *listenOnPortThread;

    //loops
    bool alive;
    void mainLoop();
    void listenOnActivePort();

public:
    SerialTypeA();
    ~SerialTypeA();

    void printPacket(std::vector<uint8_t> *tpacket);

};


#endif // CLASS_STA
