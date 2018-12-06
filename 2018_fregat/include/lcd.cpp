//  lcd.cpp   04.02.2009-06.02.
//  board of LCD calibration
//  23.01.2012
#include "i2c.cpp"

#ifndef _LCD
#define _LCD
#define LCD_FILE "./config/lcd.config"

#include "bar.cpp"

//#ifndef BaseAddrLCD
//#define BaseAddrLCD  0x3720
//0x4320
//#endif
//#define BaseAddrLCD  0x3720

class lcd: public i2c
{
  private:
        int fifo[22];
        long fifosum;
  public:
        char out[1024];
        unsigned char lcd_mask; // byte to write to 0xD address
        unsigned char manager;  // byte to write to 0xB address
        int banner_external; // flag to prohibit external pulses 

        struct lconfig
        {
            unsigned int c[9];
            unsigned int m;
            unsigned int b; 
            unsigned int l; // banner_external
            unsigned int w;
            unsigned int d;
        } ReadConf, Conf; 

        // constructor
        lcd()
        {
            fifosum = 0;
            for(int z = 0; z < 22; z++)
            {
                fifo[z] = 0;
            }
            for(int z = 0; z < 8; z++)
            {
                Conf.c[z] = 20;
                ReadConf.c[z] = 20;
            }
            Conf.c[8] = 255;
            ReadConf.c[8] = 255;
            Conf.m = 5;
            Conf.b = 1; 
            Conf.l = 1; // banner_external
            Conf.w = 1;
            Conf.d = 5;

            BaseAddr    = BaseAddrLCD;
            ADCSubAddr2 = 0x00;
            lcd_mask = 0;
            manager = 10;
            banner_external = 1; // flag to prohibit external pulses 

            lcd_init();
        }

    private:
        //  I2C interface  // obrashenie k zhelezy
        void SDA(unsigned char Data)
        {
            if (!(Data)) { outb((inb(BaseAddr)  |  2),  (BaseAddr));
                           outb((inb(BaseAddr)  |  2),  (BaseAddr));}
            else         { outb((inb(BaseAddr) & (~2)), (BaseAddr));
                           outb((inb(BaseAddr) & (~2)), (BaseAddr));}
        }

        void SCL(unsigned char Data)
        {
            if (!(Data)) {outb((inb(BaseAddr)  |  1),  (BaseAddr));
                          outb((inb(BaseAddr)  |  1),  (BaseAddr));}
            else         {outb((inb(BaseAddr) & (~1)), (BaseAddr));
                          outb((inb(BaseAddr) & (~1)), (BaseAddr));}
        }

        unsigned char SDAin(void)
        {
            SDA(1);
            if ((inb(BaseAddr) & 8))
                { return 1;}
            return 0;
        }

        // ================================================ //
        // ================================================ //
        //  LCD board functions
        // ================================================ //
        // set base address
        void SetAddr(unsigned int addr)
        {
            BaseAddr = addr;
            //if(dout) fprintf(stdout, "LED: BaseAddr = %xh\n", BaseAddr);
            if(dout) fprintf(  dout, "LED: BaseAddr = %xh\n", BaseAddr);
        }

        //read Data from DAC whit addr AddrDev = 0
        unsigned char ReadDAC( unsigned char AddrDev, unsigned char *Data)
        {
            unsigned char tmpAddr;

            if (AddrDev==0) {tmpAddr = 0x2C;}
            //else if (AddrDev==1) {tmpAddr = 0x2D;}
                 else {return 0;}
            return RX8(tmpAddr, Data);
        }

        // ================================================ //
        // init lcd
        unsigned char lcd_init()
        {
            manager = set_bit_1(manager, 0); // prohibit external trigger for LCD pulses
            manager = set_bit_0(manager, 1); // set bit 2 of manager(addr 0xB) to 1 = work position
            outb(manager, BaseAddrLCD + 0xB);
            //printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);

            // lamp off:
            //if(set_chan_amp(8, 255)) printf("\nError in set_chan_amp!!");
            return 0;
        }

