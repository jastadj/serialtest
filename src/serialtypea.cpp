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

std::vector<uint8_t> SerialTypeA::constructPacketFromData(std::vector<uint8_t> datap)
{
    std::vector<uint8_t> finalpacket;

    //add DLE
    finalpacket.push_back(DLE);

    //add data
    for(int i = 0; i < int(datap.size()); i++) finalpacket.push_back(datap[i]);

    //add checksum
    finalpacket.push_back( calculateChecksumFromData(datap));

    //add DLE
    finalpacket.push_back(DLE);

    //add ETX
    finalpacket.push_back(ETX);

    return finalpacket;
}

uint8_t SerialTypeA::calculateChecksumFromData(std::vector<uint8_t> datap)
{
    int bytesum = 0;
    //add up all the data
    for(int i = 0; i < int(datap.size()); i++) bytesum += int(datap[i]);

    //mod 256
    return uint8_t(bytesum%256);
}

void SerialTypeA::mainLoop()
{
    bool quit = false;

    int azval = 0;
    int elval = 0;

    sf::Clock transclock;

    while(!quit)
    {
        sf::Event event;

        screen->clear();

        //handle key presses/releases outside of events
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            elval = 127;
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            elval = -128;
        }
        else elval = 0;

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            azval = -128;
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            azval = 127;
        }
        else azval = 0;

        if(azval != 0 || elval != 0)
        {
            debugTransPressed();
            debugTransForced();
            debugTransAz(azval);
            debugTransEl(elval);
            debugTransReleased();
        }

        debugTransReleased();


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
                    debugCage();
                }
                else if(event.key.code == sf::Keyboard::Return)
                {
                    debugMenuToggle();
                }
                else if(event.key.code == sf::Keyboard::LControl)
                {
                    debugExecute();
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

                if(int(packet[1]) == 0x83)
                {
                    if(isBitHigh(packet[3],0))
                    {
                        std::cout << "MENU ON\n";
                        menuOpen = true;
                    }
                    else
                    {
                        std::cout << "MENU OFF\n";
                        menuOpen = false;
                    }



                }

                printPacket(&packet);
                packet.clear();
            }

            mybufferprev = mybuffer;

        }


    }
}

bool SerialTypeA::isBitHigh(uint8_t byte, int bitnum)
{
    return (byte>>bitnum)&0x01;
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

///////////////////////////////////////////////////////


void SerialTypeA::debugCage()
{
    if(activePort==NULL) return;

    //(10) (48) (01) (01) (4A) (10) (03) CAGE MODE
    std::vector<uint8_t> packet;

    packet.push_back(0x10);
    packet.push_back(0x48);
    packet.push_back(0x01);
    packet.push_back(0x01);
    packet.push_back(0x4a);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }

}

void SerialTypeA::debugMenuToggle()
{
    if(activePort==NULL) return;

    //(10) (51) (01) (02) (54) (10) (03)
    std::vector<uint8_t> packet;
    //packet.push_back(0x10);
    packet.push_back(0x51);
    packet.push_back(0x01);
    packet.push_back(0x02);
    //packet.push_back(0x54);
    //packet.push_back(0x10);
    //packet.push_back(0x03);

    packet = constructPacketFromData(packet);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }
}

void SerialTypeA::debugExecute()
{
    if(activePort==NULL) return;

    //(10) (51) (01) (0B) (5D) (10) (03)
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x51);
    packet.push_back(0x01);
    packet.push_back(0x0B);
    packet.push_back(0x5D);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }
}

void SerialTypeA::debugTransPressed()
{
    if(activePort==NULL) return;

    //force transducer, send trans pressed
    //0x29 - trans pressed
    //0x21 - trans forced
    //0x26 - elevation rate
    //0x27 - azimuth rate
    //0x2a - trans released
    /*
    (10) (29) (01) (00) (2A) (10) (03) – TRANS_PRESSED
    (10) (21) (01) (00) (22) (10) (03) – TRANS_FORCED
    (10) (2A) (01) (00) (2B) (10) (03) – TRANS_RELEASED
    */
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x29);
    packet.push_back(0x01);
    packet.push_back(0x00);
    packet.push_back(0x2A);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }

}

void SerialTypeA::debugTransForced()
{
    if(activePort==NULL) return;

    //(10) (21) (01) (00) (22) (10) (03) – TRANS_FORCED

    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x21);
    packet.push_back(0x01);
    packet.push_back(0x00);
    packet.push_back(0x22);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }
}

void SerialTypeA::debugTransReleased()
{
    if(activePort==NULL) return;

    //(10) (2A) (01) (00) (2B) (10) (03) – TRANS_RELEASED
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x2A);
    packet.push_back(0x01);
    packet.push_back(0x00);
    packet.push_back(0x2B);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }
}

void SerialTypeA::debugTransAz(int val)
{

    if(activePort==NULL) return;

    //(10) (27) (01) (7F) (A7) (10) (03) Az_Rate = 127 (8 bit 2’s complement)
    std::vector<uint8_t> packet;
    packet.push_back(0x27);
    packet.push_back(0x01);
    packet.push_back(val);

    packet = constructPacketFromData(packet);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }
}


void SerialTypeA::debugTransEl(int val)
{

    if(activePort==NULL) return;

    //(10) (27) (01) (7F) (A7) (10) (03) Az_Rate = 127 (8 bit 2’s complement)
    std::vector<uint8_t> packet;
    packet.push_back(0x26);
    packet.push_back(0x01);
    packet.push_back(val);

    packet = constructPacketFromData(packet);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }
}

void SerialTypeA::debugRight()
{

    if(activePort==NULL) return;

    //(10) (27) (01) (7F) (A7) (10) (03) Az_Rate = 127 (8 bit 2’s complement)
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x27);
    packet.push_back(0x01);
    packet.push_back(0x7F);
    packet.push_back(0xA7);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }

}

void SerialTypeA::debugLeft()
{
    if(activePort==NULL) return;

    //(10) (27) (01) (7F) (A7) (10) (03) Az_Rate = 127 (8 bit 2’s complement)
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x27);
    packet.push_back(0x01);
    packet.push_back(0x80);
    packet.push_back(0xA8);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }

}

void SerialTypeA::debugUp()
{
    if(activePort==NULL) return;

    //(10) (26) (01) (7F) (A6) (10) (03)
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x26);
    packet.push_back(0x01);
    packet.push_back(0x7F);
    packet.push_back(0xA6);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }

}

void SerialTypeA::debugDown()
{
    if(activePort==NULL) return;

    //(10) (26) (01) (80) (A7) (10) (03)
    std::vector<uint8_t> packet;
    packet.push_back(0x10);
    packet.push_back(0x26);
    packet.push_back(0x01);
    packet.push_back(0x80);
    packet.push_back(0xA7);
    packet.push_back(0x10);
    packet.push_back(0x03);

    for(int i = 0; i < int(packet.size()); i++)
    {
        activePort->writeToSerialPort(&packet[i], sizeof(uint8_t));
    }

}
