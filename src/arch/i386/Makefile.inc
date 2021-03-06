##
## This file is part of the coreboot project.
##
## Copyright (C) 2009-2010 coresystems GmbH
## Copyright (C) 2009 Ronald G. Minnich
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
##

#######################################################################
# Take care of subdirectories
subdirs-y += boot
# subdirs-y += init
subdirs-y += lib
subdirs-y += smp

OPTION_TABLE_H:=
ifeq ($(CONFIG_HAVE_OPTION_TABLE),y)
ramstage-srcs += $(obj)/option_table.c
OPTION_TABLE_H:=$(obj)/option_table.h
endif

#######################################################################
# Build the final rom image
COREBOOT_ROM_DEPENDENCIES:=
ifneq ($(CONFIG_PAYLOAD_NONE),y)
COREBOOT_ROM_DEPENDENCIES+=$(CONFIG_FALLBACK_PAYLOAD_FILE)
endif
ifeq ($(CONFIG_VGA_BIOS),y)
COREBOOT_ROM_DEPENDENCIES+=$(CONFIG_FALLBACK_VGA_BIOS_FILE)
endif
ifeq ($(CONFIG_INTEL_MBI),y)
COREBOOT_ROM_DEPENDENCIES+=$(CONFIG_FALLBACK_MBI_FILE)
endif
ifeq ($(CONFIG_BOOTSPLASH),y)
COREBOOT_ROM_DEPENDENCIES+=$(CONFIG_FALLBACK_BOOTSPLASH_FILE)
endif
ifeq ($(CONFIG_AP_CODE_IN_CAR),y)
COREBOOT_ROM_DEPENDENCIES+=$(obj)/coreboot_ap
endif
ifeq ($(CONFIG_GEODE_VSA_FILE),y)
COREBOOT_ROM_DEPENDENCIES+=$(CONFIG_VSA_FILENAME)
endif

$(obj)/coreboot.rom: $(obj)/coreboot.pre $(obj)/coreboot_ram $(CBFSTOOL) $(call strip_quotes,$(COREBOOT_ROM_DEPENDENCIES))
	@printf "    CBFS       $(subst $(obj)/,,$(@))\n"
	cp $(obj)/coreboot.pre $@.tmp
	if [ -f $(obj)/coreboot_ap ]; \
	then \
		$(CBFSTOOL) $@.tmp add-stage $(obj)/coreboot_ap $(CONFIG_CBFS_PREFIX)/coreboot_ap $(CBFS_COMPRESS_FLAG); \
	fi
	$(CBFSTOOL) $@.tmp add-stage $(obj)/coreboot_ram $(CONFIG_CBFS_PREFIX)/coreboot_ram $(CBFS_COMPRESS_FLAG)
ifeq ($(CONFIG_PAYLOAD_NONE),y)
	@printf "    PAYLOAD    \e[1;31mnone (as specified by user)\e[0m\n"
else
	@printf "    PAYLOAD    $(CONFIG_FALLBACK_PAYLOAD_FILE) (compression: $(CBFS_PAYLOAD_COMPRESS_NAME))\n"
	$(CBFSTOOL) $@.tmp add-payload $(CONFIG_FALLBACK_PAYLOAD_FILE) $(CONFIG_CBFS_PREFIX)/payload $(CBFS_PAYLOAD_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_VGA_BIOS),y)
	@printf "    VGABIOS    $(CONFIG_FALLBACK_VGA_BIOS_FILE) $(CONFIG_FALLBACK_VGA_BIOS_ID)\n"
	$(CBFSTOOL) $@.tmp add $(CONFIG_FALLBACK_VGA_BIOS_FILE) "pci$(CONFIG_FALLBACK_VGA_BIOS_ID).rom" optionrom
endif
ifeq ($(CONFIG_INTEL_MBI),y)
	@printf "    MBI        $(CONFIG_FALLBACK_MBI_FILE)\n"
	$(CBFSTOOL) $@.tmp add $(CONFIG_FALLBACK_MBI_FILE) mbi.bin mbi
endif
ifeq ($(CONFIG_BOOTSPLASH),y)
	@printf "    BOOTSPLASH $(CONFIG_FALLBACK_BOOTSPLASH_FILE)\n"
	$(CBFSTOOL) $@.tmp add $(CONFIG_FALLBACK_BOOTSPLASH_FILE) bootsplash.jpg bootsplash
