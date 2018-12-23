/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "eth.h"
#include <stdio.h>

#include "xparameters.h"

#include "netif/xadapter.h"

#include "xaxiethernet.h"	/* defines Axi Ethernet APIs */

#include "platform.h"
#include "platform_config.h"
#ifdef __arm__
#include "xil_printf.h"
#endif

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif

/* Use MII register 1 (MII status register) to detect PHY */
#define PHY_DETECT_REG  1
#define PHY_DETECT_MASK 0x1808
#define PHY_R0_DFT_SPD_1000  0x0040

#define PHY_R0_RESET         0x8000

#define PHY_R0_CTRL_REG	0
#define PHY_R2_PHY_IDENT_REG	2
#define PHY_R3_PHY_IDENT_REG	3

#define PHY_R17_SPECIFIC_STATUS_REG	17

#define RTL_8211E_ID	0x732
#define RTL_8211E_MODEL	0x11



/* defined by each RAW mode application */
void print_app_header();
int start_application();
int transfer_data();

/* missing declaration in lwIP */
void lwip_init();

#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif

static XAxiEthernet AxiEthernetInstance;

extern struct netif server_netif;
struct netif *echo_netif;

extern unsigned char u8Verbose;

u32 AxiEthernetDetectPHY(XAxiEthernet * AxiEthernetInstancePtr)
{
	u16 PhyReg;
	int PhyAddr;

	for (PhyAddr = 31; PhyAddr >= 0; PhyAddr--) {
		XAxiEthernet_PhyRead(AxiEthernetInstancePtr, PhyAddr,
				 		PHY_DETECT_REG, &PhyReg);

		if ((PhyReg != 0xFFFF) &&
		   ((PhyReg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
			/* Found a valid PHY address */
			return PhyAddr;
		}
	}

	return 0;		/* Default to zero */
}

int fnPhyTest()
{
	int Status;

	XAxiEthernet_Config *MacCfgPtr;

	u16 PhyReg0;
	u16 PhyReg2;
	u16 PhyReg3;
	u16 PhyReg17;
	signed int PhyAddr;

	u32 Phy_ID;
	u32 Phy_Model;

	u16 linkstatus;
	u16 speed_bits;
	u16 speed;
	/*
	 *  Get the configuration of AxiEthernet hardware.
	 */
	MacCfgPtr = XAxiEthernet_LookupConfig(XPAR_AXIETHERNET_0_DEVICE_ID);


	/*
	 * Initialize AxiEthernet hardware.
	 */
	Status = XAxiEthernet_CfgInitialize(&AxiEthernetInstance, MacCfgPtr,
					MacCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		if (u8Verbose) xil_printf("\r\nError initializing ethernet instance");
		return XST_FAILURE;
	}

	//Detect phy
	PhyAddr = AxiEthernetDetectPHY(&AxiEthernetInstance);
	if (u8Verbose) xil_printf("\r\nPhy Detected Address = %d", PhyAddr);

	//Check Phy Model
	XAxiEthernet_PhyRead(&AxiEthernetInstance, PhyAddr, PHY_R2_PHY_IDENT_REG, &PhyReg2);
	XAxiEthernet_PhyRead(&AxiEthernetInstance, PhyAddr, PHY_R3_PHY_IDENT_REG, &PhyReg3);

	Phy_ID = ((PhyReg2 << 6) | ((PhyReg3 >> 10) & 0x3F));
	Phy_Model = ((PhyReg3 >> 4) & 0x3F);

	if (u8Verbose) xil_printf("\r\nPhy ID = 0x%X, Phy Model Number = 0x%X", Phy_ID, Phy_Model);

	if ((Phy_ID != RTL_8211E_ID)&& (Phy_Model != RTL_8211E_MODEL)){
		if (u8Verbose) xil_printf("\r\nError: Phy ID and/or Model Number Incorrect");
		return XST_FAILURE;
	}

	//Check PHY Link and speed
	XAxiEthernet_PhyRead(&AxiEthernetInstance, PhyAddr, PHY_R17_SPECIFIC_STATUS_REG, &PhyReg17);

	if (u8Verbose) xil_printf("\r\n Phy Reg 17 value = 0x%04X", PhyReg17);

	linkstatus = ((PhyReg17 & 0x0400) >> 10);
	speed_bits  = ((PhyReg17 & 0xC000) >> 14);

	if (u8Verbose) xil_printf("\r\n linkstatus = 0x%X, speed_bits = 0x%X", linkstatus, speed_bits);

	if (linkstatus == 0) {
		if (u8Verbose) xil_printf("\r\nError: Link is Down");
		return XST_FAILURE;
	}

	switch(speed_bits){
	case 0: speed = 10;
	break;
	case 1: speed = 100;
	break;
	case 2: speed = 1000;
	break;
	default: speed = 0;
	}

	if (u8Verbose) xil_printf ("\r\nSpeed is %d", speed);


return speed;

}

void
print_ip(char *msg, struct ip_addr *ip) 
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip), 
			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

#if XPAR_GIGE_PCS_PMA_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif

int fnInitEth(XIntc *psIntc,  const macAddress_t *mac)
{
	struct ip_addr ipaddr, netmask, gw;

	echo_netif = &server_netif;
#if XPAR_GIGE_PCS_PMA_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif

	platform_setup_interrupts(psIntc);

#if LWIP_DHCP==1
	ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#else
	/* initliaze IP addresses to be used */
	IP4_ADDR(&ipaddr,  10, 113,   8, 100);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      10, 113,   8,  1);
#endif	
	print_app_header();

	lwip_init();

  	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask,
						&gw, (unsigned char*)mac,
						PLATFORM_EMAC_BASEADDR)) {
		if (u8Verbose) xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	netif_set_default(echo_netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(echo_netif);

#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */

	dhcp_start(echo_netif);
	dhcp_timoutcntr = 24;


	while(((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(echo_netif);

	if (dhcp_timoutcntr <= 0) {
		if ((echo_netif->ip_addr.addr) == 0) {
			xil_printf("DHCP Timeout\r\n");
			xil_printf("Configuring default IP of 192.168.1.10\r\n");
			IP4_ADDR(&(echo_netif->ip_addr),  192, 168,   1, 10);
			IP4_ADDR(&(echo_netif->netmask), 255, 255, 255,  0);
			IP4_ADDR(&(echo_netif->gw),      192, 168,   1,  1);
		}
	}

	ipaddr.addr = echo_netif->ip_addr.addr;
	gw.addr = echo_netif->gw.addr;
	netmask.addr = echo_netif->netmask.addr;
#endif

	print_ip_settings(&ipaddr, &netmask, &gw);

	/* start the application (web server, rxtest, txtest, etc..) */
	start_application();

	return 0;
}

void fnPrintIpSettings()
{

	struct ip_addr ipaddr, netmask, gw;

	ipaddr.addr = echo_netif->ip_addr.addr;
	gw.addr = echo_netif->gw.addr;
	netmask.addr = echo_netif->netmask.addr;

	print_ip_settings(&ipaddr, &netmask, &gw);


}



