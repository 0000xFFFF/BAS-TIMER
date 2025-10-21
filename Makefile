GCC = gcc

FILES = src/*.c
DEBUG_ARGS     = -D DEBUG -g
DEBUGFULL_ARGS = -D DEBUG -D DEBUGFULL -g
QUICK_ARGS     = -D DEBUG -D QUICK -g
RELEASE_ARGS   = -Wall -Wextra -O2 -s
LIBS = -pthread
OUTPUT = bas-server

debug:
	$(GCC) $(DEBUG_ARGS) $(LIBS) $(FILES) -o $(OUTPUT)

debugfull:
	$(GCC) $(DEBUGFULL_ARGS) $(LIBS) $(FILES) -o $(OUTPUT)

quick:
	$(GCC) $(QUICK_ARGS) $(LIBS) $(FILES) -o $(OUTPUT)

test: src/logger.c src/utils.c src/tests/main.c
	$(GCC) $(DEBUG_ARGS) $(LIBS) src/logger.c src/utils.c src/tests/main.c -o test

release: src/*.c
	$(GCC) $(RELEASE_ARGS) $(LIBS) $(FILES) -o $(OUTPUT)

install:
	sudo ln -sfr bas-server /usr/local/bin/bas-server
