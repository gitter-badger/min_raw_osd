#include "boot.h"
#include <avr/pgmspace.h>
#include "config.h"
#include "lib/timer/timer.h"
#include "lib/uart/uart.h"
#include "lib/max7456/max7456.h"
#include "osd/osd.h"
#include "telemetry/telemetry.h"

#define _BOOT_LOGO_WIDTH 5
#define _BOOT_LOGO_HEIGHT 2

#define _BOOT_CMD_BUF_SIZE 8

namespace boot
{

void _get_cmd (char *buf, uint8_t len, uint16_t timeout)
{
	uint8_t i = 0;
	uint32_t stop = timer::ticks () + timeout;
	while (true)
	{
		uint16_t data = timer::ticks () < stop ? CONSOLE_UART::receive () : 0;
		if (data & 0xff00) continue;
		if (data == 0x0d || i == len - 1) data = 0;
		buf [i ++] = data;
		if (!data) break;
	}
}

bool show ()
{
	max7456::clear ();

	max7456::open (1, 1);
	fprintf_P (&max7456::stream, PSTR ("MIN RAW OSD v.%04u"), VERSION);
	for (uint8_t x = 1; x < max7456::right; x ++)
		max7456::put (x, 2, 0x1a);

	max7456::puts_p (2, 4, PSTR ("\xfc modules:"));
	max7456::open (13, 4);
	for (uint8_t i = 0; i < telemetry::modules::count; i ++)
		fprintf_P (&max7456::stream, PSTR ("%S\n"), telemetry::modules::name_p (i));

	osd::draw::rect (2, max7456::bottom - 3, 26, 3);
	max7456::open (3, max7456::bottom - 2);
	fprintf_P (&max7456::stream, PSTR ("Send \""BOOT_CONFIG_CODE"\" or wait %us"), BOOT_CONFIG_WAIT_TIME / 1000);

	CONSOLE_UART::send_string_p (PSTR ("READY\r\n"));

	char data [_BOOT_CMD_BUF_SIZE];
	_get_cmd (data, _BOOT_CMD_BUF_SIZE, BOOT_CONFIG_WAIT_TIME);
	return !strncmp_P (data, PSTR (BOOT_CONFIG_CODE), _BOOT_CMD_BUF_SIZE);
}

}
