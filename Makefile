# PS5 RAR extractor payload
# Requires: PS5_PAYLOAD_SDK, and dmc_unrar.c in this directory (see README)

PS5_HOST ?= ps5
PS5_PORT ?= 9021

ifdef PS5_PAYLOAD_SDK
 include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
 $(error PS5_PAYLOAD_SDK is undefined)
endif

ELF := unrar_extract.elf

# dmc_unrar.c from https://github.com/DrMcCoy/dmc_unrar - add to this dir
# Note: main.c includes dmc_unrar.c directly, so we only compile main.c
# dmc_unrar.c and game_register.c are included directly in main.c
SOURCES := main.c

CFLAGS := -Wall -Werror -g -I.
LDFLAGS := -lkernel

all: $(ELF)

$(ELF): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(ELF)

test: $(ELF)
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(PS5_PORT) $<

.PHONY: all clean test
