#include "serialtypea.hpp"

SerialTypeA::SerialTypeA()
{
    screen = new sf::RenderWindow(sf::VideoMode(256,256,32),"Serial A");

    activePort = NULL;
    alive = true;

    //thread for listening on serial port
    //listenOnPortThread = new sf::Thread(&SerialTypeA::listenOnActivePort, this);

    //initialize
    init();

    //for now, hardcoding active port for testing, see enumports

    //start listening thread
    //listenOnPortThread->launch();

    debugudptest();

    //configLoop();

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

    //load font
    if(!font.loadFromFile(FONT_PATH))
    {
        std::cout << "Error loading " << FONT_PATH << std::endl;
    };

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

void SerialTypeA::configLoop()
{
    bool quit = false;

    sf::Text test("THIS IS A TEST", font, 12);
    test.setColor(sf::Color::Green);

    while(!quit)
    {
        screen->clear();

        sf::Event event;

        while(screen->pollEvent(event))
        {
            if(event.type == sf::Event::KeyPressed)
            {
                quit = true;
            }

        }

        //draw
        screen->draw(test);


        screen->display();
    }


}

void SerialTypeA::mainLoop()
{
    bool quit = false;
    int deadzonestate = 0;

    int azval = 0;
    int elval = 0;

    sf::Clock transclock;

    sf::Mouse::setPosition(sf::Vector2i(128,128), *screen);

    while(!quit)
    {
        sf::Event event;

        screen->clear();

        sf::Vector2i mousePos = sf::Mouse::getPosition(*screen);

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
                else if(event.key.code == sf::Keyboard::Left)
                {
                    debugTransPressed();
                    debugTransForced();
                    debugTransAz(-127);
                }
                else if(event.key.code == sf::Keyboard::Right)
                {
                    debugTransPressed();
                    debugTransForced();
                    debugTransAz(127);
                }
                else if(event.key.code == sf::Keyboard::Up)
                {
                    debugTransPressed();
                    debugTransForced();
                    debugTransEl(127);
                }
                else if(event.key.code == sf::Keyboard::Down)
                {
                    debugTransPressed();
                    debugTransForced();
                    debugTransEl(-127);
                }

            }
            else if(event.type == sf::Event::KeyReleased)
            {
                if(event.key.code == sf::Keyboard::Left)
                {
                    //debugTransPressed();
                    //debugTransForced();
                    debugTransAz(0);
                    debugTransReleased();
                }
                else if(event.key.code == sf::Keyboard::Right)
                {
                    //debugTransPressed();
                    //debugTransForced();
                    debugTransAz(0);
                    debugTransReleased();
                }
                else if(event.key.code == sf::Keyboard::Up)
                {
                    debugTransEl(0);
                    debugTransReleased();
                }
                else if(event.key.code == sf::Keyboard::Down)
                {
                    //debugTransEl(0);
                    debugTransEl(-1);
                    debugTransReleased();
                }
            }
        }



        //draw
        //draw deadzone radius
        sf::CircleShape dzradius(DEADZONE_RADIUS, DEADZONE_RADIUS);
        dzradius.setOrigin(DEADZONE_RADIUS, DEADZONE_RADIUS);
        dzradius.setPosition(128,128);
        dzradius.setOutlineColor(sf::Color::Yellow);
        dzradius.setFillColor(sf::Color::Black);
        dzradius.setOutlineThickness(1.0f);

        //draw transducer cursor
        sf::CircleShape tcursor(4,4);
        tcursor.setOrigin(4,4);
        tcursor.setFillColor(sf::Color::Red);
        tcursor.setPosition( mousePos.x, mousePos.y);

        //slew using mouse
        if(!dzradius.getGlobalBounds().contains(sf::Vector2f(mousePos)) )
        {
            deadzonestate = DZ_OUT;
        }
        else if(deadzonestate == DZ_OUT) deadzonestate = DZ_GOINGIN;

        switch(deadzonestate)
        {
            case DZ_IN:
                break;

            case DZ_OUT:
                debugTransPressed();
                debugTransForced();
                debugTransEl( -1*mousePos.y - 128);
                debugTransAz(mousePos.x - 128);
                debugTransReleased();
                break;

            case DZ_GOINGIN:
                debugTransPressed();
                debugTransForced();
                debugTransEl(0);
                debugTransAz(0);
                debugTransReleased();
                deadzonestate = DZ_IN;
                std::cout << "DZ_TRANSITION\n";
                break;

            default:
                break;
        }


        screen->draw(dzradius);
        screen->draw(tcursor);
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

                if(PRINTPACKETS) printPacket(&packet);
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

void SerialTypeA::printPacket(sf::Packet *tpacket)
{
    for(int i = 0; i < int(tpacket->getDataSize()); i++)
    {
        std::cout << std::dec;

        std::cout << " 0x";

        uint8_t b;

        *tpacket >> b;


        if(int( b) < 10) std::cout << "0";

        std::cout << std::hex << int(b);
    }
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

void SerialTypeA::debugudptest()
{
    //create udp socket
    sf::UdpSocket socket;
    socket.setBlocking(false);

    if(socket.bind(40000) != sf::Socket::Done)
    {
        std::cout << "Error opening UDP socket\n";
        return;
    }

    std::cout << "UDP Socket created\n";

    //IDS address
    sf::IpAddress recipient = "10.192.100.211";
    unsigned short port = 36000;

    //request connection
    std::vector<uint8_t> reqpacket;
    sf::Packet mypacket;
    //reqpacket.push_back(0x10);
    reqpacket.push_back(0x04);
    reqpacket.push_back(0x00);
    reqpacket.push_back(0x00);
    //reqpacket.push_back(0x04);
    //reqpacket.push_back(DLE);
    //reqpacket.push_back(ETX);
    reqpacket = constructPacketFromData(reqpacket);


    for(int i = 0; i < int(reqpacket.size()); i++)
    {
        mypacket << sf::Uint8( reqpacket[i]);
        std::cout << sf::Uint8(reqpacket[i]) << std::endl;
    }

    std::cout << "mypacket size : " << mypacket.getDataSize() << std::endl;

    if(socket.send(mypacket, recipient, port) != sf::Socket::Done)
    {
        std::cout << "Error sending connection request\n";
    }
    else std::cout << "Connection request sent...\n";

    //await response
    bool anyresponse = false;
    sf::IpAddress sender;
    unsigned short senderport;
    char data[100];
    std::size_t received;

    sf::Packet recpacket;
    sf::Clock mytimeoutclock;

    std::cout << "Awaiting response";
    while(!anyresponse)
    {
        sf::sleep(sf::milliseconds(1000));
        if( socket.receive(recpacket,sender, senderport) != sf::Socket::Done)
        {
            //std::cout << "Error receiving data\n";
            std::cout << ".";
        }
        else
        {
            std::cout << "\n";
            std::cout << "Received " << recpacket.getDataSize() << " bytes from " << sender << " on port " << senderport << std::endl;
            std::cout << "Connection established.\n";

            printPacket(&recpacket);

            anyresponse = true;
        }

    }


    sf::sleep(sf::milliseconds(1000));

    //udpdisconnect
    mypacket.clear();
    reqpacket.clear();
    reqpacket.push_back(0x04);
    reqpacket.push_back(0x01);
    reqpacket = constructPacketFromData(reqpacket);
    for(int i = 0; i < int(reqpacket.size()); i++) mypacket << sf::Uint8( reqpacket[i]);

    if(socket.send(mypacket,recipient, port) != sf::Socket::Done)
    {
        std::cout << "Error sending udp disconnect\n";
    }
    else std::cout << "UDP disconnect sent...\n";


}
