/*
  ISD1700.h - Library for Nuvoton ISD1700 chipcorders
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

#ifndef ISD1700_h
#define ISD1700_h

#define SCK_PIN   13
#define MISO_PIN  12
#define MOSI_PIN  11

#include "WProgram.h"

class ISD1700
{
  private:
    uint8_t  _SSPin; // Arduino's digital pin which chipcorder is attached to

    /* status registers */
    uint16_t _SR0;      // Status Register #0                   uint16_t <15:0>
	uint8_t  _SR1;      // Status Register #1                   uint8_t  <7:0>
	uint16_t _APC;      // Analog Path Configuration Register   uint16_t <11:0>
	uint16_t _PP;       // Playback Pointer                     uint16_t <10:0>
	uint16_t _RP;       // Record Pointer                       uint16_t <10:0>
	uint8_t  _DEVICEID; // Device Identification Register       uint8_t  <7:3>

	/* private methods */
	uint8_t spi_transfer(uint8_t);
	void sendCmd(uint8_t);
	void sendCmd(uint8_t, uint16_t);
	void sendCmd(uint8_t, uint16_t, uint16_t);
  public:
    /* constructor */
    ISD1700(uint8_t);

	/* functions to enquire status */
	uint8_t RDY(void);
	uint8_t ERASE(void);
	uint8_t PLAY(void);
	uint8_t REC(void);
	uint8_t SE1(void);
	uint8_t SE2(void);
	uint8_t SE3(void);
	uint8_t SE4(void);
	uint8_t CMD_ERR(void);
	uint8_t FULL(void);
	uint8_t PU(void);
	uint8_t EOM(void);
	uint8_t INT(void);

	/* chipcorder commands */
	void     pu(void);
	void     stop(void);
	void     reset(void);
	void     clr_int(void);
	uint8_t  rd_status(void);
	uint16_t rd_play_ptr(void);
	void     pd(void);
	uint16_t rd_rec_ptr(void);
	uint8_t  devid(void);
	void     play(void);
	void     rec(void);
	void     erase(void);
	void     g_erase(void);
	uint16_t rd_apc(void);
	void     wr_apc1(uint16_t);
	void     wr_nvcfg(void);
	void     ld_nvcfg(void);
	void     fwd(void);
	void     chk_mem(void);
	void     extclk(void);
	void     wr_apc2(uint16_t);
	void     set_play(uint16_t, uint16_t);
	void     set_rec(uint16_t, uint16_t);
	void     set_erase(uint16_t, uint16_t);
};

#endif