endif
ifeq ($(CONFIG_GEODE_VSA_FILE),y)
	@printf "    VSA        $(CONFIG_VSA_FILENAME)\n"
	$(OBJCOPY) --set-start 0x20 --adjust-vma 0x60000 -I binary -O elf32-i386 -B i386 $(CONFIG_VSA_FILENAME) $(obj)/vsa.o
	$(LD) -m elf_i386 -e 0x60020 --section-start .data=0x60000 $(obj)/vsa.o -o $(obj)/vsa.elf
	$(CBFSTOOL) $@.tmp add-stage $(obj)/vsa.elf vsa
endif
	mv $@.tmp $@
	@printf "    CBFSPRINT  $(subst $(obj)/,,$(@))\n\n"
	$(CBFSTOOL) $@ print

#######################################################################
# i386 specific tools

$(OPTION_TABLE_H): $(objutil)/options/build_opt_tbl $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout
	@printf "    OPTION     $(subst $(obj)/,,$(@))\n"
	$(objutil)/options/build_opt_tbl --config $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout --header $@

$(obj)/option_table.c: $(objutil)/options/build_opt_tbl $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout
	@printf "    OPTION     $(subst $(obj)/,,$(@))\n"
	$(objutil)/options/build_opt_tbl --config $(top)/src/mainboard/$(MAINBOARDDIR)/cmos.layout --option $@

$(objutil)/options/build_opt_tbl: $(top)/util/options/build_opt_tbl.c $(top)/src/include/pc80/mc146818rtc.h $(top)/src/include/boot/coreboot_tables.h
	@printf "    HOSTCC     $(subst $(obj)/,,$(@))\n"
	$(HOSTCC) $(HOSTCFLAGS) $< -o $@

#######################################################################
# Build the coreboot_ram (stage 2)

$(obj)/coreboot_ram: $(obj)/coreboot_ram.o $(src)/arch/i386/coreboot_ram.ld #ldoptions
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC) -nostdlib -nostartfiles -static -o $@ -L$(obj) -T $(src)/arch/i386/coreboot_ram.ld $(obj)/coreboot_ram.o
	$(NM) -n $(obj)/coreboot_ram | sort > $(obj)/coreboot_ram.map
	$(OBJCOPY) --only-keep-debug $@ $(obj)/coreboot_ram.debug
	$(OBJCOPY) --strip-debug $@
	$(OBJCOPY) --add-gnu-debuglink=$(obj)/coreboot_ram.debug $@

$(obj)/coreboot_ram.o: $(obj)/arch/i386/lib/c_start.ramstage.o $$(driver-objs) $(obj)/coreboot.a $(LIBGCC_FILE_NAME)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC) -nostdlib -r -o $@ $(obj)/arch/i386/lib/c_start.ramstage.o $(driver-objs) -Wl,--wrap,__divdi3 -Wl,--wrap,__udivdi3 -Wl,--wrap,__moddi3 -Wl,--wrap,__umoddi3 -Wl,--start-group $(obj)/coreboot.a $(LIBGCC_FILE_NAME) -Wl,--end-group

$(obj)/coreboot.a: $$(ramstage-objs)
	@printf "    AR         $(subst $(obj)/,,$(@))\n"
	rm -f $(obj)/coreboot.a
	$(AR) cr $(obj)/coreboot.a $^

#######################################################################
# coreboot_ap.rom

ifeq ($(CONFIG_AP_CODE_IN_CAR),y)

$(obj)/coreboot_ap: $(obj)/mainboard/$(MAINBOARDDIR)/ap_romstage.o
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC) -nostdlib -nostartfiles -static -o $@ -L$(obj) -T $(src)/arch/i386/init/ldscript_apc.lb $^
	$(OBJCOPY) --only-keep-debug $@ $(obj)/coreboot_ap.debug
	$(OBJCOPY) --strip-debug $@
	$(OBJCOPY) --add-gnu-debuglink=$(obj)/coreboot_ap.debug $@
	$(NM) -n $(obj)/coreboot_ap | sort > $(obj)/coreboot_ap.map


endif

#######################################################################
# done

crt0s = $(src)/arch/i386/init/prologue.inc
ldscripts =
ldscripts += $(src)/arch/i386/init/ldscript_fallback_cbfs.lb
ifeq ($(CONFIG_BIG_BOOTBLOCK),y)
crt0s += $(src)/cpu/x86/16bit/entry16.inc
ldscripts += $(src)/cpu/x86/16bit/entry16.lds
endif
crt0s += $(src)/cpu/x86/32bit/entry32.inc
ldscripts += $(src)/cpu/x86/32bit/entry32.lds
ifeq ($(CONFIG_BIG_BOOTBLOCK),y)
crt0s += $(src)/cpu/x86/16bit/reset16.inc
ldscripts += $(src)/cpu/x86/16bit/reset16.lds
crt0s += $(src)/arch/i386/lib/id.inc
ldscripts += $(src)/arch/i386/lib/id.lds
endif

