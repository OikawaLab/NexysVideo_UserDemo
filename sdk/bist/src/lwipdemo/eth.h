/*
 * eth.h
 *
 *  Created on: Jan 8, 2015
 *      Author: Elod
 */

#ifndef ETH_H_
#define ETH_H_

#include "xintc.h"
#include "../iic/iic.h"

int fnInitEth(XIntc *psIntc, const macAddress_t *mac);
int transfer_data();

void fnPrintIpSettings();
int fnPhyTest();


#endif /* ETH_H_ */
