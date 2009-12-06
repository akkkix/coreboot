#include <pc80/mc146818rtc.h>
#include <part/fallback_boot.h>

#ifndef CONFIG_MAX_REBOOT_CNT
#error "CONFIG_MAX_REBOOT_CNT not defined"
#endif
#if  CONFIG_MAX_REBOOT_CNT > 15
#error "CONFIG_MAX_REBOOT_CNT too high"
#endif

static unsigned char cmos_read(unsigned char addr)
{
	int offs = 0;
	if (addr >= 128) {
		offs = 2;
		addr -= 128;
	}
	outb(addr, RTC_BASE_PORT + offs + 0);
	return inb(RTC_BASE_PORT + offs + 1);
}

static void cmos_write(unsigned char val, unsigned char addr)
{
	int offs = 0;
	if (addr >= 128) {
		offs = 2;
		addr -= 128;
	}
	outb(addr, RTC_BASE_PORT + offs + 0);
	outb(val, RTC_BASE_PORT + offs + 1);
}

static int cmos_error(void)
{
	unsigned char reg_d;
	/* See if the cmos error condition has been flagged */
	reg_d = cmos_read(RTC_REG_D);
	return (reg_d & RTC_VRT) == 0;
}

static int cmos_chksum_valid(void)
{
	unsigned char addr;
	unsigned long sum, old_sum;
	sum = 0;
	/* Comput the cmos checksum */
	for(addr = CONFIG_LB_CKS_RANGE_START; addr <= CONFIG_LB_CKS_RANGE_END; addr++) {
		sum += cmos_read(addr);
	}
	sum = (sum & 0xffff) ^ 0xffff;

	/* Read the stored checksum */
	old_sum = cmos_read(CONFIG_LB_CKS_LOC) << 8;
	old_sum |=  cmos_read(CONFIG_LB_CKS_LOC+1);

	return sum == old_sum;
}


static int last_boot_normal(void)
{
	unsigned char byte;
	byte = cmos_read(RTC_BOOT_BYTE);
	return (byte & (1 << 1));
}

static int do_normal_boot(void)
{
	unsigned char byte;

	if (cmos_error() || !cmos_chksum_valid()) {
		/* There are no impossible values, no checksums so just
		 * trust whatever value we have in the the cmos,
		 * but clear the fallback bit.
		 */
		byte = cmos_read(RTC_BOOT_BYTE);
		byte &= 0x0c;
		byte |= CONFIG_MAX_REBOOT_CNT << 4;
		cmos_write(byte, RTC_BOOT_BYTE);
	}

	/* The RTC_BOOT_BYTE is now o.k. see where to go. */
	byte = cmos_read(RTC_BOOT_BYTE);
	
	/* Are we in normal mode? */
	if (byte & 1) {
		byte &= 0x0f; /* yes, clear the boot count */
	}

	/* Properly set the last boot flag */
	byte &= 0xfc;
	if ((byte >> 4) < CONFIG_MAX_REBOOT_CNT) {
		byte |= (1<<1);
	}

	/* Are we already at the max count? */
	if ((byte >> 4) < CONFIG_MAX_REBOOT_CNT) {
		byte += 1 << 4; /* No, add 1 to the count */
	}
	else {
		byte &= 0xfc;	/* Yes, put in fallback mode */
	}

	/* Save the boot byte */
	cmos_write(byte, RTC_BOOT_BYTE);

	return (byte & (1<<1));
}

static unsigned read_option(unsigned start, unsigned size, unsigned def)
{
#if CONFIG_USE_OPTION_TABLE == 1
	unsigned byte;
	byte = cmos_read(start/8);
	return (byte >> (start & 7U)) & ((1U << size) - 1U);
#else
	return def;
#endif
}