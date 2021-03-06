;******************************************************************************
;   MSP-FET430P410 Demo - WDT, Toggle P5.1, Interval Overflow ISR, 32kHz ACLK
;
;   Description: Toggle P5.1 using WDT configured for Interval Timer mode.
;   Interrupt toggles LED every 250ms based on 32kHz ACLK WDT clock source.
;   ACLK = LFXT1 = 32768Hz, MCLK = SMCLK = default DCO = 32 x ACLK = 1048576Hz
;   ;* An external watch crystal between XIN & XOUT is required for ACLK *//	
;
;                 MSP430F413
;             -----------------
;         /|\|              XIN|-
;          | |                 | 32kHz
;          --|RST          XOUT|-
;            |                 |
;            |             P5.1|-->LED
;
;   M. Buccini / S. Karthikeyan
;   Texas Instruments Inc.
;   May 2005
;   Built with Code Composer Essentials Version 1.0
;******************************************************************************
 .cdecls C,LIST,  "msp430x41x.h"
;------------------------------------------------------------------------------
            .text                  ; Program Start
;------------------------------------------------------------------------------
RESET       mov.w   #300h,SP                ; Initialize stackpointer
SetupFLL    bis.b   #XCAP14PF,&FLL_CTL0     ; Configure load caps
SetupWDT    mov.w   #WDT_ADLY_250,&WDTCTL   ; WDT 250ms, ACLK, interval timer
            bis.b   #WDTIE,&IE1             ; Enable WDT interrupt
SetupP5     bis.b   #002h,&P5DIR            ; P5.1 output
                                            ;					
Mainloop    bis.w   #LPM3+GIE,SR            ; Enter LPM3, enable interrupts
            nop                             ; Required only for debugger
                                            ;
;------------------------------------------------------------------------------
WDT_ISR;    Toggle P5.1
;------------------------------------------------------------------------------
            xor.b   #002h,&P5OUT            ; Toggle P5.1
            reti                            ;		
                                            ;
;-----------------------------------------------------------------------------
;           Interrupt Vectors Used MSP430x44x
;-----------------------------------------------------------------------------
            .sect     ".reset"                  ; MSP430 RESET Vector
            .short  RESET                   ;
            .sect   ".int10"                  ; WDT Vector
            .short  WDT_ISR                 ;
            .end
