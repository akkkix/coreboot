/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2000 AG Electronics Ltd.
 * Copyright (C) 2003-2004 Linux Networx
 * Copyright (C) 2004 Tyan by yhlu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <arch/romcc_io.h>
#include "pc87417.h"

static void pc87417_disable_dev(device_t dev)
{
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
}

static void pc87417_enable_dev(device_t dev, unsigned iobase)
{
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, iobase);
	pnp_set_enable(dev, 1);
}

static void xbus_cfg(device_t dev)
{
	uint8_t i, data;
	uint16_t xbus_index;

	pnp_set_logical_device(dev);
	/* select proper BIOS size (4MB) */
	pnp_write_config(dev, PC87417_XMEMCNF2, (pnp_read_config(dev, PC87417_XMEMCNF2)) | 0x04);
	xbus_index = pnp_read_iobase(dev, 0x60);

	/* enable writes to devices attached to XCS0 (XBUS Chip Select 0) */
	for (i=0; i<= 0xf; i++) {
		outb((i<<4), xbus_index + PC87417_HAP0);
	}
	return;
}
