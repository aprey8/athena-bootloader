/* Name: net_w5100.c
 * Author: .
 * Copyright: Arduino
 * License: GPL http://www.gnu.org/licenses/gpl-2.0.html
 * Project: eboot
 * Function: Network initialization
 * Version: 0.1 tftp / flashing functional
 */

#include <avr/eeprom.h>
#include <avr/io.h>

#include "debug.h"
#include "debug_net.h"
#include "net.h"
#include "neteeprom.h"
#include "serial.h"
#include "spi.h"

uint8_t registerBuffer[REGISTER_BLOCK_SIZE] = {
	0x80, // MR Mode - reset device

	// EEPROM block starts here
	GW_ADDR, // GWR Gateway IP Address Register
	SUB_MASK, // SUBR Subnet Mask Register
	MAC_ADDR, // SHAR Source Hardware Address Register
	IP_ADDR, // SIPR Source IP Address Register
// EEPROM block ends here

#if defined(__WIZ_W5100__)
	0, 0, // Reserved locations
	0, // IR Interrupt Register
	0, // IMR Interrupt Mask Register
	0x07, 0xd0, // RTR Retry Time-value Register
	0x80, // RCR Retry Count Register
	0x55, // RMSR Rx Memory Size Register, 2K per socket
	0x55 // TMSR Tx Memory Size Register, 2K per socket
#elif defined(__WIZ_W5200__)
	0, 0, // Reserved locations
	0, // IR Interrupt Register
	0, // IMR Interrupt Mask Register
	0x07, 0xd0, // RTR Retry Time-value Register
	0x08, // RCR Retry Count Register
	0, // Reserved in w5200
	0 // Reserved in w5200
#elif defined(__WIZ_W5500__)
	0, 0, // Interrupt Low Level Timer (INTLEVEL0), (INTLEVEL1) (0x0013, 0x0014)
	0, // IR Interrupt Register (0x0015)
	0, // IMR Interrupt Mask Register (0x0016)
	0, // Socket Interrupt (SIR) (0x0017)
	0, // Socket Interrupt Mask (SIMR) (0x0018)
	0x07, 0xd0, // RTR Retry Time-value Register ((RTR0),(RTR0)) (0x0019,0x001A)
	0x08, // RCR Retry Count Register (0x001B)
#endif
};

void netInit(void)
{
	uint8_t i;

	/* Pull in altered network settings, if available,
	 * from AVR EEPROM (if signature bytes are set) */
	if((eeprom_read_byte((uint8_t*)NETEEPROM_SIG_1) == NETEEPROM_SIG_1_VALUE) &&
	   (eeprom_read_byte((uint8_t*)NETEEPROM_SIG_2) == NETEEPROM_SIG_2_VALUE))
	{
		for(i = 0; i < EEPROM_SETTINGS_SIZE; i++)
			registerBuffer[i + 1] = eeprom_read_byte((uint8_t*)NETEEPROM_DATA + i);

		DBG_NET(tracePGMlnNet(mDebugNet_EEPROM);)
	}
	DBG_NET(else tracePGMlnNet(mDebugNet_BUILTIN);)

	DBG_NET(
		tracePGMlnNet(mDebugNet_ADDR); for(i = 15; i < 19; i++) {
			tracenet(registerBuffer[i]);
			if(i != 18)
				putch(0x2E);
		} tracePGMlnNet(mDebugNet_SUBN);
		for(i = 5; i < 9; i++) {
			tracenet(registerBuffer[i]);
			if(i != 8)
				putch(0x2E);
		} tracePGMlnNet(mDebugNet_GW);
		for(i = 1; i < 5; i++) {
			tracenet(registerBuffer[i]);
			if(i != 4)
				putch(0x2E);
		} tracePGMlnNet(mDebugNet_MAC);
		for(i = 9; i < 15; i++) {
			tracenet(registerBuffer[i]);
			if(i != 14)
				putch(0x2E);
		})

	/** Configure Wiznet chip. Network settings */
#if defined(__WIZ_W5200__)
	for(i = 0; i < REGISTER_BLOCK_SIZE - 2; i++)
#else
	for(i = 0; i < REGISTER_BLOCK_SIZE; i++)
#endif
#if defined(__WIZ_W5500__)
		spiWriteReg(i, 0x04, registerBuffer[i]);
#else
		spiWriteReg(i, 0, registerBuffer[i]);
#endif

#if defined(__WIZ_W5200__)
	for(i = 0; i < 8; i++)
	{
		spiWriteReg((0x4000 + i * 0x100 + 0x001F), 0, 0x02);
		spiWriteReg((0x4000 + i * 0x100 + 0x001E), 0, 0x02);
	}
#elif defined(__WIZ_W5500__)
	for(int i = 0; i < 8; i++)
	{
		uint8_t cntl_byte = (0x0C + (i << 5));
		spiWriteReg(0x1E, cntl_byte, 2); // 0x1E - Sn_RXBUF_SIZE
		spiWriteReg(0x1F, cntl_byte, 2); // 0x1F - Sn_TXBUF_SIZE
	}
#endif

	DBG_NET(tracePGMlnNet(mDebugNet_DONE);)
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; tab-width 4;
