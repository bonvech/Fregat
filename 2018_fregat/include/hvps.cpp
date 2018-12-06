/**
 * \file hvps.cpp of fregat
 * \data 01.06.06  modified 2011-02-02: new addressing of hvps
 * \brief Класс hvps 
 * 
 * Определение класса hvps
 */
#ifndef _HVPS
#define _HVPS

#include "i2c.cpp"

class hvps: public i2c
{
    // variables
//    int BaseAddr;
//    int ADCSubAddr2; // hvps  for ReadADCs()

  public:

    int ChanAddr;

    // constructor
    hvps()
    {
        BaseAddr = 0x378;
        ADCSubAddr2 = 0x21;
        VipInit();
        
#ifndef LINUX
        printf("hvps.cpp !!! Linux is not defined!!!");
#endif

#ifndef NO_BOOT
        OnOff_3_3V(1);
#endif
    }

  private:
    // obrashenie k zhelezy
    void SDA(unsigned char Data)
    {
#ifdef LINUX
        if (!(Data)) {outb((inb(BaseAddr+2)  |  2),  (BaseAddr+2));
                      outb((inb(BaseAddr+2)  |  2),  (BaseAddr+2));}
        else         {outb((inb(BaseAddr+2) & (~2)), (BaseAddr+2));
                      outb((inb(BaseAddr+2) & (~2)), (BaseAddr+2));}
#else
        if (!(Data)) {outp((BaseAddr+2), (inp(BaseAddr+2)  |  2));
                      outp((BaseAddr+2), (inp(BaseAddr+2)  |  2));}
        else         {outp((BaseAddr+2), (inp(BaseAddr+2) & (~2)));
                      outp((BaseAddr+2), (inp(BaseAddr+2) & (~2)));}
#endif
    }

    void SCL(unsigned char Data)
    {
#ifdef LINUX
        if (!(Data)) {outb((inb(BaseAddr+2)  | 1),   (BaseAddr+2));
                      outb((inb(BaseAddr+2)  | 1),   (BaseAddr+2));}
        else         {outb((inb(BaseAddr+2) & (~1)), (BaseAddr+2));
                      outb((inb(BaseAddr+2) & (~1)), (BaseAddr+2));}
#else
        if (!(Data)) {outp((BaseAddr+2), (inp(BaseAddr+2)  | 1));
                      outp((BaseAddr+2), (inp(BaseAddr+2)  | 1));}
        else         {outp((BaseAddr+2), (inp(BaseAddr+2) & (~1)));
                      outp((BaseAddr+2), (inp(BaseAddr+2) & (~1)));}
#endif
    }

    unsigned char SDAin(void)
    {       //char tmp = inp(BaseAddr+1);
#ifdef LINUX
        if ((inb(BaseAddr+1) & 0x40))
#else
        if ((inp(BaseAddr+1) & 0x40))
#endif
            { return 1;}
        return 0;
    }

    //////////////////////////////////
//     void SetAddr(unsigned char AddrMUX)
//     {
//         unsigned char tmpDat;
// #ifdef LINUX
//         tmpDat = inb(BaseAddr);
// #else
//         tmpDat = inp(BaseAddr);
// #endif
//         tmpDat &= 0x20;
// #ifdef LINUX
//         outb((tmpDat | AddrMUX), BaseAddr);
// #else
//         outp(BaseAddr, (tmpDat | AddrMUX));
// #endif
//     }

public:

//     void SetChannelAddr(unsigned char ChannelAddr)
//     {
// 	if((ChannelAddr > 55) || (ChannelAddr == 0)) 
// 	{
// 	    printf("\n\n\n\n\n\nERROR IN vip ADRESSATION!!!! ChannelAddr = %i ", ChannelAddr); 
// 	    return;
// 	}
//         ChanAddr =  ChannelAddr;
//         
// #ifdef LINUX
//         outb(((inb(BaseAddr) & 0x40) | ChannelAddr), BaseAddr);
// #else
//         outp(BaseAddr, ((inp(BaseAddr) & 0x40) | ChannelAddr));
// 	printf("hvps.cpp !!! Linux is not defined!!!");
// #endif
//     }
    
    //////////////////////////////////
    void VipInit()
    {
        // 
        outb(0x28, BaseAddr);        
        // reset VIP controller
        outb(0x10, BaseAddr);
        outb(0x00, BaseAddr);
        outb(0x10, BaseAddr);
    }
    
