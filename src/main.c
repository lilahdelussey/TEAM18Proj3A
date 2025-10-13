// ****************************************************************
// * TEAM18: L. DELUSSEY and A. JACOBS
// * CPEG222 3Amain 10/14/25
// * NucleoF466RE CMSIS STM32F4xx 
// * Move the standard servo using PWM on PC6 -> CHANGE LATER
// * Send angle and pulse width data over UART2
// * measured data shows +/- 45 deg rotation using 1ms to 2ms pulse width
// ****************************************************************

#include <stm32f446xx.h>
#include "SSD_Array.h"
#include <stdio.h>