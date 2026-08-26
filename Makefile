# This Makefile does not build the examples. Each project is built
# individually using the ADI CrossCore Embedded Studio (CCES) SDK.
# This Makefile installs the prebuilt firmware binaries onto the
# target Linux root filesystem.

DESTDIR      ?=
INSTALL      ?= install
INSTALL_DATA ?= $(INSTALL) -m 0644

firmwaredir   = /lib/firmware/adi

.PHONY: all install

all:

install:
	$(INSTALL) -d $(DESTDIR)$(firmwaredir)
	$(INSTALL_DATA) \
		echo_example/rpmsg_echo_example_Core1/Debug/rpmsg_echo_example_Core1.ldr \
		$(DESTDIR)$(firmwaredir)/rpmsg_echo_sharc_core1.ldr
	$(INSTALL_DATA) \
		echo_example/rpmsg_echo_example_Core2/Debug/rpmsg_echo_example_Core2.ldr \
		$(DESTDIR)$(firmwaredir)/rpmsg_echo_sharc_core2.ldr
	$(INSTALL_DATA) \
		fir_example/Debug/rpmsg_shared_mem_example.ldr \
		$(DESTDIR)$(firmwaredir)/rpmsg_fir_sharc_core1.ldr
