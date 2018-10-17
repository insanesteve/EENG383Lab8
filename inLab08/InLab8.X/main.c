//--------------------------------------------------------------------
// Name:            Chris Coulston
// Date:            Fall 2018
// Purp:            inLab08
//
// Assisted:        The entire class of EENG 383
// Assisted by:     Microchips 18F26K22 Tech Docs 
//-
//- Academic Integrity Statement: I certify that, while others may have
//- assisted me in brain storming, debugging and validating this program,
//- the program itself is my own work. I understand that submitting code
//- which is the work of other individuals is a violation of the course
//- Academic Integrity Policy and may result in a zero credit for the
//- assignment, or course failure and a report to the Academic Dishonesty
//- Board. I also understand that if I knowingly give my original work to
//- another individual that it could also result in a zero credit for the
//- assignment, or course failure and a report to the Academic Dishonesty
//- Board.
//------------------------------------------------------------------------
#include "mcc_generated_files/mcc.h"


#define     NUM_SAMPLES     64
#define     ZERO_LEVEL      128

uint8_t     NEW_SAMPLE = false;
uint8_t     upperThreshold = 138, lowerThreshold = 118;
uint8_t     thresholdDelta = 5;

void INIT_PIC (void);

//----------------------------------------------
// Main "function"
//----------------------------------------------
void main (void) {

	uint8_t	i, num_crosses = 0, adc_reading[NUM_SAMPLES], crossIndicies[NUM_SAMPLES>>1];
    char cmd;
    
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    ADC_SelectChannel(4);

  	printf("inLab 08 terminal\r\n");
    printf("Microphone experiments\r\n");
    printf("Dev'18 board wiring\r\n");
	printf("\r\n> ");                       // print a nice command prompt

	for(;;) {

		if (EUSART1_DataReady) {			// wait for incoming data on USART
            cmd = EUSART1_Read();
			switch (cmd) {		// and do what it tells you to do

                case '?':
                    printf("------------------------------\r\n");
                    printf("?: Help menu\r\n");                   
                    printf("o: k\r\n");
                    printf("Z: Reset processor\r\n");          
                    printf("z: Clear the terminal\r\n");  
                    printf("T/t: Increase/decrease threshold %d - %d\r\n", upperThreshold, lowerThreshold);
                    printf("f: gather %d samples from ADC\r\n",NUM_SAMPLES); 
                    printf("------------------------------\r\n");                    
                    break;

                //--------------------------------------------
                // Reply with "k", used for PC to PIC test
                //--------------------------------------------    
                case 'o':
                    printf(" k\r\n");
                    break;
                    
                //--------------------------------------------
                // Reset the processor after clearing the terminal
                //--------------------------------------------                      
                case 'Z':
                    printf("%c\r\n", cmd);
                    for (i=0; i<40; i++) printf("\n");
                    RESET();                    
                    break;
                    
                //--------------------------------------------
                // Clear the terminal
                //--------------------------------------------                      
                case 'z':
                    printf("%c\r\n", cmd);
                    for (i=0; i<40; i++) printf("\n");                            
                    break;    
                    
                //--------------------------------------------
                // Clear the terminal
                //--------------------------------------------                      
                case 'T':
                case 't':
                    printf("%c\r\n", cmd);
                    upperThreshold += cmd=='T'?thresholdDelta:-thresholdDelta;
                    lowerThreshold += cmd=='T'?-thresholdDelta:thresholdDelta;
                    printf("Threshold is now: %d - %d\r\n", upperThreshold, lowerThreshold);
                    break;     
                    
                //--------------------------------------------
                // Find the frequency 
                //--------------------------------------------                                          
                case'f':
                    printf("%c\r\n", cmd);
                    while(ADRESH < upperThreshold && ADRESH > lowerThreshold);
                    NEW_SAMPLE = false;
                    for(i=0; i<NUM_SAMPLES; i++) {
                        while (NEW_SAMPLE == false);
                        NEW_SAMPLE = false;
                        adc_reading[i] = ADRESH;
                    } // end while

                    printf("The last %d ADC samples from the microphone are:",NUM_SAMPLES);
                    for (i=0; i<NUM_SAMPLES; i++){ 		// print-out samples
                        if (i%16 == 0){
                            printf("\r\nS[%2u]", i);
                        }
                        printf("%3u ",adc_reading[i]);
                        if (i != NUM_SAMPLES-1){
                            if (adc_reading[i] <= 128 && adc_reading[i+1] > 128){
                                crossIndicies[num_crosses] = i;
                                num_crosses++;
                            }
                        }
                    }
                    printf("\r\n");
                    printf("The sound wave crossed at the following indicies\r\n");
                    for (i = 0; i < num_crosses; i++){
                        printf("%u ", crossIndicies[i]);
                    }
                    printf("\r\nThe sound wave had %u periods\r\n", num_crosses-1);
                    uint16_t avg = 0;
                    for (i = 0; i < num_crosses-1; i++){
                        printf("%u - %u = %u \r\n", crossIndicies[i+1], crossIndicies[i], crossIndicies[i+1] - crossIndicies[i]);
                        avg += crossIndicies[i+1] - crossIndicies[i];
                    }
                    avg = avg * 200 / (num_crosses-1);
                    printf("average period = %u us\r\n", avg);
                    printf("average frequency = %u Hz\r\n", 1/avg);
                    break;
                    
                //--------------------------------------------
                // If something unknown is hit, tell user
                //--------------------------------------------
                default:
                    printf("Unknown key %c\r\n",cmd);
                    break;
			} // end switch
		}	// end if
	} // end while 

} // end main




//-----------------------------------------------------------------------------
// Start an analog to digital conversion every 100uS.  Toggle RC1 so that users
// can check how fast conversions are being performed.
//-----------------------------------------------------------------------------
void TMR0_DefaultInterruptHandler(void){

	SAMPLE_PIN_SetHigh();			// Set high when every we start a new conversion
	ADCON0bits.GO_NOT_DONE = 1;		// start a new conversion
	NEW_SAMPLE = true;				// tell main that we have a new value
	TMR0_Reload();
    SAMPLE_PIN_SetLow();            // Monitor pulse width to determine how long we are in ISR
    
} // end TMR0_DefaultInterruptHandler

