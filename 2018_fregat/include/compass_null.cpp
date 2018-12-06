/**
 * \file  compass_null.cpp
 * \brief Class compass. No compas
 * \date  November 2018
 * 
 * Class with no commpass. Exist for backward compatibility with fregat-2013
*/
#ifndef _COMPASS
#define _COMPASS
#include "i2c.cpp"
#include "hvps.cpp"

#define CHADDR_COMPASS 61  // ChannelAddres on vip plate
#define AddrCOMPASS 0x21  // inner address to read/write

class compass: private hvps
{
 public:
        //unsigned int Offset[5];   // koefficients of compass module
        //unsigned char WriteAddr;
        //unsigned char ReadAddr;
        //unsigned char OperatMode; // in ideal = 0x52 
        //unsigned short  Heading;
        //char          log_out[300];

        // constructor
        compass()
        {
            BaseAddr    = 0x378;
            ADCSubAddr2 = 0x21;
            //WriteAddr = AddrCOMPASS;
            //ReadAddr  = AddrCOMPASS;
            //SetChannelAddr(CHADDR_COMPASS); // adress of COMPASS
            //init();
        }

 private:

    void SCL(unsigned char Data)
    {
        int t = 0, z = 0;

        if (!(Data)) {outb((inb(BaseAddr+2)  | 1),   (BaseAddr+2));
                      outb((inb(BaseAddr+2)  | 1),   (BaseAddr+2));}
        else         {outb((inb(BaseAddr+2) & (~1)), (BaseAddr+2));
                      outb((inb(BaseAddr+2) & (~1)), (BaseAddr+2));}
        for(t = 0; t <= 9000; t++)
            z++;
    }

 public:
     //---------------------------------------------------------
    unsigned int make_read()
    {
        return 0;
    }

    unsigned int send_command(unsigned char Command)
    {
        return 0;
    }

    unsigned int read_data()
    {
        return 0;
    }

    //  ----------- EVERY SEC ------
    unsigned int every_sec(char* message)
    {
        return 0;
    }

    //  ----------- EEPROM ------
    unsigned int read_from_EEPROM(unsigned char EEAddr)
    {
        return 0;
    }

    unsigned int write_to_EEPROM(unsigned char EEAddr, unsigned char Dat)
    {
        return 0;
    }

    unsigned int read_full_EEPROM()
    {
         return 0;
    }

    // ------------- RAM ------------
    unsigned int read_from_RAM(unsigned char RamAddr)
    {
        return 0;
    }

    unsigned int write_to_RAM(unsigned char RamAddr, unsigned char Dat)
    {
        return 0;
    }
    
    //   -------------  INIT ----------------
    unsigned int init()
    {
        return 0;
    }


 }; // class end
#endif

