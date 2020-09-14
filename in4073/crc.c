#include "crc.h"


uint8_t get_crc(uint8_t crc, void const *msg, uint8_t bufferSize)
{
	uint8_t const *buffer = msg;

	if (buffer == NULL) return 0xff;

	crc &= 0xff;

	for (int i = 0; i < bufferSize; i++)
	{
		crc = crc8_table[ crc ^ buffer[i] ];
	}

	return crc;
}