/*
 * inteltool - dump all registers on an Intel CPU + chipset based system.
 *
 * Copyright (C) 2008-2010 by coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include "inteltool.h"

/*
 * (G)MCH MMIO Config Space
 */
int print_mchbar(struct pci_dev *nb)
{
	int i, size = (16 * 1024);
	volatile uint8_t *mchbar;
 	uint64_t mchbar_phys;

	printf("\n============= MCHBAR ============\n\n");

	switch (nb->device_id) {
	case PCI_DEVICE_ID_INTEL_82915:
	case PCI_DEVICE_ID_INTEL_82945GM:
	case PCI_DEVICE_ID_INTEL_82945GSE:
	case PCI_DEVICE_ID_INTEL_82945P:
 	case PCI_DEVICE_ID_INTEL_82975X:
		mchbar_phys = pci_read_long(nb, 0x44) & 0xfffffffe;
		break;
 	case PCI_DEVICE_ID_INTEL_PM965:
 	case PCI_DEVICE_ID_INTEL_82Q35:
 	case PCI_DEVICE_ID_INTEL_82G33:
 	case PCI_DEVICE_ID_INTEL_82Q33:
 		mchbar_phys = pci_read_long(nb, 0x48) & 0xfffffffe;
 		mchbar_phys |= ((uint64_t)pci_read_long(nb, 0x4c)) << 32;
 		break;
 	case PCI_DEVICE_ID_INTEL_Q965:
	case PCI_DEVICE_ID_INTEL_ATOM_DXXX:
	case PCI_DEVICE_ID_INTEL_ATOM_NXXX:
 		mchbar_phys = pci_read_long(nb, 0x48);

		/* Test if bit 0 of the MCHBAR reg is 1 to enable memory reads.
		 * If it isn't, try to set it. This may fail, because there is 
		 * some bit that locks that bit, and isn't in the public 
		 * datasheets.
		 */

		if(!(mchbar_phys & 1))
		{
			printf("Access to the MCHBAR is currently disabled, "\
						"attempting to enable.\n");
			mchbar_phys |= 0x1;
			pci_write_long(nb, 0x48, mchbar_phys);
	 		if(pci_read_long(nb, 0x48) & 1)
				printf("Enabled successfully.\n");
			else
				printf("Enable FAILED!\n");
		}
		mchbar_phys &= 0xfffffffe;
 		mchbar_phys |= ((uint64_t)pci_read_long(nb, 0x4c)) << 32;
 		break;
	case PCI_DEVICE_ID_INTEL_82443LX:
	case PCI_DEVICE_ID_INTEL_82443BX:
	case PCI_DEVICE_ID_INTEL_82810:
	case PCI_DEVICE_ID_INTEL_82810E_MC:
	case PCI_DEVICE_ID_INTEL_82810DC:
	case PCI_DEVICE_ID_INTEL_82830M:
		printf("This northbrigde does not have MCHBAR.\n");
		return 1;
	case PCI_DEVICE_ID_INTEL_GS45:
		mchbar_phys = pci_read_long(nb, 0x48) & 0xfffffffe;
		mchbar_phys |= ((uint64_t)pci_read_long(nb, 0x4c)) << 32;
 		break;
	default:
		printf("Error: Dumping MCHBAR on this northbridge is not (yet) supported.\n");
		return 1;
	}

	mchbar = map_physical(mchbar_phys, size);

	if (mchbar == NULL) {
		perror("Error mapping MCHBAR");
		exit(1);
	}

	printf("MCHBAR = 0x%08llx (MEM)\n\n", mchbar_phys);

	for (i = 0; i < size; i += 4) {
		if (*(uint32_t *)(mchbar + i))
			printf("0x%04x: 0x%08x\n", i, *(uint32_t *)(mchbar+i));
	}

	unmap_physical((void *)mchbar, size);
	return 0;
}