    public:
        // prepare lcd - run before mesurements start:
        // set work parameters
        unsigned char lcd_prepare()
        {
            set_lcd_duration(20);

            manager = set_bit_1(manager, 0); // prohibit external trigger for LCD pulses
            manager = set_bit_0(manager, 1); //
            manager = set_bit_0(manager, 2); //
            manager = set_bit_0(manager, 3); //
            manager = set_bit_0(manager, 4); //
            manager = set_bit_0(manager, 5); //
            manager = set_bit_1(manager, 6); //
            manager = set_bit_0(manager, 7); //
            outb(manager, BaseAddrLCD + 0xB);
            //printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);
            return 0;
        }

//-----------------------------------------------------
    public:
        // permit 
        unsigned char ban_external(unsigned char onoff) // permit extermal trigger
        {
            // set bit 1 of manager(addr 0xB) to on/off
            if(onoff)   manager = set_bit_1(manager,0);
            else        manager = set_bit_0(manager,0);

            banner_external = onoff;

            outb(manager, BaseAddrLCD + 0xB);
            printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);
            return 0;
        }

        unsigned char led_in_event_onoff(unsigned char onoff) // permit extermal trigger
        {
            // set bit 1 of manager(addr 0xB) to on/off
            if(onoff)   manager = set_bit_1(manager,1);
            else        manager = set_bit_0(manager,1);

            outb(manager, BaseAddrLCD + 0xB);
            printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);
            return 0;
        }

        //  !!!!! on(1)/off(0) generator & _12V for barometers
        unsigned char generator_12_onoff( unsigned char onoff )
        {
            // set bit 2 of manager(addr 0xB) to on/off generator and +12V
            if(onoff) manager = set_bit_1(manager,2);
            else      manager = set_bit_0(manager,2);
            printf("LCD:man = %i, addr =%xh\n", manager, BaseAddrLCD + 0xB);
            if(dout) fprintf(dout, "LCD:man = %i, addr =%xh\n", manager, BaseAddrLCD + 0xB);

            outb(manager, BaseAddrLCD + 0xB);
            usleep(50000);
            return 0;
        }

        //  !!!!! on(1)/off(0) barometer
        unsigned char bar_onoff(unsigned char onoff )
        {
            // set bit 3 of manager(addr 0xB) to on/off
            if(onoff)   manager = set_bit_1(manager,3);
            else        manager = set_bit_0(manager,3);

            outb(manager, BaseAddrLCD + 0xB);
            return 0;
        }

        // make lcd pulse
        unsigned char lcd_pulse()
        {
            manager = set_bit_0(manager,0); // set bit 0 of manager(addr 0xB) to 1
            outb(manager, BaseAddrLCD + 0xB);
            //printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);
            manager = set_bit_1(manager, 0); // set bit 0 of manager(addr 0xB) to 0 = start lcd pulse
            outb(manager, BaseAddrLCD + 0xB);
            //printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);
            if(!banner_external) 
            {
                manager = set_bit_0(manager,0); //  permit external trigger for pulses
                outb(manager, BaseAddrLCD + 0xB);
            }
            return 0;
        }

        // set lcd delay after trigger
        unsigned char set_trigger_delay(unsigned char delay)
        {
            delay   = delay << 4;      // dddd0000
            manager = manager & 15;    // 0000mmmm
            manager = delay | manager; // ddddmmmm

            outb(manager, BaseAddrLCD + 0xB);
            //printf("\nBaseAddrLCD + 0xB = %xh, byte = %i\n", BaseAddrLCD + 0xB, manager);
            return 0;
        }

        //-----------------------------------------------------
        // make N lcd pulses
        unsigned char lcd_pulseN(unsigned int nn)
        {
            unsigned int ii = 0;

            for(ii = 0; ii < nn; ii ++)
            {
                lcd_pulse();
                usleep(1000);
            }
            return 0;
        }

        // make N lcd pulses with step 1 sec
        unsigned char lcd_pulseN_sec(unsigned int nn)
        {
            unsigned int ii = 0;

            for(ii = 0; ii < nn; ii ++)
            {
                lcd_pulse();
                sleep(1);
            }
            return 0;
        }
        // read DAC in LCD channel
        unsigned int read_chan_amp(unsigned char Chan)
        {
            unsigned char Dat = 0;
            unsigned int  addr = BaseAddrLCD + Chan - 1;

            //printf("read_chan_amp in\n");
            SetAddr(addr);
            if(!ReadDAC(0, &Dat))
            {
                printf("ReadDAC failed in recieve\n");
                if(dout) fprintf(dout, "ReadDAC failed in recieve\n");
                return 1;
            }
            printf("DAC[%1i] = %3i \n", Chan, Dat);
            if(dout) fprintf(dout, "DAC[%1i] = %3i \n", Chan, Dat);
            return 0;
        }

        //
        unsigned char set_chan_amp(unsigned char chan, unsigned char Data)
        {
            unsigned char out = 0;
            unsigned int addr = BaseAddrLCD + chan - 1;

            //printf("set_chan_amp in\n");
            SetAddr(addr);
            //printf("SetAdrr o'k\n");
            if( !WriteDAC(0x0, Data))  //
            {
                out = 1;
                printf("\nset_lcd_amp: Error in DAC`s transmittion !!! chan = %d\n", (unsigned int)chan);
                if(dout) fprintf(dout, "\nset_lcd_amp: Error in DAC`s transmittion, chan = %d\n", chan);
            }
            printf("WriteDAC O'K");
            return out;
        }

        //
        int set_lcd_duration( unsigned char Data)
        {
            printf("\nBaseAddrLCD + 0xC = %xh\n", BaseAddrLCD + 0xC);
            outb(Data, BaseAddrLCD + 0xC);
            return 0;
        }

        // add to fifo current value of CH0-CH2
        int sum_to_fifo(int val)
        {
            int i = 0;
            // ---- shift fifo
            //printf("sum_to_fifo: Enter\n");
            if( fifo[0] == 20)
            {
                fifosum -= fifo[1];
                for(i = 1; i <= 19; i++)
                {
                    fifo[i] = fifo[i+1];
                }
                fifo[0] = 19;
            }

            //add value to fifo
            if( fifo[0] < 20)
            {
                fifo[0] ++;
                fifo[fifo[0]] = val;
                fifosum += val;
            }
            else
            {
                printf("sum_to_fifo: Error!");
            }

/*            for(i = 1; i <= fifo[0]; i++)
            {
                printf("   fifo[%4d] = %2d\n", i, fifo[i]);
            }
            printf("sum_to_fifo: Exit\n");
*/
            return (long) fifosum * 10 /fifo[0];
        }

        // read and print ADC registers
        int read_ADC(char *message)
        {
            unsigned int   Data[4];
            unsigned short i = 0; //, ii = 0;
            unsigned int   tmp = 0; //, n = 0;
            //unsigned long  sum = 0;
            char mes[100];
            float dp = 0.;

            strcpy(message, "LED: ");
            SetAddr(BaseAddrLCD + 0x000E);
            if(!ReadADCs(0x00, (unsigned int*)&Data))
            {
                printf("LED:  No device\n");
                if(dout) fprintf(dout, "  No device\n");
                return 1;
            }
            for(i = 0; i < 4; i++) // read ADC
            {
                tmp  = Data[i];
                tmp &= 0x0FFF;
                sprintf(mes, " CH%1i[%4i]", i, tmp);
                strcat(message, mes);
            }

            tmp  = Data[0] - Data[2];
            tmp &= 0x0FFF;
            dp = tmp;
            //if(tmp < 0) tmp = 0;
            //if(tmp) sum = sum_to_fifo(tmp);
            //printf("   dP[%4d]  aver = %5ld\n", tmp, sum);
            //sprintf(mes, "   dP[%4d]  aver = %4ld.%ld\n", tmp, sum/10, sum%10);
            dp = (dp - 5.) * 2.74;
            sprintf(mes, "   DeltaP = %7.3f kPa  (%6.2f mm w)\n", dp/1000, dp/9.81);
            //sprintf(tmp,"Delta P:  %6.2f kPa (%6.2f mm w)\n",dP, dP/0.00981);

            //sprintf(mes, "   DeltaP = %6.2f Pa =  aver = %4ld.%ld\n", dp, sum/10, sum%10);
            strcat(message, mes);

            //if(stdout) fprintf(stdout, "\n%s", message);
            if(  dout) fprintf(  dout, "\n%s", message);
            return 0;
        }

