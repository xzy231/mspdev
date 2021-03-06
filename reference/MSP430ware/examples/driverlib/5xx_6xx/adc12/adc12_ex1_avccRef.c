/* --COPYRIGHT--,BSD
 * Copyright (c) 2011, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
#include "inc/hw_memmap.h"
#include "driverlib/5xx_6xx/adc12.h"
#include "driverlib/5xx_6xx/wdt.h"
#include "driverlib/5xx_6xx/gpio.h"

//******************************************************************************
//!
//!   ADC12 - Sample A0 Input, AVcc Ref, LED ON if A0 > 0.5*AVcc
//!   MSP430F552x Demo
//!
//!   Description: A single sample is made on A0 with reference to AVcc.
//!   Software sets ADC12SC to start sample and conversion - ADC12SC
//!   automatically cleared at EOC. ADC12 internal oscillator times sample (16x)
//!   and conversion. In Mainloop MSP430 waits in LPM0 to save power until ADC12
//!   conversion complete, ADC12_ISR will force exit from LPM0 in Mainloop on
//!   reti. If A0 > 0.5*AVcc, P1.0 set, else reset.
//!
//!                 MSP430F552x
//!             -----------------
//!         /|\|                 |
//!          | |                 |
//!          --|RST       P6.0/A0|<- Vin
//!            |             P1.0|--> LED
//!            |                 |
//!
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - ADC12 peripheral
//! - GPIO Port peripheral
//! - A0
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - ADC12_VECTOR
//!
//******************************************************************************

void main (void)
{
    //Stop Watchdog Timer
    WDT_hold(__MSP430_BASEADDRESS_WDT_A__);

    //P6.0 ADC option select
    GPIO_setAsPeripheralModuleFunctionOutputPin(__MSP430_BASEADDRESS_PORT6_R__,
        GPIO_PORT_P6,
        GPIO_PIN0);
    GPIO_setAsOutputPin(
        __MSP430_BASEADDRESS_PORT1_R__,
        GPIO_PORT_P1,
        GPIO_PIN0
        );

    //Initialize the ADC12 Module
    /*
     * Base address of ADC12 Module
     * Use internal ADC12 bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC12_init(__MSP430_BASEADDRESS_ADC12_PLUS__,
        ADC12_SAMPLEHOLDSOURCE_SC,
        ADC12_CLOCKSOURCE_ADC12OSC,
        ADC12_CLOCKDIVIDER_1);
    
    ADC12_enable(__MSP430_BASEADDRESS_ADC12_PLUS__);
        
    /*
     * Base address of ADC12 Module
     * For memory buffers 0-7 sample/hold for 64 clock cycles
     * For memory buffers 8-15 sample/hold for 4 clock cycles (default)
     * Disable Multiple Sampling
     */
    ADC12_setupSamplingTimer(__MSP430_BASEADDRESS_ADC12_PLUS__,
        ADC12_CYCLEHOLD_64_CYCLES,
        ADC12_CYCLEHOLD_4_CYCLES,
        ADC12_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
     * Base address of the ADC12 Module
     * Configure memory buffer 0
     * Map input A0 to memory buffer 0
     * Vref+ = AVcc
     * Vr- = AVss
     * Memory buffer 0 is not the end of a sequence
     */
    ADC12_memoryConfigure(__MSP430_BASEADDRESS_ADC12_PLUS__,
        ADC12_MEMORY_0,
        ADC12_INPUT_A0,
        ADC12_VREFPOS_AVCC,
        ADC12_VREFNEG_AVSS,
        ADC12_NOTENDOFSEQUENCE);

    //Enable memory buffer 0 interrupt
    ADC12_enableInterrupt(__MSP430_BASEADDRESS_ADC12_PLUS__,
        ADC12IE0);

    while (1)
    {
        //Enable/Start sampling and conversion
        /*
         * Base address of ADC12 Module
         * Start the conversion into memory buffer 0
         * Use the single-channel, single-conversion mode
         */
        ADC12_startConversion(__MSP430_BASEADDRESS_ADC12_PLUS__,
            ADC12_MEMORY_0,
            ADC12_SINGLECHANNEL);

        //LPM0, ADC12_ISR will force exit
        __bis_SR_register(LPM0_bits + GIE);
        //for Debugger
        __no_operation();
    }
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR (void)
{
    switch (__even_in_range(ADC12IV,34)){
        case  0: break;   //Vector  0:  No interrupt
        case  2: break;   //Vector  2:  ADC overflow
        case  4: break;   //Vector  4:  ADC timing overflow
        case  6:          //Vector  6:  ADC12IFG0
            //Is Memory Buffer 0 = A0 > 0.5AVcc?
            if (ADC12_getResults(__MSP430_BASEADDRESS_ADC12_PLUS__,
                    ADC12_MEMORY_0)
                >= 0x7ff){
                //set P1.0
                GPIO_setOutputHighOnPin(
                    __MSP430_BASEADDRESS_PORT1_R__,
                    GPIO_PORT_P1,
                    GPIO_PIN0
                    );
            } else   {
                //Clear P1.0 LED off
                GPIO_setOutputLowOnPin(
                    __MSP430_BASEADDRESS_PORT1_R__,
                    GPIO_PORT_P1,
                    GPIO_PIN0
                    );
            }

            //Exit active CPU
            __bic_SR_register_on_exit(LPM0_bits);
        case  8: break;   //Vector  8:  ADC12IFG1
        case 10: break;   //Vector 10:  ADC12IFG2
        case 12: break;   //Vector 12:  ADC12IFG3
        case 14: break;   //Vector 14:  ADC12IFG4
        case 16: break;   //Vector 16:  ADC12IFG5
        case 18: break;   //Vector 18:  ADC12IFG6
        case 20: break;   //Vector 20:  ADC12IFG7
        case 22: break;   //Vector 22:  ADC12IFG8
        case 24: break;   //Vector 24:  ADC12IFG9
        case 26: break;   //Vector 26:  ADC12IFG10
        case 28: break;   //Vector 28:  ADC12IFG11
        case 30: break;   //Vector 30:  ADC12IFG12
        case 32: break;   //Vector 32:  ADC12IFG13
        case 34: break;   //Vector 34:  ADC12IFG14
        default: break;
    }
}

