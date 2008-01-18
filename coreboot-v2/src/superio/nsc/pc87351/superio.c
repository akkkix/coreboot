/* Copyright 2000  AG Electronics Ltd. */
/* Copyright 2003-2004 Linux Networx */
/* This code is distributed without warranty under the GPL v2 (see COPYING) */

/* 
 * Richard A Smith 
 * I derived this code from the pc87360 device and removed the stuff the 87351
 * dosen't do.
*/

#include <arch/io.h>
#include <device/device.h>
#include <device/pnp.h>
#include <console/console.h>
#include <string.h>
#include <bitops.h>
#include <uart8250.h>
#include <pc80/keyboard.h>
#include "chip.h"
#include "pc87351.h"

static void init(device_t dev)
{
	struct superio_nsc_pc87351_config *conf;
	struct resource *res0, *res1;
	/* Wishlist handle well known programming interfaces more
	 * generically.
	 */
	if (!dev->enabled) {
		return;
	}
	conf = dev->chip_info;
	switch(dev->path.u.pnp.device) {
	case PC87351_SP1: 
		res0 = find_resource(dev, PNP_IDX_IO0);
		init_uart8250(res0->base, &conf->com1);
		break;
	case PC87351_SP2:
		res0 = find_resource(dev, PNP_IDX_IO0);
		init_uart8250(res0->base, &conf->com2);
		break;
	case PC87351_KBCK:
		res0 = find_resource(dev, PNP_IDX_IO0);
		res1 = find_resource(dev, PNP_IDX_IO1);
		init_pc_keyboard(res0->base, res1->base, &conf->keyboard);
		break;
	}
}

static struct device_operations ops = {
	.read_resources   = pnp_read_resources,
	.set_resources    = pnp_set_resources,
	.enable_resources = pnp_enable_resources,
	.enable           = pnp_enable,
	.init             = init,
};

static struct pnp_info pnp_dev_info[] = {
 { &ops, PC87351_FDC,  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, { 0x07fa, 0}, },
 { &ops, PC87351_PP,   PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, { 0x04f8, 0}, },
 { &ops, PC87351_SP2,  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0 | PNP_DRQ1, { 0x7f8, 0 }, },
 { &ops, PC87351_SP1,  PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
 { &ops, PC87351_SWC,  PNP_IO0 | PNP_IRQ0, { 0xfff0, 0 }, },
 { &ops, PC87351_KBCM, PNP_IRQ0 },
 { &ops, PC87351_KBCK, PNP_IO0 | PNP_IO1 | PNP_IRQ0, { 0x7f8, 0 }, { 0x7f8, 0x4}, },
 { &ops, PC87351_GPIO, PNP_IO0 | PNP_IRQ0, { 0xfff8, 0 } },
 { &ops, PC87351_FSD,  PNP_IO0 | PNP_IRQ0, { 0xfff8, 0 } },
};


static void enable_dev(struct device *dev)
{
	pnp_enable_devices(dev, &pnp_ops,
		sizeof(pnp_dev_info)/sizeof(pnp_dev_info[0]), pnp_dev_info);
}

struct chip_operations superio_nsc_pc87351_ops = {
	CHIP_NAME("NSC PC87351 Super I/O")
	.enable_dev = enable_dev,
};