/*        int read_ADC(void)
        {
            unsigned int   Data[4];
            unsigned short i = 0;
            unsigned int   tmp = 0;
            unsigned long  sum = 0;
            char mes[100];

            strcpy(out, "LED: ");
            SetAddr(BaseAddrLCD + 0x000E);
            if(!ReadADCs(0x00, (unsigned int*)&Data))
            {
                printf("LED:  No device\n");
                if(dout) fprintf(dout, "  No device\n");
                return 1;
            }
            for(i = 0; i < 4; i++) // read ADC
            {
                tmp  = Data[i];
                tmp &= 0x0FFF;
                sprintf(mes, " CH%1i[%4i]", i, tmp);
                strcat(out, mes);
            }

            tmp  = Data[0] - Data[2];
            tmp &= 0x0FFF;
            if(tmp < 0) tmp = 0;
            if(tmp) sum = sum_to_fifo(tmp);
            //printf("   dP[%4d]  aver = %5ld\n", tmp, sum);
            //sprintf(mes, "   deltaP = [%4d] =  aver = %4ld.%ld\n", (tmp-5)*2.74, sum/10, sum%10);
            sprintf(mes, "   deltaP = %6.2f =  aver = %4ld.%ld\n", (tmp-5)*2.74, sum/10, sum%10);
            strcat(out, mes);

            //if(stdout) fprintf(stdout, "\n%s", message);
            if(  dout) fprintf(  dout, "\n%s", out);
            return 0;
        }
*/

        // functions:
        int read_config_from_file();
        int print_config(struct lconfig Conf);
        int import_config(); // write lcd configuration ReadConf to Conf
        int set_config();
        int save_config();
        int test_config(struct lconfig ReadConf);
        int set_config_from_file();
};

