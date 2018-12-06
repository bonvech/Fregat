/**
 * \file hvpssipm.cpp (old hvpstest.cpp)
 * \brief Класс hvps_test. Источники высоковольтного напряжения питания ФЭУ. 
 * \date  november 2018
 * 
 * Class hvps_test. New version for one VIP for SiPM.
 */
#include "hvcalibr.h"
/// Dimension of hv arrays from 112 rediced to 1
#define HVDIM 112
/// max kod to high voltage DAC
#define HIGHMAX 254 //169 // !!! 254 - maximum!!!
/// min kod to high voltage DAC
#define HIGHMIN 100
/// max kod to high voltage DAC to HAMAMATSU
#define H_HIGHMAX 150 //169 // !!! 254 - maximum!!!
/// min kod to high voltage DAC to HAMAMATSU
#define H_HIGHMIN 100

/// Address of SiPM VIP
#define VIP_ADDR 26
/// SubAddress of SiPM VIP
#define VIP_SUBADDR 0

extern struct input_parameters Work;

const int CurKoef = 100;

class hvps_test : public hvps
{

public:
    unsigned char SubAddr;
    unsigned char On;
    //private:
    unsigned char highv[HVDIM];   ///< value of High voltage
    unsigned char hvwork[HVDIM];  ///< marker of channel OnOff
    unsigned int  pmt_cur[HVDIM]; ///< array of currents in channels: int(cur * CurKoef)
  
public:
    hvps_test() // constructor
    {
        int ii = 0;
        
        hvps();
        SubAddr = 0;
        On = 0;
        self_test();
        for(ii = 0; ii < HVDIM; ii++)
        {
            pmt_cur[ii] = 0;
        }
    }

