//  compass cpp   24.02.2012
//  compass
//  2012 modification
#include "i2c.cpp"
#include "hvps.cpp"

//#define MONOCHAN

#ifndef _COMPASS
#define _COMPASS

#define CHADDR_COMPASS 61  // ChannelAddres on vip plate
//#define AddrEEPROM  0x8
#define AddrCOMPASS 0x21  // inner address to read/write

//class compass: public hvps
class compass: private hvps
{
 public:
        unsigned int Offset[5];   // koefficients of compass module
        unsigned char WriteAddr;
        unsigned char ReadAddr;
        unsigned char OperatMode; // in ideal = 0x52 
        unsigned short  Heading;
        char          log_out[300];

        // constructor
        compass()
        {
            BaseAddr    = 0x378;
            ADCSubAddr2 = 0x21;
            WriteAddr = AddrCOMPASS;
            ReadAddr  = AddrCOMPASS;
            SetChannelAddr(CHADDR_COMPASS); // adress of COMPASS
            init();
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
        unsigned int Res = 0;
	char info[100];

#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
        //Send A=0x41 command
        if(!TX_reg(WriteAddr, 0x41)) //
        {
	    strcpy(info, "compass: make_read: Error in TX_reg putting!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 0;
        }
        // wait 6000 mksec
        usleep(10000);

        // read from 0x43 addres 2 bytes
        if(!RX16(ReadAddr, &Res)) //
        {
	    strcpy(info, "compass: make_read: Error in RX16 reading!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 0;
        }
        return Res;
    }

    unsigned int send_command(unsigned char Command)
    {
	char info[100];
#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif

        sprintf(info, "compass: send_command: '%c'\n", Command);
        //fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
        //Send A=0x41 command
        if(!TX_reg(WriteAddr, Command)) //
        {
	    strcpy(info, "compass: send_command: Error in TX_reg putting!!!!\n");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 1;
        }
        return 0;
    }

    unsigned int read_data()
    {
        unsigned short Res = 0;
	char info[100];
	
#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
        // read from 0x43 addres 2 bytes
        if(!RX16(ReadAddr, &Res)) //
        {
	    strcpy(info, "compass: read_data: Error in RX16 reading!!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 0;
        }
        sprintf(info, "Compass: %d\n", Res );
        //fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
        return Res;
    }

    //  ----------- EVERY SEC ------
    unsigned int every_sec(char* message)
    {
        // read data
        Heading = read_data();
        if( ! (OperatMode & 2))
            send_command('A');
        sprintf(log_out, "Compass: %d.%d gr", Heading/10, Heading%10);
        strcpy(message, log_out);

        return Heading;
    }

    //  ----------- EEPROM ------
    unsigned int read_from_EEPROM(unsigned char EEAddr)
    {
        //unsigned int Res = 0;
        unsigned char Res = 0;
	char info[100];

#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
        if(!TX8(WriteAddr, 0x72, EEAddr)) //
        {
	    strcpy(info, "compass: read_from_EE: Error in TX8 putting!!!! ");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 1<<16 ;
        }
        // delay 70 usec
        usleep(100);

        // read from 0x43 addres 1 bytes
        if(!RX8(ReadAddr, &Res)) //
        {
	    strcpy(info, "compass: read_from_EE: Error in RX8 reading!!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 1<<17;
        }
        return Res;
    }

    unsigned int write_to_EEPROM(unsigned char EEAddr, unsigned char Dat)
    {
	char info[100];

#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
        Conv.tChar[1] = EEAddr;
        Conv.tChar[0] = Dat;
        if(!TX16(WriteAddr, 0x77, Conv.tInt)) //
        {
	    strcpy(info, "compass: write_to_EE: Error in TX16 putting!!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 1000; // err
        }
        // delay 70 usec
        usleep(200);
        return 0;
    }

    unsigned int read_full_EEPROM()
     {
         unsigned char i = 0; //, ADat = 0;
         unsigned int result = 0;
 	 char info[100];

#ifndef MONOCHAN
         SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
         for (i = 0; i <= 8; i++)
         {
            usleep(100000);
            sprintf(info,"i = %i: ", i);
            //fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);

            result = read_from_EEPROM(i);

            sprintf(info,"  res%i = 0x%x\n", i, result);
            //fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
         }
         return 0;
     }

    // ------------- RAM ------------
    unsigned int read_from_RAM(unsigned char RamAddr)
    {
        //unsigned int Res = 0;
        unsigned char Res = 0;
	char info[100];

#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
        if(!TX8(WriteAddr, 0x67, RamAddr)) //
        {
	    strcpy(info, "compass: read_from_Ram: Error in TX8 putting!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 1000;
        }
        // delay 70 usec
        usleep(1000);

        // read from 0x43 addres 1 bytes
        if(!RX8(ReadAddr, &Res)) //
        {
	    strcpy(info, "compass: read_from_RAM: Error in RX8 reading!!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 1000;
        }
        return Res;
    }

    unsigned int write_to_RAM(unsigned char RamAddr, unsigned char Dat)
    {
        //unsigned int Res = 0;
        unsigned char Res = 0;
	char info[100];

#ifndef MONOCHAN
        SetChannelAddr(CHADDR_COMPASS); // address of compass = 61
#endif
        Conv.tChar[1] = RamAddr;
        Conv.tChar[0] = Dat;
        if(!TX16(WriteAddr, 0x47, Conv.tInt)) //
        {
	    strcpy(info, "compass: write_to_RAM: Error in TX16 putting!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 0;
        }
        // delay 70 usec
        usleep(1000);

        // read from 0x43 addres 1 bytes
        if(!RX8(ReadAddr, &Res)) //
        {
	    strcpy(info, "compass: write_to_RAM: Error in RX8 reading!!");
            fprintf(stdout, "%s", info);
            if(dout) fprintf(dout, "%s", info);
            return 0;
        }
        return Res;
    }
    
    //   -------------  INIT ----------------
     unsigned int init()
    {
        unsigned int OperationalMode = 0x52;
	unsigned int res = 0;
	char info[100];

/*        // write 1 - summing mode
        write_to_EEPROM(0x6,1);
        usleep(200);
        // write operational mode
        write_to_RAM(0x74, OperationalMode);
*/
        sprintf(info, "\n====== Compass init ======\n");
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);

	// read EEPROM
	res = read_from_EEPROM(0x0);
	sprintf(info,"EE  addr = 0x00 res = 0x%x  == compass address\n", res);
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);

	if(res == 0x42)
	    sprintf(info, "Compass looks to be Stable\n");
        else 
	    sprintf(info, "CRITICAL ERROR! Compass looks to be UnStable! Enlarge delay in SCL() function in compass.cpp!!\n");

        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);

        // read operational mode
	res = read_from_RAM(0x74);
	sprintf(info, "RAM addr = 0x74 res = 0x%x  == operation mode\n", res);
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
	OperatMode = res;
	// read operational mode
	res = read_from_RAM(0x74);
        sprintf(info, "RAM addr = 0x74 res = 0x%x  == operation mode\n", res);
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
	if((OperatMode == res) && (OperatMode != 1000)) 
	    sprintf(info, "Compass looks to be Stable\n");
        else 
	    sprintf(info, "Compass looks to be UnStable. Enlarge delay in SCL() function in compass.cpp!!\n");
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
	
	if(OperatMode == OperationalMode) 
	    sprintf(info, "Compass looks to be in Continuous mode: OK\n");
        else 
	    sprintf(info, "Compass looks to be in non-Continuous mode...\n");
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
    
	// read output data mode
	res = read_from_RAM(0x4E);
	sprintf(info, "RAM addr = 0x4E res = 0x%x  == output data mode \n", res);
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);

	// read EEPROM
	res = read_from_EEPROM(0x6);
	sprintf(info,"EE  addr = 0x06 res = 0x%x  == summing\n", res);
        fprintf(stdout, "%s", info);
        if(dout) fprintf(dout, "%s", info);
    
        read_full_EEPROM();

	send_command('A');
	Heading = 0;
	Heading = every_sec(info);

        sprintf(info, "====== Compass init End ======\n");
        if(stdout) fprintf(stdout, "%s", info);
        if(  dout) fprintf(  dout, "%s", info);
	
        return 0;
     }


 }; // class end
#endif

