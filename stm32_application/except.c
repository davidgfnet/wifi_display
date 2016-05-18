
#include <stdint.h>
#include "stm32f10x/stm32f10x.h"
#include "except.h"

void NmISR() {
	while(1);
}


void FaultISR() {
	while(1);
}

void MPUFaultHander() {
	while(1);
}

void BusFaultHandler() {
	while (1);
}

void UsageFaultHandler() {
	while (1);
}

void IntDefaultHandler() {
	while (1);
}


