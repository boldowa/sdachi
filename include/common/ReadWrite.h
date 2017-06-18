#pragma once
/**********************************************************
 *
 * ReadWrite is responsible for ...
 *
 **********************************************************/

uint16 read16(const uint8*);
uint32 read24(const uint8*);

void write16(uint8*, const uint16);
void write24(uint8*, const uint32);

