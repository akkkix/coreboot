ramstage-y += socket_AM3.c
subdirs-y += ../model_10xxx
subdirs-y += ../quadcore
subdirs-y += ../mtrr
subdirs-y += ../microcode
subdirs-y += ../../x86/tsc
subdirs-y += ../../x86/lapic
subdirs-y += ../../x86/cache
subdirs-y += ../../x86/pae
subdirs-y += ../../x86/smm
subdirs-y += ../../x86/mtrr

cpu_incs += $(src)/cpu/amd/car/cache_as_ram.inc
