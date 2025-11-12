CC = gcc
PROG = bas-server
PACK = ./pack
OUT = -o $(PROG)
SOURCES = src/*.c
RELEASE_SOURCES = $(filter-out src/mongoose.c, $(wildcard src/*.c))
CFLAGS += -DMG_ENABLE_LINES=1 -DMG_ENABLE_PACKED_FS=1 -D MG_TLS=MG_TLS_OPENSSL -D MG_ENABLE_OPENSSL=1 -lssl -lcrypto

DEBUG_ARGS   = -g -D DEBUG
RELEASE_ARGS = -Wall -Wextra -Wpedantic -Wformat=2 -Wcast-qual -Wcast-align \
               -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith \
               -Wstrict-prototypes -Wmissing-prototypes -Wstringop-overflow \
               -Wswitch-enum -Wundef -Wuninitialized -Wdouble-promotion \
               -Wnull-dereference -Walloc-zero -Walloca -Wvla \
               -Werror -O2 -march=native -s \
               -Wno-cast-qual -Wno-alloca -Wno-discarded-qualifiers src/mongoose.c
TESTING_ARGS = -g

debug:
	$(CC) $(DEBUG_ARGS) $(SOURCES) $(CFLAGS) $(OUT)

release:
	$(CC) $(RELEASE_ARGS) $(RELEASE_SOURCES) $(CFLAGS) $(OUT)

testing:
	$(CC) $(TESTING_ARGS) $(SOURCES) $(CFLAGS) $(OUT)

clean:
	rm bas-server
	rm var/infos.bin

pack: certs/*
	$(CC) src/pack/pack.c -o $(PACK)
	$(PACK) certs/* > src/packed_fs.c

install:
	sudo ln -sfr bas-server /usr/local/bin/bas-server
