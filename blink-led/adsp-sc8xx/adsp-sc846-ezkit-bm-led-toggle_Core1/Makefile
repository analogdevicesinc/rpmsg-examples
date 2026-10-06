# Command-line build of the LED toggle example for both supported SoCs:
#
#   sc598 - ADSP-SC598 SHARC+ core,  built with cc21k
#   sc846 - ADSP-SC846 SHARC-FX core, built with ccfx
#
# Usage:
#   make                 # build all targets
#   make sc846           # build one target
#   make CCES_DIR=/opt/analog/cces/3.1.0 sc598
#   make clean
#
# Output goes to build/<target>/$(APP).dxe

CCES_DIR ?= /opt/analog/cces/3.1.0

APP       := adsp-sc8xx-ezkit-bm-led-toggle_Core1
TARGETS   := sc598 sc846
BUILD_DIR := build

SRCS := $(wildcard src/*.c) $(wildcard system/*.c)
HDRS := $(wildcard src/*.h) $(wildcard system/*.h)

CFLAGS := -g -Isrc -Isystem

# ADSP-SC598 (SHARC+): the default LDF selects the core via CORE1/CORE2,
# OTHERCORE leaves booting of the ARM core to someone else.
CC_sc598      := $(CCES_DIR)/cc21k
PROC_sc598    := ADSP-SC598
SIREV_sc598   ?= 0.0
CFLAGS_sc598  := -si-revision $(SIREV_sc598) -O
LDFLAGS_sc598 := -si-revision $(SIREV_sc598) -flags-link -MDCORE1,-MDOTHERCORE

# ADSP-SC846 (SHARC-FX): links against the default LSP.
CC_sc846      := $(CCES_DIR)/ccfx
PROC_sc846    := ADSP-SC846
CFLAGS_sc846  := -O2
LDFLAGS_sc846 :=

.PHONY: all clean $(TARGETS)

all: $(TARGETS)

# $(1) - target name
define TARGET_RULES
$(1)_OBJS := $$(patsubst %.c,$(BUILD_DIR)/$(1)/%.o,$$(SRCS))

$(1): $(BUILD_DIR)/$(1)/$(APP).dxe

$(BUILD_DIR)/$(1)/$(APP).dxe: $$($(1)_OBJS)
	$$(CC_$(1)) -proc $$(PROC_$(1)) $$(LDFLAGS_$(1)) $$^ -o $$@

$(BUILD_DIR)/$(1)/%.o: %.c $$(HDRS)
	@mkdir -p $$(@D)
	$$(CC_$(1)) -proc $$(PROC_$(1)) $$(CFLAGS) $$(CFLAGS_$(1)) -c $$< -o $$@
endef

$(foreach t,$(TARGETS),$(eval $(call TARGET_RULES,$(t))))

clean:
	rm -rf $(BUILD_DIR)
