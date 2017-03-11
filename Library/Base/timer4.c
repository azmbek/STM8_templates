/**
  \file timer4.c
   
  \author G. Icking-Konert
  \date 2013-11-22
  \version 0.1
   
  \brief implementation of timer TIM4 (1ms clock) functions/macros
   
  implementation of timer TIM4 functions as 1ms master clock.
  Optional functionality via #define:
    - USE_MILLI_ISR: allow attaching user function to 1ms interrupt
*/

/*----------------------------------------------------------
    INCLUDE FILES
----------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include "stm8as.h"
#include "config.h"
#define _TIM4_MAIN_          // required for globals in timer4.h
  #include "timer4.h"
#undef _TIM4_MAIN_

/*-----------------------------------------------------------------------------
    DECLARATION OF MODULE VARIABLES
-----------------------------------------------------------------------------*/
 
#if defined(USE_MILLI_ISR)
  volatile void (*m_TIM4UPD_pFct)(void) = TIM4_Default;    ///< function pointer to call in TIM4UPD ISR
#endif



/*----------------------------------------------------------
    FUNCTIONS
----------------------------------------------------------*/

/**
  \fn void TIM4_init(void)
   
  \brief init timer 4 for 1ms master clock with interrupt
   
  init 8-bit timer TIM4 with 1ms tick. Is used for SW master clock
  via below interrupt.
*/
void TIM4_init(void) {

  // stop the timer
  TIM4.CR1.reg.CEN = 0;
  
  // initialize global clock variables
  g_flagMilli = 0;
  g_millis    = 0;
  g_micros    = 0;
  
  // reset function pointer for TIM4 ISR
  #if defined(USE_MILLI_ISR)
    m_TIM4UPD_pFct = TIM4_Default;
  #endif

  // clear counter
  TIM4.CNTR.byte = 0x00;

  // auto-reload value buffered
  TIM4.CR1.reg.ARPE = 1;

  // clear pending events
  TIM4.EGR.byte  = 0x00;

  // set clock to 16Mhz/2^6 = 250kHz -> 4us period
  TIM4.PSCR.reg.PSC = 6;

  // set autoreload value for 1ms (=250*4us)
  TIM4.ARR.byte  = 250;

  // enable timer 4 interrupt
  TIM4.IER.reg.UIE = 1;
  
  // start the timer
  TIM4.CR1.reg.CEN = 1;
  
} // TIM4_init


  
#if defined(USE_MILLI_ISR)

  /**
    \fn void TIM4UPD_Default(void)
   
    \brief default dummy function for TIM4 ISRs
  
    default dummy function for below ISRs. Is faster
    than checking if a function pointer was given or not
  */
  void TIM4_Default(void)
  {
    return;
  
  } // TIM4UPD_Default



  /**
    \fn TIM4UPDAttachInterrupt(void (*pFct)(void))
   
    \brief attach/detach function call to timer 4 ISR
  
    \param[in]  pFct  function to call in ISR
   
    attach/detach function which is called in timer 4 interrupt
    service routine. Parameter NULL detaches the function again.
  */
  void TIM4UPDAttachInterrupt(void (*pFct)(void)) {
  
    uint8_t   tmp;
  
    // disable TIM4 interrupt while modifying
    tmp = TIM4.IER.reg.UIE;
    TIM4.IER.reg.UIE = 0;
  
    // attach function to ISR
    m_TIM4UPD_pFct = pFct;

    // restore original interrupt setting
    TIM4.IER.reg.UIE = tmp;
  
  } // TIM4UPDAttachInterrupt

#endif // USE_MILLI_ISR



/**
  \fn void TIM4UPD_ISR(void)
   
  \brief ISR for timer 4 (1ms master clock)
   
  interrupt service routine for timer TIM4.
  Used for 1ms master clock and optional user function
*/
#if defined(__CSMC__)
  @near @interrupt void TIM4UPD_ISR(void)
#elif defined(__SDCC)
  void TIM4UPD_ISR() __interrupt(__TIM4UPD_VECTOR__)
#endif
{
  // clear timer 4 interrupt flag
  TIM4.SR1.reg.UIF = 0;

  // set/increase global variables
  g_micros += 1000L;
  g_millis++;
  g_flagMilli = 1;
  
  // if set via TIM4UPD_attach_interrupt(), call user function
  #if defined(USE_MILLI_ISR)
    (*m_TIM4UPD_pFct)();
  #endif

  return;

} // TIM4UPD_ISR


/*-----------------------------------------------------------------------------
    END OF MODULE
-----------------------------------------------------------------------------*/
