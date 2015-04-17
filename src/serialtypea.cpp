#include "serialtypea.hpp"

SerialTypeA::SerialTypeA()
{
    screen = new sf::RenderWindow(sf::VideoMode(640,480,32),"Serial A");

    activePort = NULL;
    alive = true;

    listenOnPortThread = new sf::Thread(&SerialTypeA::listenOnActivePort, this);

    //initialize
    init();

    //for now, hardcoding active port for testing, see enumports

    //start listening thread
    listenOnPortThread->launch();

    //start main loop
    mainLoop();
}

SerialTypeA::~SerialTypeA()
{

}

void SerialTypeA::init()
{
    //get ports
    enumPorts();

    //open logfile
    ofile.open("log.txt");

}

void SerialTypeA::enumPorts()
{
    for(int i = 0; i < 10; i++)
    {
        SerialPort *newport;

        newport = new SerialPort(i);

        if(newport->isValid())
        {
            ports.push_back(newport);

            //debug
            if(i == 5) activePort = ports.back();
        }

    }
}

void SerialTypeA::mainLoop()
{
    bool quit = false;

    while(!quit)
    {
        sf::Event event;

        screen->clear();

        //handle events
        while(screen->pollEvent(event))
        {
            //key pressed
            if(event.type == sf::Event::KeyPressed)
            {
                //escape key
                if(event.key.code == sf::Keyboard::Escape) quit = true;
                else if(event.key.code == sf::Keyboard::Space)
                {
                    //debug
                    if(activePort != NULL) activePort->debugCage();
                }

            }
        }

        //draw

        //display
        screen->display();
    }

    alive = false;
}

void SerialTypeA::listenOnActivePort()
{
    std::vector<uint8_t> packet;
    uint8_t mybufferprev = 0;

    while(alive)
    {

        if(activePort == NULL)
        {
            std::cout << "Active port is NULL!  Not listening!\n";
            continue;
        }

        uint8_t mybuffer;

        //DWORD mydword = readFromSerialPort(hComm, mybuf, 32);
        DWORD mydword = activePort->readFromSerialPort(&mybuffer, sizeof(uint8_t));

        if(mydword)
        {

            packet.push_back(mybuffer);

            ofile << std::dec;

            ofile << " 0x";

            if(int(mybuffer) < 10)
            {
                ofile << "0";
            }
            ofile << std::hex << int(mybuffer);

            //packet is done
            if(int(mybuffer) == ETX && int(mybufferprev) == DLE)
            {
                ofile << std::endl;

                printPacket(&packet);
                packet.clear();
            }

            mybufferprev = mybuffer;

        }


    }
}

void SerialTypeA::printPacket(std::vector<uint8_t> *tpacket)
{
    for(int i = 0; i < int(tpacket->size()); i++)
    {
        std::cout << std::dec;

        std::cout << " 0x";

        if(int( (*tpacket)[i]) < 10) std::cout << "0";

        std::cout << std::hex << int((*tpacket)[i]);
    }

    std::cout << std::endl;
}