//======================================
// return:  0  -- OK
//         -1  -- error in file opening
//        err  -- number of errors in file reading
// 
int lcd::read_config_from_file()
{
    char  buf[100] = {0};
    char  line[250] = {0};
    int   kk = 0, err = 0;
    unsigned short lpar = 0, lnum = 0;
    FILE *fpar = NULL;

    // ------ open config file --------------------------
    if((fpar = fopen( LCD_FILE, "r")) == NULL)
    {
        printf("\n  Error: file \"%s\" is not open!\n", LCD_FILE);
        if(dout) fprintf(dout, "\n  Error: file \"%s\" is not open!\n", LCD_FILE);
        return -1;
    }
    printf("\n\n\n\nfile \"%s\" is open!\n", LCD_FILE);
    if(dout) fprintf(dout, "===> MASTER from \"%s\"  <=====\n", LCD_FILE);

    // ------- read parameters from config file ----------
    while( fgets(line, sizeof(line), fpar) != NULL )
    {
        //printf("line:%s", line);
        if( (line[0] == '\n') || (line[0] == '/') )
        {
            continue;
        }
        if(dout) fprintf(dout, "%s", line);        

        kk = sscanf(line, "%s", buf);
        //printf( "\n==>%s<==", buf);
        if(!kk) continue;
        //if( !strcmp(buf,"/") ) continue;

        lpar = 0;  
        lnum = 0;
        kk = 0;
        if( !strcmp(buf,"c") )
        {
            kk = sscanf(line, "%s %hu %hu", buf, &lnum, &lpar);
            if(kk > 2)
            {
                if((lnum > 0) && (lnum < 9)) 
                    ReadConf.c[lnum] = lpar;
            }
            else      err++;
            continue;
        }
        if( !strcmp(buf,"d") )
        {
            kk = sscanf(line, "%s %hu", buf, &lpar);
            if(kk > 1)    ReadConf.d = lpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"m") )
        {
            kk = sscanf(line, "%s %hu", buf, &lpar);
            if(kk > 1)    ReadConf.m = lpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"b") )
        {
            kk = sscanf(line, "%s %hu", buf, &lpar);
            if(kk > 1)    ReadConf.b = lpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"l") )
        {
            kk = sscanf(line, "%s %hu", buf, &lpar);
            if(kk > 1)    ReadConf.l = lpar;
            else      err++;
            continue;
        }
        if( !strcmp(buf,"w") )
        {
            kk = sscanf(line, "%s %hu", buf, &lpar);
            if(kk > 1)    ReadConf.w = lpar;
            else      err++;
            continue;
        }
    }
    fclose(fpar);
    if(dout) fprintf(dout, "===> end of file \"%s\" <=====\n", LCD_FILE);
    if(dout) fprintf(dout, "=============================================\n");
    return err; // number of errors
}

int lcd::print_config(struct lconfig Conf)
{
    int i = 0;

    printf("LCD.Config:\n");
    if(dout) fprintf(dout, "LCD.Config:\n");

    for(i = 1; i <= 8; i++)
    {
        printf("c %d %d\n", i, Conf.c[i]);
        if(dout) fprintf(dout, "c %d %d\n", i, Conf.c[i]);
    }
    printf("d %d\n", Conf.d);
    printf("m %d\n", Conf.m);
    printf("b %d\n", Conf.b);
    printf("l %d\n", Conf.l);
    printf("w %d\n", Conf.w);
    if(dout) fprintf(dout,"d %d\n", Conf.d);
    if(dout) fprintf(dout,"m %d\n", Conf.m);
    if(dout) fprintf(dout,"b %d\n", Conf.b);
    if(dout) fprintf(dout,"l %d\n", Conf.l);
    if(dout) fprintf(dout,"w %d\n", Conf.w);

    return 0; // 
}