    /** -------------------------------------------------------
    *  \brief Set SubAddress
    * 
    */
    void SetSubAddr(unsigned char addr)
    {
        SubAddr = addr;
    }

    
    /** -------------------------------------------------------
    *  \brief test high  voltage channels
    *  \date november 2018
    * 
    *  Проверяется возможность и правильность записи и чтения в каналы ЦАП
    */
    int self_test()
    {
        unsigned char ii = 0;
        unsigned char kod = 0;
        unsigned char err[112] = {0};
        int Hvchan = HVDIM;     Hvchan = Work.hvchan;

        /// -- write numbers to channels
        printf("\nHV Channels Self-Test: write numbers to HV channels:\n");
        if(dout) fprintf(dout, "\nHV Channels Self-Test: write numbers to HV channels:\n");
        //ii = ADDR * 2 + SUBADDR;
        for(ii = 0; ii < Hvchan; ii++)
        {
            //printf("\n ii=%2i--%2i--%i ", ii, (int)ii/2, ii%2);
            //if(dout) fprintf(dout, "\n ii=%2i-%2i-%i", ii, (int)ii/2, ii%2);

            if(SetChanHigh(ii, ii))  // set high = 0
            {   // error
                printf("self_test: Error in DAC`s transmittion chan = %i", ii);
                if(dout) fprintf(dout, "self_test: Error in DAC`s transmittion chan = %i", ii);
            }
        }

        // ------- read numbers
        /// -- read actual high value from chanels
        if(stdout) fprintf(stdout, "\n\nREAD numbers from HV channels:\n");
        if(  dout) fprintf(  dout, "\n\nREAD numbers from HV channels:\n");
        //ii = ADDR * 2 + SUBADDR;
        for(ii = 0; ii < Hvchan; ii++)
        {
            if((ii+1)%8 == 0) printf("\n");
            //SetChannelAddr(int(ii/2)); // 2011 year
            //SetSubAddr(ii%2);
            SetChannelAddr(26); // !!! for SiPM in 2018 year
            SetSubAddr(0);       // !!! for SiPM in 2018 year

            //printf("\n ii=%2i--%2i--%i ", ii, ii/2, ii%2);
            printf(" [%3i]", ii);
            if(dout) fprintf(dout, "\n ii=%2i-%2i-%i", ii, ii/2, ii%2);

            if(!ReadDAC(SubAddr, &kod))
            {
                //if(stdout) fprintf(stdout, " self_test: ReadDAC failed in recieve ");
                if(  dout) fprintf(  dout, " self_test: ReadDAC failed in recieve ");
                //return 1; // error in read DAC
                kod = 200;
            }
            else
            {
                //printf("= %3i", kod);
                if(dout) fprintf(dout," kod = %3i", kod);
            }

            // diagnostics
            if(ii == kod)
            {
                printf("  OK");
                if(dout) fprintf(dout," OK");
            }
            else
            {
                printf(" ERR \n");
                if(dout) fprintf(dout," ERR");
                err[0] ++;
                err[err[0]] = ii;
            }
        }

        /// -- write 0 to channels -------------
        printf("\n\nwrite 0 to HV channels:\n");
        if(dout) fprintf(dout, "\n\nwrite 0 to HV channels:\n");
        for(ii = 0; ii < Hvchan; ii++)
        {
            //printf("\n ii=%2i--%2i--%i ", ii, (int)ii/2, ii%2);
            //if(dout) fprintf(dout, "\n ii=%2i-%2i-%i", ii, (int)ii/2, ii%2);
            if(SetChanHigh(ii, 0))  // set high = 0
            {   // error
                printf("self_test: Error in DAC`s transmittion chan = %i", ii);
                if(dout) fprintf(dout, "self_test: Error in DAC`s transmittion chan = %i", ii);
            }
        }

        /// -- print diagnostics results
        printf("\n\nHV Channels Self-Test: ");
        if(dout) fprintf(dout, "\n\nHV Channels Self-Test: ");
        if(err[0])  // there are errors in channels
        {
            printf("Errors in %i channels:", err[0]);
            if(dout) fprintf(dout, "Errors in %i channels:", err[0]);
            for(ii = 1; ii <= err[0]; ii++)
            {
                printf(" %2i", err[ii]);
                if(dout) fprintf(dout, " %2i", err[ii]);
            }
        }
        else  // -- no errors
        {
            printf(" Success");
            if(dout) fprintf(dout, " Success");
        }
        printf("\n\n");
        if(dout) fprintf(dout, "\n\n");

        return err[0];
    }

    
    /** -------------------------------------------------------
     *  \brief set voltage high to one channel with Addr = ii/2 and SubAddr = ii%2
     */
    int SetChanHigh(unsigned char ii, unsigned char high)
    {
        unsigned char kod = 0;

        SetChannelAddr(int(ii/2)); // 2011 year
        SetSubAddr(ii%2);
        if(dout) fprintf(dout, "\n ii=%2i-%2i-%i", ii, (int)ii/2, ii%2);

        // ---- read actual high value ----
        if(!ReadDAC(SubAddr, &kod))
        {
            printf("\n ii=%2i--%2i--%i ", ii, (int)ii/2 , ii%2);
            printf(" SCH: ReadDAC failed in recieve, kod set to 0. ");
            if(dout) fprintf(dout, " SCH: ReadDAC failed in recieve, kod set to 0. ");
            kod = 0;
        }
//       else
//       {
//           printf("DAC[%3i]", kod);
//           if(dout) fprintf(dout, "DAC[%3i]", kod);
//       }

        //printf(" high = %i", high);
        if(dout) fprintf(dout, " high = %i", high);
        if( (high - kod) > 5) // new value is biggest
        {
            if(  dout)  fprintf(  dout, " sleep");
            if(stdout) {fprintf(stdout, " sleep"); fflush(stdout);}
            usleep(500000);
        }

        // --- set high
        if(!WriteDAC(SubAddr, high))  // set high kods
        {
                //printf("ER: ii=%2i  adr=%2i  sad=%i need=%i === ", ii, ChanAddr, SubAddr, need[ii]);
                printf(" SCH: Error in DAC`s transmittion - set high!!");
                if(dout) fprintf(dout, " SCH: Error in DAC`s transm\n");
                return 2;  // error in write to DAC
        }
        return 0; // no errors
    }

    
    /** -------------------------------------------------------
    *  \brief measure high in all work channels
    *  \date 2018.11 changed to one channel (2010.03.11)
    */
    int  measure_all_high()
    {
        unsigned char ii = 0;
        //int Hvchan = HVDIM;  Hvchan = Work.hvchan;

        printf("\nMeasure high\n");
        if(dout) fprintf(dout, "\nMeasure high\n");
        
        ii = VIP_ADDR * 2 + VIP_SUBADDR;
        //for(ii = 0; ii < Hvchan; ii++)
        {
            //if( !hvwork[ii] )  continue;
            SetChannelAddr(int(ii/2));
            SetSubAddr(ii%2);

            if(dout) fprintf(dout,   "i = %2i--%2i--%i: ", ii, ChanAddr, SubAddr);
            printf("i = %2i: ", ii);
            //printf("i = %2i--%2i--%i: ", ii, ChanAddr, SubAddr);
            measure_high();
        }
        return 0;
    }

    
/** -------------------------------------------------------
*  \brief set optimal high voltage in all vip
*  \date 2018.11 
*/
int high()
{
    unsigned char high = 0, highset = 0;
    //unsigned char h_high = 0; // 2012: for Humamatsu
    unsigned char flag = 0, ii = 0;
    unsigned char need[HVDIM] = {0};
    unsigned highmax = HIGHMAX;
    unsigned highmin = HIGHMIN;
    //unsigned h_highmax = H_HIGHMAX; // 2012: for Humamatsu
    //unsigned h_highmin = H_HIGHMIN; // 2012: for Humamatsu
    float cur = 0;
    float Workcur = 0;
    //int H_Workcur = 0;
    //int Hvchan = HVDIM;
    char debug[230]={""};

    On = 1;
    //Hvchan = Work.hvchan;
    //h_highmax = Work.h_umax;
    //h_highmin = Work.h_umin;
    highmax = Work.umax;
    highmin = Work.umin;

    cur     = Work.workcur;
    Workcur = Work.workcur; //I_to_kod(Work.workcur);
    sprintf(debug, "Workcur = %.1f mA,\n",  Workcur );
    print_debug(debug);

    //cur = Work.h_workcur;
    //H_Workcur = I_to_kod(Work.h_workcur);
    //printf("H_Workcur = %i [kod]  = %i mkA,\n",  H_Workcur, cur);
    //if(dout) fprintf(dout, "H_Workcur = %4i kod  =  %i mkA \n", H_Workcur, cur);

    // -------- turn on all HV  -----------
    printf("Turn on all\n");
    if(dout) fprintf(dout, "Turn on all\n");
    
    ii = VIP_ADDR * 2 + VIP_SUBADDR;

    //for(ii = 0; ii < Hvchan; ii++)
    {
        need[ii]   = 1;
        highv[ii]  = 0; //code value of high voltage
        hvwork[ii] = 1; // flag onoff

        if(SetChanHigh(ii, 0))  // set high = 0
        {   // error
            printf("set high: Error in DAC`s transmittion\n ");
            if(dout) fprintf(dout,"set high: Error in DAC`s transmittion\n ");
            need[ii]   = 0;
            hvwork[ii] = 0;
        }
        else
        {   // turn on
            if(!TX8(0x20+SubAddr,0x02,0x09)) // turn on
            {
                sprintf(debug, "set high: Error in ADC`s setting ---> NO ON\n");
                print_debug(debug);
                need[ii]   = 0;
                hvwork[ii] = 0;
                // ??? iskluchit iz triggera hvtrig[ii] = 0;
            }
            else
            {
                printf(" +> ON");
                if(dout) fprintf(dout, "+>ON");
            }
        }
        fflush(stdout);
        sleep(1); //  1 sec after each turn on
    }
    sleep(10);

    /// -----------------------
    measure_all_high();  // measure high in all work channels

    // -----------------------
    // search optimal high to provide current =  workcur by increasing high
    sprintf(debug, "--------------------------------------------------\n\n!!!CHANGE high\n");
    print_debug(debug);

    // --- change high voltage ----
    //for (high = 169; high <= HIGHMAX; high++)
    //for (high = highmin; high <= highmax; high++, h_high++)
    for (high = highmin; high <= highmax; high++) // 14.02.12
    {
        printf("\n---------------------------------\n");
        printf("++ HIGH = %3i\n", high);
        if(dout) fprintf(dout,"\n---------------------------------\n");
        if(dout) fprintf(dout,"++ HIGH = %3i", high);

        /// -- set high --
        ii = VIP_ADDR * 2 + VIP_SUBADDR;
        //for(ii = 0; ii < Hvchan; ii++)
        {
            if(!need[ii])  continue;

            highset = high;

            if(SetChanHigh(ii, highset))  // set high
            {
                //need[ii] = 0;
                printf("ER: ii=%2i  adr=%2i  sad=%i need=%i ===> ", ii, ChanAddr, SubAddr, need[ii]);
                if(dout) fprintf(dout,"ER: ii=%2i  adr=%2i  sad=%i need=%i ===> ",
                    ii, ChanAddr, SubAddr, need[ii]);
           }
        }

        /// -- measure current --
        flag = 0;
        printf("Currents: [mA]\n");
        //printf("     0[uA]  1[uA]  2[uA]  3[uA]  4[uA]  5[uA]  6[uA]  7[uA]  8[uA]  9[uA]");
        ii = VIP_ADDR * 2 + VIP_SUBADDR;
        //for(ii = 0; ii < Hvchan; ii++)
        {
            if(ii%10 == 0)    printf("\n%3d", ii/10);
            if( !need[ii] )    
            {
                printf("      ");
                continue;
            }

            SetChannelAddr(int(ii/2));
            SetSubAddr(ii%2);
            //printf("\nii=%2i--%2i--%i ", ii, ChanAddr, SubAddr);           
            if(dout) fprintf(dout,"\ni=%2i--%02i--%i ", ii, ChanAddr, SubAddr);
            cur = measure_current();
            pmt_cur[ii] = int(cur*CurKoef);

            sprintf(debug, " cur = %7.3f mA", cur);  
            print_debug(debug); 

            // -- test current --
            if(cur > Workcur )
            {
                need[ii]  = 0;      // stop
                highv[ii] = high;   //
            }
            else flag++ ;
            if(high > 254)       highv[ii] = 254;
            if(high >= highmax)  highv[ii] = highmax;
           
        }
        printf("\n ==> flag = %3i\n", flag);
        if(dout) fprintf(dout,"\n ==> flag = %3i\n", flag);
        // check need
        if( !flag )
            break;
    }

    // -----------------------
    // measure high in all work channels
    printf(               "--------------------------------------------------\n");
    if(dout) fprintf(dout,"--------------------------------------------------\n");
    measure_all_high();

    ii = VIP_ADDR * 2 + VIP_SUBADDR;
    //for(ii = 0; ii < Hvchan; ii++)
    {
        //if(!hvwork[ii]) continue;
        if(highv[ii] == 0)    hvwork[ii] = 0;
    }
    // print results:
    printf(               "--------------------------------------------------\n");
    if(dout) fprintf(dout,"--------------------------------------------------\n");
    printf(                "\n Results:");
    if(dout) fprintf(dout, "\n RESULTS:_");
    ii = VIP_ADDR * 2 + VIP_SUBADDR;
    //for(ii = 0; ii < Hvchan; ii++)
    {
        printf(                "\ni = %3i--%2i--%1i, high = %3i", ii, ii/2 + 1, ii%2, highv[ii]);
        if(dout) fprintf(dout, "\ni = %2i--%2i--%1i, high = %3i", ii, ii/2 + 1, ii%2, highv[ii]);
        if(!hvwork[ii])
        {
            printf(" no work");
            if(dout) fprintf(dout, "NoWork");
        }
    }
    print_currents(stdout);
    return 0;
}


/** -------------------------------------------------------
 *  \brief high scout
 *  \return 0 - OK\par 
 *          1 - high illumination\par
 *          2 - error in channel
 */
int high_scout(unsigned char Addr, unsigned char SubAddr)
{
    float cur = 0, Maxcur = 0;
    char  debug[250] = {""};

    sprintf(debug, "Reconnaissance: \n"); 
    print_debug(debug);
    if( (Addr > 55) || (SubAddr > 1))
    {
        sprintf(debug, "Bad channel numbers \n"); 
        print_debug(debug);
        return 2;
    }

    /// -- turn on HV on one channel
    sprintf(debug, "Turn on \n"); 
    print_debug(debug);

    SetChannelAddr( Addr);
    SetSubAddr(SubAddr);

    sprintf(debug, " adr=%2i  sad=%i\n", ChanAddr, SubAddr ); 
    print_debug(debug);

    if(!WriteDAC(SubAddr, 0))  // set high==0 kod
    {
        sprintf(debug, " Error in DAC`s transmit: NO set high==0kod "); 
        print_debug(debug);
        return 2;
    }
    if(!TX8(0x20+SubAddr,0x02,0x09)) // turn on
    {
        sprintf(debug, " Error in ADC`s setting: NO turn on "); 
        print_debug(debug);
        return 2;
    }

    sleep(10);
    measure_high();

    Maxcur = Work.maxcur;; // !!!2018  I_to_kod(Work.maxcur);
    sprintf(debug, "Maxcur  = %.3f mA\n", Maxcur);
    print_debug(debug);

    cur = measure_current();
    sprintf(debug, "I = %4.2f mkA\n", cur); 
    print_debug(debug);

    if(cur > Maxcur ) // current > MAXCUR
    {
        sprintf(debug, "\n===> ERROR:  Current > MAXCUR \n");
        print_debug(debug);
        return 1;
    }
    sprintf(debug, "\n===> SUCCES:  Current < MAXCUR \n");
    print_debug(debug);
    return 0;
}

 
/** -------------------------------------------------------
 *  \brief measure_high in one actual channel
 *  \return: 0 - Error in ADC`s reading\par 
 *           1 - OK
 */
unsigned char measure_high(void)
{
    unsigned char  iii = 0; 
    unsigned int   MData[4] = {1,1,1,1};
    unsigned int   kod[10]; 
    float Up = 0., I = 0.;
    float T = 0.;
    char debug[250];

    /// -- read ADC
    if(!ReadADCs(SubAddr,(unsigned int *)&MData))  // read ADC
    {
        printf("Error in ADC`s reading\n");
        if(dout) fprintf(dout, "!measure_high: Error in ADC`s reading\n");
        return 0;
    }
    else
    {
        for(iii=0; iii<4; iii++)
        {
            MData[iii] &= 0x0FFF; // set 4 major bit to 0
            kod[iii]   = MData[iii];
        }
    }
    sprintf(debug, "\nMeasure_high: current: %d %d %d %d", kod[0], kod[1], kod[2], kod[3]);
    print_debug(debug);

    /// -- calculate Up, I, T
    Up  = kod_to_Up(kod[0]);
    I   = kod_to_I(kod[1], kod[2]);
    T   = kod_to_T(kod[3]);

    sprintf(debug, " Up =%6.2f V  Ianode = %7.3f mA T = %4.1f oC\n", Up, I, T);
    print_debug(debug);
    
    return 1;
}

