// 05.02.08
/**
 *  \file hvpsdev.cpp
 *  \brief Класс hvps_dev. Температура мозаики.
 * 
 *  Чтение температуры мозаики
 */
#ifndef _HVPS_DEV
#define _HVPS_DEV

#include "i2c.cpp"
#include "hvps.cpp"

class hvps_dev: public hvps
{
public:
    //int ChanAddr;

    // constructor
    hvps_dev()
    {
        BaseAddr = 0x378;
        ADCSubAddr2 = 0x21;
    }

///////////////////////////////////////////////////////////////
public:
    /** -------------------------------------------------------
    *  \brief read_fadc_temp \par 
    *  Чтение температуры FADC (мозаики) and print it to debug file
    */
    float read_fadc_temp(unsigned int addr)
    {
        unsigned int Res1 = 0;
        float Tem1 = 0.;

        if(!RX16(addr, &Res1))
        {
            printf("Error in T RX16 reading!!!!");
            return 0;
        }
        
        Tem1 = kod_2_fadc_temp(Res1);
        //printf("T = %6.2f oC", Tem1); // --- lena
        if(dout) fprintf(dout, "T=%ikod=%6.2foC", Res1, Tem1);
        return Tem1;
    }

private:
    /** -------------------------------------------------------
    *  \brief kod_2_fadc_temp \par 
    *  Перевод температуры FADC из кода АЦП в градусы
    */
    float kod_2_fadc_temp(unsigned int kod)
    {
        float temp = 0.;
        short int skod = 0;

        skod = (short int) kod;
        //printf("\n    T1 %i", skod);
        skod >>= 6;
        //printf("    T1 %i", skod);
        temp = (float)skod;
        temp /= 4.;

        return temp;
    }
    
public:
    /** -------------------------------------------------------
    *  \brief read_mosaic_temp \par 
    *  Чтение температуры мозаики and print it to string message.
    */
    unsigned int read_mosaic_temp(char *message)
    {
        float Tem1 = 0.;
    
        SetChannelAddr(62); // adress of mosaic temperature
        //SetChannelAddr(61); // adress of mosaic temperature
        
        //printf("\nread_mosaic_temp: "); // -- lena
        if(dout) fprintf(dout, "\nread_mosaic_temp: ");
        Tem1 = read_fadc_temp(0x49);
        //printf("\nread_mosaic_temp: %3.2f oC\n\n", Tem1);
        // p.publichenko
        sprintf(message, "Tm = %6.2f oC", Tem1);
        return 0;
    }
     
};  // end of class hvps_dev
#endif
