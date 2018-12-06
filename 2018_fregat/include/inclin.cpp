// inclin.cpp
// 05.02.08

#ifndef _INCL
#define _INCL

#define PI 3.14159
#define AL 1.3
#define BE 0.7
//#define CHADDR_INCL 0  // ChannelAddres on vip plate
#define CHADDR_INCL 63  // ChannelAddres on vip plate
#define ADDR_INCL 0x50 // 

#include "i2c.cpp"
#include "hvps.cpp"

class inclinometer: public hvps
{
  public:

    int ChanAddr;
    float Theta;

    // constructor
    inclinometer()
    {
        BaseAddr    = 0x378;
        ADCSubAddr2 = 0x21;
        SetChannelAddr(CHADDR_INCL); // adress of inclinometer
    }

 private:

    void SCL(unsigned char Data)
    {
        int t = 0;
#ifdef LINUX
        if (!(Data)) {outb((inb(BaseAddr+2)  | 1),   (BaseAddr+2));
                      outb((inb(BaseAddr+2)  | 1),   (BaseAddr+2));
                      //usleep(1);
                      for(t = 0; t <= 10000; t++);
                      }
        else         {outb((inb(BaseAddr+2) & (~1)), (BaseAddr+2));
                      outb((inb(BaseAddr+2) & (~1)), (BaseAddr+2));
                      for(t = 0; t <= 10000; t++);
                      //usleep(1);
                     }
#else
        if (!(Data)) {outp((BaseAddr+2), (inp(BaseAddr+2)  | 1));
                      outp((BaseAddr+2), (inp(BaseAddr+2)  | 1));}
        else         {outp((BaseAddr+2), (inp(BaseAddr+2) & (~1)));
                      outp((BaseAddr+2), (inp(BaseAddr+2) & (~1)));}
#endif
    }
    
//////////////////////////////////////////////////////////////

public:
    //---------------------------------------------------------
    unsigned int read_incl_angle(char *message)
    {
        unsigned int Res = 0;
        unsigned char addr = ADDR_INCL;

        SetChannelAddr(CHADDR_INCL); // adress of inclinometer

        //printf("Res0 = %i = %xh\n", Res, Res);
        if(!TX_reg(addr, 7)) // AADDR = 0x50
        {
            printf("read_inclin: Error in TX_reg putting!!!!\n");
            if(dout) printf("read_inclin: Error in TX_reg putting!!!!\n");
            return 0;
        }
        if(!RX32(addr, &Res)) // AADDR = 0x50
        {
            printf("read_inclin:Error in T RX32 reading!!!!");
            if(dout) printf("read_inclin:Error in T RX32 reading!!!!");
            return 0;
        }
        output_incl_angle(Res, message);

        return Res;
    }

    //////////////////////////////////////////////
    void printf_incl_angle(unsigned int Res)
    {
        short ang1 = 0;
        short ang2 = 0;
        float fang1 = 0., fang2 = 0.;

        Conv.tInt = Res;
        //printf("Res1 = %i = %xh\n", Res, Res);
        ang1 = Conv.tChar[3] * 256 + Conv.tChar[2];
        ang2 = Conv.tChar[1] * 256 + Conv.tChar[0];

        //printf("Angle1 = %5d = %x    Angle2 = %5d = %x    grad E-3\n", ang1, ang1, ang2 , ang2);
        //if(dout) fprintf(dout, "Angle1 = %5i[kod]   Angle2 = %5i grad E-3", ang1 , ang2);
        fang1 = (float)ang1 * 0.001;
        fang2 = (float)ang2 * 0.001;
        fang1 -= AL;
        fang2 -= BE;

        printf("Inclination:   %2.1f  %2.1f grad\n", fang1, fang2);
        if(dout) fprintf(dout,"Angles:   %2.1f  %2.1f grad\n", fang1, fang2);
    }

    //////////////////////////////////////////////
    void output_incl_angle(unsigned int Res, char *message)
    {
        short ang1 = 0;
        short ang2 = 0;
        float fang1 = 0., fang2 = 0., ftheta = 0.;

        Conv.tInt = Res;
        //printf("Res1 = %i = %xh\n", Res, Res);
        ang1 = Conv.tChar[3] * 256 + Conv.tChar[2];
        ang2 = Conv.tChar[1] * 256 + Conv.tChar[0];

        fang1 = (float)ang1 * 0.001;
        fang2 = (float)ang2 * 0.001;
        fang1 -= AL;
        fang2 -= BE;
        ftheta = calc_mosaic_vector(fang1, fang2);

        //strcpy( message, "");
        sprintf(message, "Clin: %5.1f  %5.1f gr  Th: %5.1f gr\n", fang1, fang2, ftheta);
    }

    //////////////////////////////////////////////
    int read_eeprom_0a(void)
    {
        unsigned char Res = 0;
        unsigned char addr = ADDR_INCL;

        if(!TX_reg(addr, 4)) // AADDR = 0x50
        {
            printf("read_eeprom: Error in TX_reg putting!!!!  1\n");           
            if(dout) printf("read_eeprom: Error in TX_reg putting!!!! 1\n");           
            return 0;
        }
        if(!RX8(addr, &Res)) // AADDR = 0x50
        {
            printf("read_eeprom: Error in T RX8 reading!!!! 1");           
            if(dout) printf("read_eeprom: Error in T RX8 reading!!!! 1");           
            return 0;
        }
        printf("Res = %u \n", Res);

        if(!TX_reg(addr, 0x0A)) // AADDR = 0x50
        {
            printf("read_eeprom: Error in TX_reg putting!!!!  2\n");           
            if(dout) printf("read_eeprom: Error in TX_reg putting!!!! 2\n");           
            return 0;
        }
        if(!RX8(addr, &Res)) // AADDR = 0x50
        {
            printf("read_eeprom: Error in T RX8 reading!!!! 2");           
            if(dout) printf("read_eeprom: Error in T RX8 reading!!!! 2");           
            return 0;
        }
        printf("Res = %u \n", Res);
        return 0;
    }


  float calc_mosaic_vector(double alpha, double betta)
  {
    double theta = 0., phi = 0.;
    double xx = 0.,    xy = 0.;

    //alpha -= AL;
    //betta -= BE;

    alpha *= PI / 180.;
    betta *= PI / 180.;

    xx = tan(alpha);
    xy = tan(betta);

    theta = 1. + xx*xx + xy*xy;
    theta = sqrt(theta);
    theta = 1. / theta;
    theta = acos(theta);

    if(xx  != 0.0) 
    {
        phi = xy/xx;
        phi = atan(phi);
        phi -= 2.5;
        if(alpha < 0)   phi += PI;
    }
    else phi = 0.;

    theta *= 180./PI;
    phi   *= 180./PI;
    //printf("\ntheta = %f grad,    phi = %f grad\n\n", theta, phi);
    return theta;
  }
};  // end of class inclinimeter

#endif
