#define K8_ALLOCATE_IO_RANGE 1

#define QRANK_DIMM_SUPPORT 1

#if CONFIG_LOGICAL_CPUS==1
#define SET_NB_CFG_54 1
#endif

#include <stdint.h>
#include <string.h>
#include <device/pci_def.h>
#include <arch/io.h>
#include <device/pnp_def.h>
#include <arch/romcc_io.h>
#include <cpu/x86/lapic.h>
#include <pc80/mc146818rtc.h>
#include <console/console.h>
#include <lib.h>
#include <cpu/amd/model_fxx_rev.h>
#include "northbridge/amd/amdk8/incoherent_ht.c"
#include "southbridge/nvidia/ck804/ck804_early_smbus.c"
#include "northbridge/amd/amdk8/raminit.h"
#include "cpu/amd/model_fxx/apic_timer.c"
#include "lib/delay.c"
#include "cpu/x86/lapic/boot_cpu.c"
#include "northbridge/amd/amdk8/reset_test.c"
#include "superio/smsc/lpc47b397/lpc47b397_early_serial.c"
#include "superio/smsc/lpc47b397/lpc47b397_early_gpio.c"
#define SUPERIO_GPIO_DEV PNP_DEV(0x2e, LPC47B397_RT)
#define SUPERIO_GPIO_IO_BASE 0x400
#include "cpu/x86/bist.h"
#include "northbridge/amd/amdk8/debug.c"
#include <cpu/amd/mtrr.h>
#include "cpu/x86/mtrr/earlymtrr.c"
#include "northbridge/amd/amdk8/setup_resource_map.c"
#define SERIAL_DEV PNP_DEV(0x2e, LPC47B397_SP1)

static void memreset_setup(void)
{
}

static void memreset(int controllers, const struct mem_controller *ctrl)
{
}

static void sio_gpio_setup(void)
{
	unsigned value;

	/*Enable onboard scsi*/
	lpc47b397_gpio_offset_out(SUPERIO_GPIO_IO_BASE, 0x2c, (1<<7)|(0<<2)|(0<<1)|(0<<0)); // GP21, offset 0x2c, DISABLE_SCSI_L
	value = lpc47b397_gpio_offset_in(SUPERIO_GPIO_IO_BASE, 0x4c);
	lpc47b397_gpio_offset_out(SUPERIO_GPIO_IO_BASE, 0x4c, (value|(1<<1)));
}

static inline void activate_spd_rom(const struct mem_controller *ctrl)
{
	/* nothing to do */
}

static inline int spd_read_byte(unsigned device, unsigned address)
{
	return smbus_read_byte(device, address);
}

#include "northbridge/amd/amdk8/raminit.c"
#include "northbridge/amd/amdk8/coherent_ht.c"
#include "lib/generic_sdram.c"

 /* tyan does not want the default */
#include "resourcemap.c"

#include "cpu/amd/dualcore/dualcore.c"

#include "southbridge/nvidia/ck804/ck804_early_setup_ss.h"

//set GPIO to input mode
#define CK804_MB_SETUP \
	RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+ 5, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* M9,GPIO6, PCIXB2_PRSNT1_L*/  \
	RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+15, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* M8,GPIO16, PCIXB2_PRSNT2_L*/ \
	RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+44, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* P5,GPIO45, PCIXA_PRSNT1_L*/  \
	RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+ 7, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* M5,GPIO8, PCIXA_PRSNT2_L*/   \
	RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+16, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* K4,GPIO17, PCIXB_PRSNT1_L*/  \
	RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+45, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* P7,GPIO46, PCIXB_PRSNT2_L*/

#include "southbridge/nvidia/ck804/ck804_early_setup_car.c"

#include "cpu/amd/car/post_cache_as_ram.c"

#include "cpu/amd/model_fxx/init_cpus.c"

#include "southbridge/nvidia/ck804/ck804_enable_rom.c"
#include "northbridge/amd/amdk8/early_ht.c"

static void sio_setup(void)
{
	unsigned value;
	u32 dword;
	u8 byte;

	pci_write_config32(PCI_DEV(0, CK804_DEVN_BASE+1, 0), 0xac, 0x047f0400);

	byte = pci_read_config8(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0x7b);
	byte |= 0x20;
	pci_write_config8(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0x7b, byte);

	dword = pci_read_config32(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0xa0);
	dword |= (1<<29)|(1<<0);
	pci_write_config32(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0xa0, dword);

	dword = pci_read_config32(PCI_DEV(0, CK804_DEVN_BASE+1, 0), 0xa4);
	dword |= (1<<16);
	pci_write_config32(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0xa4, dword);

	lpc47b397_enable_serial(SUPERIO_GPIO_DEV, SUPERIO_GPIO_IO_BASE);
	value = lpc47b397_gpio_offset_in(SUPERIO_GPIO_IO_BASE, 0x77);
	value &= 0xbf;
	lpc47b397_gpio_offset_out(SUPERIO_GPIO_IO_BASE, 0x77, value);
}

void cache_as_ram_main(unsigned long bist, unsigned long cpu_init_detectedx)
{
	static const u16 spd_addr [] = {
		(0xa<<3)|0, (0xa<<3)|2, 0, 0,
		(0xa<<3)|1, (0xa<<3)|3, 0, 0,
		(0xa<<3)|4, (0xa<<3)|6, 0, 0,
		(0xa<<3)|5, (0xa<<3)|7, 0, 0,
	};

	int needs_reset;
	unsigned bsp_apicid = 0;

	struct mem_controller ctrl[8];
	unsigned nodes;

	if (!cpu_init_detectedx && boot_cpu()) {
		/* Nothing special needs to be done to find bus 0 */
		/* Allow the HT devices to be found */

		enumerate_ht_chain();

		sio_setup();

		/* Setup the ck804 */
		ck804_enable_rom();
	}

	if (bist == 0) {
		bsp_apicid = init_cpus(cpu_init_detectedx);
	}

	lpc47b397_enable_serial(SERIAL_DEV, CONFIG_TTYS0_BASE);
	uart_init();
	console_init();

	/* Halt if there was a built in self test failure */
	report_bist_failure(bist);

	sio_gpio_setup();

	setup_mb_resource_map();

	needs_reset = setup_coherent_ht_domain();

	wait_all_core0_started();

	// It is said that we should start core1 after all core0 launched
	start_other_cores();
	wait_all_other_cores_started(bsp_apicid);

	needs_reset |= ht_setup_chains_x();

	needs_reset |= ck804_early_setup_x();

	if (needs_reset) {
		printk(BIOS_INFO, "ht reset -\n");
		soft_reset();
	}

	allow_all_aps_stop(bsp_apicid);

	nodes = get_nodes();
	//It's the time to set ctrl now;
	fill_mem_ctrl(nodes, ctrl, spd_addr);

	enable_smbus();

	memreset_setup();
	sdram_initialize(nodes, ctrl);

	post_cache_as_ram();
}

