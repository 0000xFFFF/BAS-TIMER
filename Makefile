CC = gcc
PROG = bas-server
PACK = ./pack
OUT = -o $(PROG)
SOURCES = src/*.c
CFLAGS += -DMG_ENABLE_LINES=1 -DMG_ENABLE_PACKED_FS=1 -D MG_TLS=MG_TLS_OPENSSL -D MG_ENABLE_OPENSSL=1 -lssl -lcrypto

DEBUG_ARGS   = -g -D DEBUG
RELEASE_ARGS = -W -Wall -Wextra -O2 -march=native -s
TESTING_ARGS = -g

debug:
	$(CC) $(DEBUG_ARGS) $(SOURCES) $(CFLAGS) $(OUT)

release:
	$(CC) $(RELEASE_ARGS) $(SOURCES) $(CFLAGS) $(OUT)

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