    /** -------------------------------------------------------
    *  \brief read_vip_ADC \par 
    *  Read vip ADC and write to debug file and string message
    */
    unsigned int read_vip_ADC(char *message)
    {
        unsigned char  iii = 0; 
        unsigned int   MData[4] = {1,1,1,1};
        unsigned int   kod[10]; 
        float Up = 0., I = 0.;
        float T = 0.;
        char debug[250];
            
        /// -- read ADC
        SetChannelAddr(26); // adress of ADC of vip         
        if(!ReadADCs(0,(unsigned int *)&MData))  // read ADC
        {
            sprintf(debug, "!read_vip_ADC: Error in ADC`s reading\n\n");
            print_debug(debug);
            return 1;
        }
        else
        {
            for(iii=0; iii<4; iii++)
            {
                MData[iii] &= 0x0FFF; // set 4 major bit to 0
                kod[iii] = MData[iii];
            }
        }
        
        sprintf(debug, "\nread_vip_ADC: CH0[%3i] CH1[%3i] CH2[%3i] CH3[%3i]\n\n", 
                                kod[0], kod[1], kod[2], kod[3]);
        print_debug(debug);
       
        /// -- calculate Up, I, T
        Up  = kod_to_Up(kod[0]);
        I   = kod_to_I(kod[1], kod[2]);
        T   = kod_to_T(kod[3]);

        sprintf(debug, "Up =%6.2f V  Ianode = %7.3f mA T = %4.1f oC\n", Up, I, T);
        print_debug(debug);
        strcpy(message, debug);

        return 0;
    }


/** -------------------------------------------------------
 *  \brief measure current in one channel
 *  \date november 2018
 *  \return: curr - float current            
 */
float measure_current(void)
{
    unsigned int   MData[4] = {1,1,1,1};
    char debug[230];

    //------- read ADC -----
    if(!ReadADCs(SubAddr,(unsigned int *)&MData))  // read ADC
    {
        printf("Error in ADC`s reading\n");
        if(dout) fprintf(dout,"!measure_current: Error in ADC`s reading\n");
        return 0;
    }
    for(int iii = 0; iii < 4; iii++)
    {
        MData[iii] &= 0x0FFF; // set 4 major bit to 0
    }
    sprintf(debug, "\nMeasure current: %d %d %d %d", MData[0], MData[1], MData[2], MData[3]);
    print_debug(debug);
    return kod_to_I(MData[1], MData[2]);
}


/** -------------------------------------------------------
 *  \brief check current
 *  \return 0 - O'k
 *          1 - some channel was off
 *          2 - some channel was off and now no enough channels to trigger           
 */
int check_current()
{
    unsigned char ii = 0, flag = 0, n_work = 0;
    //unsigned int  Hvchan = HVDIM;
    float   cur = 0;
    float   Maxcur = 0; //, H_Maxcur = 0;  // max current in mA
    float   tok = 0.0;
    char    debug[250];

    //Hvchan = Work.hvchan;
    cur    = Work.maxcur;
    Maxcur = Work.maxcur; // !!!2018 I_to_kod(Work.maxcur);
    printf(                "\n < CHECK CURRENT: Maxcur = %.3f mA", Maxcur);
    if(dout) fprintf(dout, "\n < CHECK CURRENT: Maxcur = %.3f mA", Maxcur);

    /// -- measure current and off high if current > Maxcur   
    ii = VIP_ADDR * 2 + VIP_SUBADDR;
    //for(ii = 0; ii < Hvchan; ii++)
    {
        if(ii%10 == 0)    printf("\n%3d", ii/10);
        if( !hvwork[ii] ) 
        {
             printf("\n\n!!!! TERRIBLE ERROR!!!  Do not work !!!!!!!\n     ");
             //continue;
        }

        /// --- set channel address
        SetChannelAddr(int(ii/2));
        SetSubAddr(ii%2);

        /// --- measure current and print
        cur = measure_current();
        pmt_cur[ii] = int(cur*CurKoef);
        tok = cur; // !!!2018 kod_to_I(cur);
        
        printf(" %7.3f ", tok);  //kod_to_I(cur));
        if(dout)  fprintf(dout,  "\n ii = %2i cur = %7.3f mA",ii,  tok);  //kod_to_I(cur));
               
        if(ffmin) fprintf(ffmin, "\n C %2i %4.2f", ii, tok);  //kod_to_I(cur));
        
        /// -- check current > Maxcur
        if( cur > Maxcur  ) // current > MAXCUR
        {
            highv[ii]  = 0;  // set high = 0
            hvwork[ii] = 0;  // off channel
            printf(                " ===> ER:  Current > MAXCUR  => high_off\n");
            if(dout) fprintf(dout, " ===> ER:  Current > MAXCUR  => high_off\n");

            // set high = 0
            if(!WriteDAC(SubAddr, highv[ii]))  // set high = 0
            {
                printf("ER: ii=%2i  adr=%2i  sad=%i ===> ", ii, ChanAddr, SubAddr);
                printf("Error in DAC`s transmittion\n");
                if(dout) fprintf(dout,"ER: ii=%2i  adr=%2i  sad=%i ===> ", ii, ChanAddr, SubAddr);
                if(dout) fprintf(dout, "Error in DAC`s transmittion\n");
                sleep(1); //1 sec
            }
            flag++; // number of channels to OFF
        }
        else n_work ++; // number of ON channels
    }
    fflush(ffmin);
    
    sprintf(debug, "CHECK CURRENT />");
    print_debug(debug);

    /// -- if no channels to off - return 0=0k
    if( !flag ) return 0;

    sprintf(debug, "CHECK CURRENT: %2i channels to trigger\n", n_work);
    print_debug(debug);

    if( (n_work >= Work.master) || ( n_work >= Work.gmaster))
        return 1; // there are channels to run

    // --------------------
    // if no enough channels to trigger
    sprintf(debug, "CHECK CURRENT: NO enough channels to trigger!! Turn_off!!!\n");
    print_debug(debug);
    turn_off();
    return 2; // no enough channels to trigger
  }

  
/** -------------------------------------------------------
 *  \brief print currents to file *ff
 */
int print_currents(FILE *ff)
{
    int  ii  = 0;
    int  tok = 0.0;
    
    fprintf(ff,"\nCurrents:\n");
    //fprintf(ff,"     0[uA]  1[uA]  2[uA]  3[uA]  4[uA]  5[uA]  6[uA]  7[uA]  8[uA]  9[uA]");
    ii = VIP_ADDR * 2 + VIP_SUBADDR;
    //for(ii = 0; ii < HVDIM; ii++)
    {
        if(ii%10 == 0)    printf("\n%3d", ii/10);
        if( !hvwork[ii] ) 
        {
            fprintf(ff,"       ");
            //continue;
        }
        //tok = kod_to_I(pmt_cur[ii]);
        tok = pmt_cur[ii];        
        fprintf(ff," %7.3f mA", float(tok)/CurKoef);
    }
    return 0;
}


/** -------------------------------------------------------
 *  \brief print currents to binary file
 */
int print_currents_to_binary(FILE *ff)
{
    int  ii = 0;
    
    fprintf(ff,"i");
    //ii = VIP_ADDR * 2 + VIP_SUBADDR;
    for(ii = 0; ii < HVDIM; ii++)
    {
        Conv.tInt = pmt_cur[ii];
        fprintf(ff,"%c%c%c",  Conv.tChar[2], Conv.tChar[1], Conv.tChar[0]);
    }
    fflush(ff);
    return 0;
}

////////////////////////////////////////////
/*  unsigned char read_blocks(void)
  {
    unsigned int  Data[4];
    unsigned char Dat = 0;
    unsigned int  tmpp = 0;
    unsigned char ii = 0;

    printf("\n#");
    if(dout) fprintf(dout, "\n#");
    for(ii = 0; ii < 4; ii++)  Data[ii] = 0;

    ///////////////////////
    if(!ReadADCs(SubAddr,(unsigned int *)&Data))
    {
        printf("Error in ADC`s reading\n");
        if(dout) fprintf(dout, "!read_bloks: Error in ADC`s reading\n");
        return 0 ;
    }

    ///////////////////////
    tmpp = Data[0];
    tmpp &= 0x0FFF; // set 4 major bit to 0

    if(On)
        { printf("ON");
          if(dout) fprintf(dout,"ON");}
    else
        { printf("OFF");
          if(dout) fprintf(dout,"OFF");}

    printf("    SubAddr = %1i\n#", SubAddr);
    if(dout) fprintf(dout, "    SubAddr = %1i\n#", SubAddr);

    if(!ReadDAC(SubAddr, &Dat))
        { printf("ReadDAC failed in recieve\n");
          if(dout) fprintf(dout, "ReadDAC failed in recieve\n");}
    else
        { printf("DAC[%3i]\n", Dat);
          if(dout) fprintf(dout, "DAC[%3i]\n", Dat);}

    ///////////////////////
    printf("#ADC:   ");
    if(dout) fprintf(dout, "#ADC:   ");
    for (ii=0; ii<4; ii++)
    //for (i=3; i>=0; i--)
    {
        tmpp = Data[ii];
        tmpp &= 0x0FFF; // set 4 major bit to 0
        //printf(" CH%d=[%d] ", ii, *Data[ii]);
        printf(" CH%d=[%d] ", ii, tmpp);
        if(dout) fprintf(dout," CH%d=[%d] ", ii, tmpp);
    }
    printf("\n");
    if(dout) fprintf(dout,"\n");

    return 1;
  }
*/
  ////////////////////////////////////////////
/*  unsigned char read_from_registers(void)
  {
    unsigned char j=0, k=0;
    unsigned int  ResData = 0, Res[12];

    for(j = 0; j<12; j++)
    {
        if(!ReadReg(0x20+SubAddr, (j+4), &ResData))
        {
            printf("Error in %d RegRes reading", j);
        }
        ResData <<= 4;
        ResData >>= 4;
        Res[j] = ResData;
    }

    printf("\n");
    printf("                CH1        CH2        CH3        CH4\n");

    for(j = 0; j<=2; j++)
        {
        if(j==0) printf("DATAlow  ");
        if(j==1) printf("DATAhigh ");
        if(j==2) printf("Hysteres ");

        for(k=0; k<=3; k++)
        {
            printf("  R%2d[%4d]", j+4+k*3, Res[j+k*3]);
        }
        printf("\n");
    }

    return 1;
  }
*/
  ////////////////////////////////////////////
/*  unsigned char read1_from_registers(void)
  {
      unsigned char j=0;
      unsigned int  ResData = 0;

      printf("\n");
      for(j = 0; j<16; j++)
      {
          if(!ReadReg(0x20+SubAddr, (j), &ResData))
          {
              printf("Error in %d RegRes reading", j);
          }
          ResData <<= 4;
          ResData >>= 4;
          printf("  R[%2d]=%4d",j, ResData);
          if(!((j+1)%4)) printf("\n");
      }
      printf("\n");
      return 1;
  }
*/


