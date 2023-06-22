# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023 NXP

include ../scripts/Makefile.include

bindir ?= /usr/bin

ifeq ($(srctree),)
srctree := $(patsubst %/,%,$(dir $(CURDIR)))
srctree := $(patsubst %/,%,$(dir $(srctree)))
endif

# Do not use make's built-in rules
# (this improves performance and avoids hard-to-debug behaviour);
MAKEFLAGS += -r

CFLAGS += -O2 -Wall -Wno-pointer-sign -Wno-stringop-truncation -g -D_GNU_SOURCE -I$(OUTPUT)include

ALL_TARGETS := obx_system_stats_d obx_stats_cli
ALL_PROGRAMS := $(patsubst %,$(OUTPUT)%,$(ALL_TARGETS))

all: $(ALL_PROGRAMS)

export srctree OUTPUT CC LD CFLAGS
include $(srctree)/tools/build/Makefile.include

OBX_CLI_IN := $(OUTPUT)obx_system_stats_d-in.o
$(OBX_CLI_IN): prepare FORCE
	$(Q)$(MAKE) $(build)=obx_system_stats_d
$(OUTPUT)obx_system_stats_d: $(OBX_CLI_IN)
	$(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

OBX_CLI_IN := $(OUTPUT)obx_stats_cli-in.o
$(OBX_CLI_IN): prepare FORCE
	$(Q)$(MAKE) $(build)=obx_stats_cli
$(OUTPUT)obx_stats_cli: $(OBX_CLI_IN)
	$(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f $(ALL_PROGRAMS)
	rm -rf $(OUTPUT)include/
	find $(or $(OUTPUT),.) -name '*.o' -delete
	find $(or $(OUTPUT),.) -name '\.*.o.d' -delete
	find $(or $(OUTPUT),.) -name '\.*.o.cmd' -delete

install: $(ALL_PROGRAMS)
	install -d -m 755 $(DESTDIR)$(bindir);		\
	for program in $(ALL_PROGRAMS); do		\
		install $$program $(DESTDIR)$(bindir);	\
	done

FORCE:

.PHONY: all install clean FORCE prepare
