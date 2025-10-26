GCC = gcc
PROG ?= bas-server                # Program we are building
PACK ?= ./pack                    # Packing executable
DELETE = rm -rf                   # Command to remove files
OUT ?= -o $(PROG)                 # Compiler argument for output file
SOURCES = src/*.c                 # Source code files, packed_fs.c contains ca.pem, which contains CA certs for TLS
CFLAGS = -W -Wall -Wextra -g -I.  # Build options

DEBUG_ARGS     = -D DEBUG -g
RELEASE_ARGS   = -O2 -s

# Mongoose build options. See https://mongoose.ws/documentation/#build-options
CFLAGS_MONGOOSE += -DMG_ENABLE_LINES=1 -DMG_ENABLE_PACKED_FS=1
CFLAGS_EXTRA ?= -D MG_TLS=MG_TLS_OPENSSL -D MG_ENABLE_OPENSSL=1 -lssl -lcrypto

ifeq ($(OS),Windows_NT)   # Windows settings. Assume MinGW compiler. To use VC: make CC=cl CFLAGS=/MD OUT=/Feprog.exe
  PROG ?= example.exe           # Use .exe suffix for the binary
  PACK = pack.exe               # Packing executable
  CC = gcc                      # Use MinGW gcc compiler
  CFLAGS += -lws2_32            # Link against Winsock library
  DELETE = cmd /C del /Q /F /S  # Command prompt command to delete files
  OUT ?= -o $(PROG)             # Build output
  MAKE += WINDOWS=1 CC=$(CC)
endif

debug:
	$(CC) $(DEBUG_ARGS) $(SOURCES) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(OUT)

release: $(PROG)

$(PROG): $(SOURCES)       # Build program from sources
	$(CC) $(RELEASE_ARGS) $(SOURCES) $(CFLAGS) $(CFLAGS_MONGOOSE) $(CFLAGS_EXTRA) $(OUT)

clean:                    # Cleanup. Delete built program and all build artifacts
	$(DELETE) $(PROG) *.o *.obj *.exe *.dSYM mbedtls $(PACK)

# Generate packed filesystem for serving cert
pack: certs/*
	$(CC) src/pack/pack.c -o $(PACK)
	$(PACK) certs/* > src/packed_fs.c

# see https://mongoose.ws/tutorials/tls/#how-to-build for TLS build options

mbedtls:                  # Pull and build mbedTLS library
	git clone --depth 1 -b v2.28.2 https://github.com/mbed-tls/mbedtls $@
	$(MAKE) -C mbedtls/library

install:
	sudo ln -sfr bas-server /usr/local/bin/bas-server
