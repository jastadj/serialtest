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
#include <cmath>

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "serialport.hpp"

#define FONT_PATH ".\\Data\\font.ttf"

#define DLE 0x10
#define ETX 0x03

#define PRINTPACKETS 0

#define DEADZONE_RADIUS 16

//dead zone state
enum DZState{DZ_IN, DZ_OUT, DZ_GOINGIN};

class SerialTypeA
{
private:

    //screen
    sf::RenderWindow *screen;

    //resources
    sf::Font font;

    //serial
    std::vector<SerialPort*> ports;
    SerialPort *activePort;
    std::ofstream ofile; //log file output

    //init
    void init();
    void enumPorts();

    //threads
    sf::Thread *listenOnPortThread;

    //data
    bool menuOpen;
    bool polarity;
    //used to construct packet full packet with data only, no DLEs or checksums should be in input
    std::vector<uint8_t> constructPacketFromData(std::vector<uint8_t> datap);
    //calculate checksum from data only
    uint8_t calculateChecksumFromData(std::vector<uint8_t> datap);

    //loops
    bool alive;
    void mainLoop();
    void configLoop();
    void listenOnActivePort();


    //debug
    void debugCage();
    void debugMenuToggle();
    void debugExecute();
    void debugTransForced();
    void debugTransPressed();
    void debugTransReleased();
    void debugTransAz(int val);
    void debugTransEl(int val);

    void debugUp();
    void debugDown();
    void debugLeft();
    void debugRight();

    bool isBitHigh(uint8_t byte, int bitnum);

    void debugudptest();

public:
    SerialTypeA();
    ~SerialTypeA();

    void printPacket(std::vector<uint8_t> *tpacket);
    void printPacket(sf::Packet *tpacket);

};


#endif // CLASS_STA