crt0s += $(src)/cpu/x86/fpu_enable.inc
ifeq ($(CONFIG_SSE),y)
crt0s += $(src)/cpu/x86/sse_enable.inc
endif

crt0s += $(cpu_incs)

#
# FIXME move to CPU_INTEL_SOCKET_MPGA604
#
ifeq ($(CONFIG_BOARD_TYAN_S2735),y)
crt0s += $(src)/cpu/intel/car/cache_as_ram.inc
endif

ifeq ($(CONFIG_LLSHELL),y)
crt0s += $(src)/arch/i386/llshell/llshell.inc
endif

crt0s += $(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc

ifeq ($(CONFIG_SSE),y)
crt0s += $(src)/cpu/x86/sse_disable.inc
endif
ifeq ($(CONFIG_MMX),y)
crt0s += $(src)/cpu/x86/mmx_disable.inc
endif

ifeq ($(CONFIG_BIG_BOOTBLOCK),y)
crt0s += $(chipset_bootblock_inc)
ldscripts += $(chipset_bootblock_lds)
endif

ifeq ($(CONFIG_ROMCC),y)
crt0s += $(src)/arch/i386/init/crt0_romcc_epilogue.inc
endif

ifeq ($(CONFIG_ROMCC),y)
ROMCCFLAGS ?= -mcpu=p2 -O2

$(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc: $(src)/mainboard/$(MAINBOARDDIR)/romstage.c $(objutil)/romcc/romcc $(OPTION_TABLE_H) $(obj)/build.h $(obj)/config.h
	printf "    ROMCC      romstage.inc\n"
	$(ROMCC) -c -S $(ROMCCFLAGS) -D__PRE_RAM__ -I. $(INCLUDES) $< -o $@
else

$(obj)/mainboard/$(MAINBOARDDIR)/ap_romstage.o: $(src)/mainboard/$(MAINBOARDDIR)/ap_romstage.c $(OPTION_TABLE_H)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC) -MMD $(CFLAGS) -I$(src) -I. -I$(obj) -c $(src)/mainboard/$(MAINBOARDDIR)/ap_romstage.c -o $@

$(obj)/mainboard/$(MAINBOARDDIR)/romstage.pre.inc: $(src)/mainboard/$(MAINBOARDDIR)/romstage.c $(OPTION_TABLE_H) $(obj)/build.h
	@printf "    CC         romstage.inc\n"
	$(CC) -MMD $(CFLAGS) -D__PRE_RAM__ -I$(src) -I. -I$(obj) -c -S $< -o $@

$(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc: $(obj)/mainboard/$(MAINBOARDDIR)/romstage.pre.inc
	@printf "    POST       romstage.inc\n"
	sed -e 's/\.rodata/.rom.data/g' -e 's/\.text/.section .rom.text/g' $^ > $@.tmp
	mv $@.tmp $@
endif

# Things that appear in every board
romstage-srcs += $(obj)/mainboard/$(MAINBOARDDIR)/crt0.s
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mainboard.c
ifeq ($(CONFIG_GENERATE_MP_TABLE),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/mptable.c
endif
ifeq ($(CONFIG_GENERATE_PIRQ_TABLE),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/irq_tables.c
endif
ifeq ($(CONFIG_BOARD_HAS_HARD_RESET),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/reset.c
endif
ifeq ($(CONFIG_GENERATE_ACPI_TABLES),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/acpi_tables.c
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/dsdt.asl
# make doesn't have arithmetic operators or greater-than comparisons
ifeq ($(subst 5,4,$(CONFIG_ACPI_SSDTX_NUM)),4)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt2.asl
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt3.asl
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt4.asl
endif
ifeq ($(CONFIG_ACPI_SSDTX_NUM),5)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/ssdt5.asl
endif
ifeq ($(CONFIG_BOARD_HAS_FADT),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/fadt.c
endif
endif

ifeq ($(CONFIG_HAVE_BUS_CONFIG),y)
ramstage-srcs += src/mainboard/$(MAINBOARDDIR)/get_bus_conf.c
endif

ifeq ($(CONFIG_TINY_BOOTBLOCK),y)
include $(src)/arch/i386/Makefile.bootblock.inc
else
include $(src)/arch/i386/Makefile.bigbootblock.inc
endif
