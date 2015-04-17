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

    //data
    //used to construct packet full packet with data only, no DLEs or checksums should be in input
    std::vector<uint8_t> constructPacketFromData(std::vector<uint8_t> datap);
    //calculate checksum from data only
    uint8_t calculateChecksumFromData(std::vector<uint8_t> datap);

    //loops
    bool alive;
    void mainLoop();
    void listenOnActivePort();


    //debug
    void debugCage();
    void debugMenuToggle();
    void debugExecute();
    void debugTransForced();
    void debugTransPressed();
    void debugTransReleased();
    void debugUp();
    void debugDown();
    void debugLeft();
    void debugRight();

public:
    SerialTypeA();
    ~SerialTypeA();

    void printPacket(std::vector<uint8_t> *tpacket);

};


#endif // CLASS_STA
