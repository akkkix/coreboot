/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008 Advanced Micro Devices, Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef SB600_CHIP_H
#define SB600_CHIP_H

struct southbridge_amd_sb600_config 
{
	unsigned int ide0_enable : 1;
	unsigned int sata0_enable : 1;
	unsigned long hda_viddid;
};
struct chip_operations;
extern struct chip_operations southbridge_amd_sb600_ops;

#endif /* SB600_CHIP_H */