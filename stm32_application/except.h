/**
 * @file except.h
 * @author Tedesys Global S.L.
 * @date Dec 13, 2010
 */

#ifndef EXCEPT_H_
#define EXCEPT_H_

void NmISR(void) __attribute__ ((interrupt));
void FaultISR(void) __attribute__ ((interrupt));
void MPUFaultHander(void) __attribute__ ((interrupt));
void BusFaultHandler(void) __attribute__ ((interrupt));
void UsageFaultHandler(void) __attribute__ ((interrupt));
void IntDefaultHandler(void) __attribute__ ((interrupt));

#endif /* EXCEPT_H_ */