    /** -------------------------------------------------------
    *  \brief turn off vip of channels
    */
    void turn_off(void)
    {
        unsigned char ii = 0;
        //int Hvchan = HVDIM;

        if(  dout) fprintf(  dout, "\n<VIP_OFF:\n");
        if(stdout) fprintf(stdout, "\n<VIP_OFF:\n");
        ii = VIP_ADDR * 2 + VIP_SUBADDR;
        //for(ii = 0; ii < Hvchan; ii++)
        {
            //if(!hvwork[ii])  continue; if no working channel
            SetChannelAddr(int(ii/2));
            SetSubAddr(ii%2);

            printf("\n ii=%2i--%2i--%i", ii, ChanAddr, SubAddr);
            if(dout) fprintf(dout, "\n ii=%2i--%2i--%i", ii, ChanAddr, SubAddr);
            if(!WriteDAC(SubAddr, 0))  // set high = 0 kod
            {
                printf(" turn off: Error in DAC`s transmittion - set high \n");
                if(dout) fprintf(dout, "turn off: Error in DAC`s transmittion\n");
                printf("ER: ii=%2i  adr=%2i  sad=%i ===> ", ii, ChanAddr, SubAddr);
                if(dout) fprintf(dout,"ER: ii=%2i  adr=%2i  sad=%i ===> ",
                        ii, ChanAddr, SubAddr);
            }
            //sleep(1); //1 sec
            usleep(100000); //0.1 sec

            if(!TX8(0x20+SubAddr,0x02,0x0C)) // turn off
            {
                printf("turn off: Error in ADC`s setting ---> NO OFF");
                if(dout) fprintf(dout, "turn off: Error in ADC`s setting ---> NO OFF");
            }
            //sleep(1); //  1 sec after each turn on
            usleep(100000); //0.1 sec
            On = 0;
        }
        if(dout)   fprintf(  dout, "\nVIP_OFF>\n");
        if(stdout) fprintf(stdout, "\nVIP_OFF>\n");

        sleep(1);
        check_turn_off();
    }  

    
    /** -------------------------------------------------------
    *  \brief check if vip are off
    */
    void check_turn_off(void)
    {
        unsigned char ii = 0, iii = 0;
        int Hvchan = HVDIM;
        unsigned int   MData[4] = {0};
        unsigned int   kod[10] = {0};
        char debug[250] = {""};
        //unsigned int   tmppp = 0;

        if(  dout) fprintf(  dout, "\n<VIP_OFF check:\n");
        if(stdout) fprintf(stdout, "\n<VIP_OFF check:\n");
        for(ii = 0; ii < Hvchan; ii++)
        {
            //if(!hvwork[ii])  continue; if no working channel
            SetChannelAddr(int(ii/2));
            SetSubAddr(ii%2);

            printf("\n ii=%2i--%2i--%i", ii, ChanAddr, SubAddr);
            if(dout) fprintf(dout, "\n ii=%2i--%2i--%i", ii, ChanAddr, SubAddr);

            if(!ReadADCs(SubAddr,(unsigned int *)&MData))  // read ADC
            {
                sprintf(debug, " !measure_high: Error in ADC`s reading");
                print_debug(debug);
                continue;
            }

            for(iii=0; iii<4; iii++)
            {
                kod[iii] = MData[iii] & 0x0FFF; // set 4 major bit to 0
            }
            /*
            if(kod[2] == 4095)
            {
                if(dout)   fprintf(  dout, " OK");
                if(stdout) fprintf(stdout, " OK");
            }
            else
            */
            {
                sprintf(debug, " %d", kod[1]);
                print_debug(debug);
            }
            usleep(100000); //0.1 sec
        }

        sprintf(debug, "\nVIP_OFF_CHECK>\n");
        print_debug(debug);
    }

}; // class hvps_test
