##
## This file is part of the superiotool project.
##
## Copyright (C) 2007 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
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

PROGRAM = superiotool

CC      = gcc
INSTALL = /usr/bin/install
PREFIX  = /usr/local

# TODO: -pedantic
CFLAGS = -O2 -Wall -Werror -Wstrict-prototypes -Wundef -Wstrict-aliasing \
         -Werror-implicit-function-declaration -ansi

OBJS = superiotool.o ali.o fintek.o ite.o nsc.o smsc.o winbond.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS) superiotool.h
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS)

install: $(PROGRAM)
	$(INSTALL) $(PROGRAM) $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man8
	$(INSTALL) $(PROGRAM).8 $(PREFIX)/share/man/man8

clean:
	rm -f $(PROGRAM) *.o

.PHONY: all install clean