    //////////////////////////////////
    int SetChannelAddr(unsigned char ChannelAddr)
    {
        unsigned char Dev = 0, Reg = 0;
        unsigned char de = 0;
        
        if(ChannelAddr > 63) 
        {
            printf("\n\n\n hvps.cpp: ERROR IN vip ADRESSATION!!!! ChannelAddr = %i ", ChannelAddr); 
            if(dout) fprintf(dout,"\n\n\n hvps.cpp: ERROR IN vip ADRESSATION!!!! ChannelAddr = %i ", ChannelAddr); 
            return 1;
        }
        ChanAddr =  ChannelAddr;
        
        // block lvps
        outb(0x28, BaseAddr); 
        
        // set channel address
        Dev = 0x70 + ChannelAddr/8;
        Reg = 0x8  + ChannelAddr%8;
        //printf("\nAddr = %i, Dev = %xh, Reg = %xh\n", ChannelAddr, Dev, Reg);        
        //if(dout) fprintf(dout,"\nAddr = %i, Dev = %xh, Reg = %xh\n", ChannelAddr, Dev, Reg);        
        if(!TX_reg(Dev, Reg))
        { 
            printf("hvps.cpp: SetChannelAddr ERROR!==> repeat");
            if(dout) fprintf(dout,"hvps.cpp: SetChannelAddr ERROR!==> repeat");
            if(!TX_reg(Dev, Reg))
            { 
                printf("hvps.cpp: ==== DOUBLE ==== SetChannelAddr ERROR! === !!!!!");
                if(dout) fprintf(dout,"hvps.cpp: ==== DOUBLE ==== SetChannelAddr ERROR! === !!!!!");
                return 2;
            }                        
        }                        
        
        // block all microchemes: set no channel selected position to all
        Reg = 0x0; 
        for(de = 0x70; de < 0x78; de ++)
        {
            if(de == Dev) continue; // skip microscheme
                
            //printf("Dev = %xh\n", de);        
            //if(dout) fprintf(dout, "Dev = %xh\n", de);        
            if(!TX_reg(de, Reg))
            { 
                printf("hvps.cpp: ========>SetChannelAddr ERROR! in block all channels Dev=%xh, Reg = %d <=====", Dev, Reg);
                if(dout) fprintf(dout,"hvps.cpp: ========>SetChannelAddr ERROR! in block all channels Dev=%xh, Reg = %d <=====", Dev, Reg);
                if(!TX_reg(de, Reg))
                { 
                    printf("hvps.cpp: ========>SetChannelAddr ERROR! in block all channels Dev=%xh, Reg = %d <=====", Dev, Reg);
                    if(dout) fprintf(dout,"hvps.cpp: ========>SetChannelAddr ERROR! in block all channels Dev=%xh, Reg = %d <=====", Dev, Reg);
                    return 3;
                }            
            }            
        }  
        return 0;
    }  
           
    //////////////////////////////////
    void OnOff_3_3V(unsigned char OnOff)
    {
/*        if (OnOff)
        {
#ifdef LINUX
            outb((inb(BaseAddr) | 0x40), BaseAddr); return;
#else
            outp(BaseAddr, (inp(BaseAddr) | 0x40)); return;
#endif
        }
#ifdef LINUX
        outb((inb(BaseAddr) & (~0x40)), BaseAddr);
#else
        outp(BaseAddr, (inp(BaseAddr) & (~0x40)));
#endif*/
        ;
    }

    //////////////////////////////////////////////////////////////
    unsigned char ReadDAC(unsigned char AddrDev, unsigned char *Data)
    //read Data from DAC whit addr AddrDev
    {
        unsigned char tmpAddr;
        if (AddrDev==0) {tmpAddr = 0x2C;}
        else if (AddrDev==1) {tmpAddr = 0x2D;}
                else {return 0;}
        return RX8(tmpAddr,Data);
    }


    /////////////////////////////
    unsigned char ReadResRegs(unsigned char AddrDev, unsigned int *ResData[4])
    {

        if(!TX8(AddrDev, 0x00, 0x0F))
            return 0;

        return RX64(AddrDev, ResData);
    }
    
    /////////////////////////////
    unsigned char Read1ResReg(unsigned char AddrDev, unsigned char AddrReg, unsigned int *RegRes)
    {
        unsigned char tmpAddr = AddrReg;

        //tmpAddr = 1 << (tmpAddr-1);

        printf(" Addr=%d", tmpAddr);
        if(!TX8(AddrDev, 0x00, tmpAddr))
        {
            printf("Error in Read1ResReg!!");
            return 0;
        }

        return RX16(AddrDev, RegRes);
    }

};  // end of class hvps

#endif