// write lcd configuration ReadConf to Conf
int lcd::import_config()
{
    int i = 0;

    printf("port LCD.Config:");
    if(dout) fprintf(dout, "port LCD.Config:");

    for(i = 1; i <= 8; i++)
    {
        Conf.c[i] = ReadConf.c[i];
    }
    Conf.d = ReadConf.d;
    Conf.m = ReadConf.m;
    Conf.b = ReadConf.b;
    Conf.l = ReadConf.l;
    Conf.w = ReadConf.w;

    printf("... done\n");
    if(dout) fprintf(dout, "... done\n");

    return 0; // 
}

// return 1 -- file opening error
int lcd::save_config()
{
    FILE *fpar = NULL;
    int i = 0;
    char fname[100] = {"./log/lcd.con"};
    
    // ------ open config file --------------------------
    if((fpar = fopen(fname, "w")) == NULL)
    {
        printf("\n  Error: file \"%s\" is not open!\n", fname);
        if(dout) fprintf(dout, "\n  Error: file \"%s\" is not open!\n", fname);
        return 1;
    }
    printf("\n\n\nfile \"%s\" is open!\n", fname);
    //if(dout) fprintf(dout, "\n\n\nfile \"%s\" is open!\n", fname);

    printf("Save LCD.Config:");
    //if(dout) fprintf(dout, "Save LCD.Config:");
    
    // ------ print config file --------------------------
    fprintf(fpar,"// channel amplitudes\n");
    for(i = 1; i <= 8; i++)
    {
        fprintf(fpar, "c %d %d\n", i, ReadConf.c[i]);
    }
    
    fprintf(fpar,"\n// Baseaddress\n");
    fprintf(fpar,"a %x\n", BaseAddr);
    fprintf(fpar,"\n// light pulse duration\n");
    fprintf(fpar,"d %d\n", ReadConf.d);
    fprintf(fpar,"\n// set delay of LED pulse after trigger \n");
    fprintf(fpar,"m %d\n", ReadConf.m);
    fprintf(fpar,"\n// include lcd pulse in event: onoff\n");
    fprintf(fpar,"b %d\n", ReadConf.b);
    fprintf(fpar,"\n// onoff external start: 1 for On or 0 for Off to ban external start\n");
    fprintf(fpar,"l %d\n", ReadConf.l);
    fprintf(fpar,"\n// onoff generator and +12V\n");
    fprintf(fpar,"w %d\n", ReadConf.w);
    
    fclose(fpar);

    printf(" ... done\n");
    if(dout) fprintf(dout, " ... done\n");
    
    return 0; // 
}

int lcd::set_config()
{
    int i = 0;
    
    printf("set LCD.Config:"); 
    if(dout) fprintf(dout, "set LCD.Config:"); 
    for(i = 1; i <= 8; i++)
    {
        printf("Setting lcd c %d %d", i, Conf.c[i]);
        if(dout) fprintf(dout, "Setting lcd c %d %d", i, Conf.c[i]);
        fflush(stdout);
        if(dout) fflush(dout);
        set_chan_amp(i, Conf.c[i] );
        printf("... done\n");
        if(dout) fprintf(dout,"... done\n");       
    }
    
    printf("d %d\n", Conf.d);
    
    // d
    printf("Setting lcd d %d", Conf.d);    
    fflush(stdout);    
    if(dout) fprintf(dout, "Setting lcd d %d", Conf.d);
    if(dout) fflush(dout);
    set_lcd_duration(Conf.d);
    printf("... done\n");
    if(dout) fprintf(dout,"... done\n");       
    
    // m
    printf("Setting lcd m %d", Conf.m);    
    fflush(stdout);    
    if(dout) fprintf(dout, "Setting lcd m %d", Conf.m);
    if(dout) fflush(dout);
    set_trigger_delay(Conf.m);
    printf("... done\n");
    if(dout) fprintf(dout,"... done\n");       
 
    // b
    printf("Setting lcd b %d", Conf.b);    
    fflush(stdout);    
    if(dout) fprintf(dout, "Setting lcd b %d", Conf.b);
    if(dout) fflush(dout);
    led_in_event_onoff(Conf.b);
    printf("... done\n");
    if(dout) fprintf(dout,"... done\n");       

    // l
    printf("Setting lcd l %d", Conf.l);    
    fflush(stdout);    
    if(dout) fprintf(dout, "Setting lcd l %d", Conf.l);
    if(dout) fflush(dout);
    ban_external(Conf.l);
    printf("... done\n");
    if(dout) fprintf(dout,"... done\n");       

    // w
    printf("Setting lcd w %d", Conf.w);    
    fflush(stdout);    
    if(dout) fprintf(dout, "Setting lcd w %d", Conf.w);
    if(dout) fflush(dout);
    generator_12_onoff(Conf.w);

    printf("... done\n");
    if(dout) fprintf(dout,"... done\n");       

    return 0; // 
}


