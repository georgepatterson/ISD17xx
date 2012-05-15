/*
  ISD1700.cpp - Library for Nuvoton ISD1700 chipcorders
  Copyright (c) 2009 Marcelo Shiniti Uchimura.
  Author: Marcelo Shiniti Uchimura, CITS, <www.cits.br>
  Version: February 8, 2009

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
 * Includes
 */
#include "WProgram.h"
#include "ISD1700.h"

/*
 * Definitions
 */

/*
 * Constructor
 * Based on Cam Thompson's Spi library
 */
ISD1700::ISD1700(uint8_t SSPin)
{
  uint8_t dummy;

  _SSPin = SSPin;

  // initialize SPI communication
  pinMode(SCK_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  pinMode(_SSPin, OUTPUT);

  // initialize SPI communication
  SPCR = (1<<CPHA) | (1<<CPOL) | (1<<DORD) | (1<<SPE) | (1<<MSTR);
  dummy = SPSR;
  dummy = SPDR;
  digitalWrite(_SSPin, HIGH);

  // initialize private vars
  _SR0 = 0;      // Status Register #0                   uint16_t <15:0>
  _SR1 = 0;      // Status Register #1                   uint8_t  <7:0>
  _APC = 0;      // Analog Path Configuration Register   uint16_t <11:0>
  _PP = 0;       // Playback Pointer                     uint16_t <10:0>
  _RP = 0;       // Record Pointer                       uint16_t <10:0>
  _DEVICEID = 0; // Device Identification Register       uint8_t  <7:3>

  // reset chipcorder
  reset();
}

/*
 * Transfer data to SPI peripheral slaves
 * Based on Cam Thompson's Spi library
 */
uint8_t ISD1700::spi_transfer(uint8_t data)
{
  SPDR = data;
  while (!(SPSR & (1<<SPIF)));
  return SPDR;
}

/*
 * Send a command to chipcorder
 */
void ISD1700::sendCmd(uint8_t cmd)
{
  uint8_t data[4] = {0xFF,0xFF,0xFF,0xFF};

  digitalWrite(_SSPin, LOW);
  data[0] = spi_transfer(cmd);
  data[1] = spi_transfer(0);
  switch(cmd)
  {
    case 0x05:                     // RD_STATUS
	case 0x09:                     // DEVID
	  data[2] = spi_transfer(0);
	  break;
	case 0x06:                     // RD_PLAY_PTR
	case 0x08:                     // RD_REC_PTR
	case 0x44:                     // RD_APC
	  data[2] = spi_transfer(0);
	  data[3] = spi_transfer(0);
	  break;
  }
  digitalWrite(_SSPin, HIGH);
  _SR0 = word(data[1],data[0]);
  switch(cmd)
  {
    case 0x05:    // RD_STATUS
	  _SR1 = data[2];
	  break;
	case 0x06:    // RD_PLAY_PTR
	  _PP = word(data[3],data[2]);
	  break;
	case 0x08:    // RD_REC_PTR
	  _RP = word(data[3],data[2]);
	  break;
	case 0x09:    // DEVID
	  _DEVICEID = data[2];
	  break;
	case 0x44:    // RD_APC
	  _APC = word(data[3],data[2]);
	  break;
  }
}

/*
 * Send a command to chipcorder
 * plus one 12-bit parameter
 *
 * Usage is valid for the following commands:
 *   WR_APC1
 *   WR_APC2
 */
void ISD1700::sendCmd(uint8_t cmd, uint16_t apc)
{
  uint8_t data[3] = {0xFF,0xFF,0xFF};

  // check if it is either WR_APC1 or WR_APC2 instruction
  // exit otherwise
  if(cmd != 0x45 && cmd != 0x65)
	return;

  digitalWrite(_SSPin, LOW);
  data[0] = spi_transfer(cmd);
  data[1] = spi_transfer( (uint8_t)(apc & 0xFF) );
  data[2] = spi_transfer( (uint8_t)(apc>>8) );
  digitalWrite(_SSPin, HIGH);
  _SR0 = word(data[1],data[2]);
}

/*
 * Send a command to chipcorder
 * plus two 11-bit parameters
 *
 * Usage is valid for the following commands:
 *   SET_PLAY
 *   SET_REC
 *   SET_ERASE
 */
void ISD1700::sendCmd(uint8_t cmd, uint16_t startAddr, uint16_t endAddr)
{
  uint8_t data[2] = {0xFF,0xFF};

  // check if it is either SET_PLAY, SET_REC, or SET_ERASE instruction
  // if not, exit method
  if(cmd != 0x80 && cmd != 0x81 && cmd != 0x82)
	return;

  digitalWrite(_SSPin, LOW);
  data[0] = spi_transfer(cmd);
  data[1] = spi_transfer(0);
  data[0] = spi_transfer( (uint8_t)(startAddr & 0xFF) );
  data[1] = spi_transfer( (uint8_t)(startAddr>>8) );
  data[0] = spi_transfer( (uint8_t)(endAddr & 0xFF) );
  data[1] = spi_transfer( (uint8_t)(endAddr>>8) );
  data[0] = spi_transfer(0);
  digitalWrite(_SSPin, HIGH);
  _SR0 = word(data[1],data[0]);
}

/*
 * Status flags
 */
uint8_t ISD1700::RDY(void)
{
  rd_status();
  return (_SR1 & 1);
}

uint8_t ISD1700::ERASE(void)
{
  rd_status();
  return (_SR1 & 2);
}

uint8_t ISD1700::PLAY(void)
{
  rd_status();
  return (_SR1 & 4);
}

uint8_t ISD1700::REC(void)
{
  rd_status();
  return (_SR1 & 8);
}

uint8_t ISD1700::SE1(void)
{
  rd_status();
  return (_SR1 & 16);
}

uint8_t ISD1700::SE2(void)
{
  rd_status();
  return (_SR1 & 32);
}

uint8_t ISD1700::SE3(void)
{
  rd_status();
  return (_SR1 & 64);
}

uint8_t ISD1700::SE4(void)
{
  rd_status();
  return (_SR1 & 128);
}

uint8_t ISD1700::CMD_ERR(void)
{
  return (uint8_t)(_SR0 & 1);
}

uint8_t ISD1700::FULL(void)
{
  return (uint8_t)(_SR0 & 2);
}

uint8_t ISD1700::PU(void)
{
  return (uint8_t)(_SR0 & 4);
}

uint8_t ISD1700::EOM(void)
{
  return (uint8_t)(_SR0 & 8);
}

uint8_t ISD1700::INT(void)
{
  return (uint8_t)(_SR0 & 16);
}


/*
 * PU Power up
 */
void ISD1700::pu(void)
{
  sendCmd(0x01);
}

/*
 * STOP Stop
 */
void ISD1700::stop(void)
{
  sendCmd(0x02);
}

/*
 * RESET Reset
 */
void ISD1700::reset(void)
{
  sendCmd(0x03);
}

/*
 * CLR_INT Clear interrupt
 */
void ISD1700::clr_int(void)
{
  sendCmd(0x04);
}

/*
 * RD_STATUS Read status
 */
uint8_t ISD1700::rd_status(void)
{
  sendCmd(0x05);
  return _SR1;
}

/*
 * RD_PLAY_PTR Read playback pointer
 */
uint16_t ISD1700::rd_play_ptr(void)
{
  sendCmd(0x06);
  return _PP;
}

/*
 * PD Power down
 */
void ISD1700::pd(void)
{
  sendCmd(0x07);
}

/*
 * RD_REC_PTR Read record pointer
 */
uint16_t ISD1700::rd_rec_ptr(void)
{
  sendCmd(0x08);
  return _RP;
}

/*
 * DEVID Read device ID register
 */
uint8_t ISD1700::devid(void)
{
  sendCmd(0x09);
  return _DEVICEID;
}

/*
 * PLAY Play from current location
 */
void ISD1700::play(void)
{
  sendCmd(0x40);
}

/*
 * REC Record from current location
 */
void ISD1700::rec(void)
{
  sendCmd(0x41);
}

/*
 * ERASE Erase current message
 */
void ISD1700::erase(void)
{
  sendCmd(0x42);
}

/*
 * G_ERASE Erase all messages except Sound Effects
 */
void ISD1700::g_erase(void)
{
  sendCmd(0x43);
}

/*
 * RD_APC Read Analog Path Configuration register
 */
uint16_t ISD1700::rd_apc(void)
{
  sendCmd(0x44);
  return _APC;
}

/*
 * WR_APC1 Write data <11:0> into the Analog Path Configuration register
 *         with volume settings from /VOL pin
 */
void ISD1700::wr_apc1(uint16_t apc)
{
  sendCmd(0x45, apc);
}

/*
 * WR_NVCFG Write the contents of APC register into NVCFG register
 */
void ISD1700::wr_nvcfg(void)
{
  sendCmd(0x46);
}

/*
 * LD_NVCFG Load contents of NVCFG into the APC register
 */
void ISD1700::ld_nvcfg(void)
{
  sendCmd(0x47);
}

/*
 * FWD Forward playback pointer to the start address of the next message
 */
void ISD1700::fwd(void)
{
  sendCmd(0x48);
}

/*
 * CHK_MEM Check circular memory
 */
void ISD1700::chk_mem(void)
{
  sendCmd(0x49);
}

/*
 * EXTCLK Enable/disable external clock mode
 */
void ISD1700::extclk(void)
{
  sendCmd(0x4A);
}

/*
 * WR_APC2 Write data <11:0> into the Analog Path Configuration register
 *         with volume settings from bits <2:0>
 */
void ISD1700::wr_apc2(uint16_t apc)
{
  sendCmd(0x65, apc);
}


/*
 * SET_PLAY Play from start address to end address, or stop at EOM
 *          depending on D11 of APC
 */
void ISD1700::set_play(uint16_t startAddr, uint16_t endAddr)
{
  sendCmd(0x80, startAddr, endAddr);
}

/*
 * SET_REC Record from start address to end address
 */
void ISD1700::set_rec(uint16_t startAddr, uint16_t endAddr)
{
  sendCmd(0x81, startAddr, endAddr);
}

/*
 * SET_ERASE Erase from start address to end address
 */
void ISD1700::set_erase(uint16_t startAddr, uint16_t endAddr)
{
  sendCmd(0x82, startAddr, endAddr);
}
