#include "types.h"
#include "ReadWrite.h"

uint16 read16(const uint8 *data)
{
	return (uint16)(data[0] + ((uint16)(data[1]) << 8));
}

uint32 read24(const uint8 *data)
{
	return (uint32)(data[0] + ((uint32)(data[1]) << 8) + ((uint32)(data[2]) << 16));
}

void write16(uint8* data, const uint16 val)
{
	data[0] = (uint8)(val & 0xff);
	data[1] = (uint8)(val >> 8);
}

void write24(uint8* data, const uint32 val)
{
	data[0] = (uint8)(val & 0xff);
	data[1] = (uint8)((val >> 8) & 0xff);
	data[2] = (uint8)(val >> 16);
}