// return number of errors in lcd configuration
int lcd::test_config(struct lconfig ReadConf)
{
    int i = 0;
    int panic = 0;
    
    printf("test LCD.Config:"); 
    if(dout) fprintf(dout, "test LCD.Config:"); 
    
    for(i = 0; i <= 8; i++)
    {
        if(ReadConf.c[i] > 255) 
        {
            panic ++;
            printf("Error in c %d %d\n", i, ReadConf.c[i]); 
            if(dout) fprintf(dout, "Error in c %d %d\n", i, ReadConf.c[i]); 
        }
    }
    if(ReadConf.c[8] < 255) 
    {
        printf("Warning in c 8 %d\n", ReadConf.c[8]); 
        if(dout) fprintf(dout, "Warning! in c 8 %d\n", ReadConf.c[8]); 
    }
    
    // 0 < ReadConf.d < 255 
    if((ReadConf.d > 15) && (ReadConf.d < 3))
    {
        panic ++;
        printf("Error in d %d: check d: 3 < d < 15\n", ReadConf.d); 
        if(dout) fprintf(dout, "Error in d %d\n", ReadConf.d); 
    }    
    
    // ReadConf.m <= 15
    if(ReadConf.m > 15) 
    {
        panic ++;
        printf("Error in m %d\n", ReadConf.m);
        if(dout) fprintf(dout, "Error in m %d\n", ReadConf.m); 
    }
    
    // b = 0, 1
    if((ReadConf.b != 1) &&  (ReadConf.b != 0))
    {
        panic ++;
        printf("Error in b %d:\n", ReadConf.b); 
        if(dout) fprintf(dout, "Error in b %d:\n", ReadConf.b); 
    }
    
    // ReadConf.l = 0, 1    
    if((ReadConf.l != 1) &&  (ReadConf.l != 0))
    {
        panic ++;
        printf("Error in l %d:\n", ReadConf.l); 
        if(dout) fprintf(dout, "Error in l %d:\n", ReadConf.l); 
    }
    
    // ReadConf.w = 0, 1
    
    if((ReadConf.w != 1) &&  (ReadConf.w != 0))
    {
        panic ++;
        printf("Error in w %d:\n", ReadConf.w); 
        if(dout) fprintf(dout, "Error in w %d:\n", ReadConf.w); 
    }
    // ------ print summary --------
    printf(" ... done. ");
    if(dout) fprintf(dout, "...  done. ");
    if(panic)
    {
        printf("%d errors in LCD.Config found\n Check %s file \"lcd.config\" !!!! ", panic, LCD_FILE);
	fflush(stdout);
        if(dout) fprintf(dout, "%d errors in LCD.Config found\n Check %s file \"lcd.config\" !!! ", panic, LCD_FILE);
	fflush(dout);
    }
    else
    {
        printf("OK ");
        if(dout) fprintf(dout, "OK ");
    }    
    
    fflush(stdout);
    if(dout) fflush(dout);
    return panic;
}

// return: 0 - OK
//         1 - errors in file reading
//         2 - errors in parameters in file
int lcd::set_config_from_file()
{
    //int ii = 0;
    int err = 0;
    
    if(read_config_from_file() )
    {
	printf("Error in reading \"lcd.config\" file!");
        err ++;	
	return 1;
    }
    print_config(ReadConf);  //  print values
    if(test_config(ReadConf))  // errors in check
    {
        err ++;
	return 2;
    }
    
    import_config(); // write lcd configuration ReadConf to Conf
    set_config();    // set to registers
    save_config();   // save to file
    return 0;
}

    //---------------------------------------------------------
#endif
