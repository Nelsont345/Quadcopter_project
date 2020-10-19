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

int main (int argc, uint8_t *argv[])
{
	uint8_t value = get_crc(0, argv[0], 1);
	printf("%u \n", value);
	uint8_t value2 = get_crc(0, value, 1);
	printf("%u \n", value2);
}