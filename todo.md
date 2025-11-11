# compiler options (warnings)
## gcc
RELEASE_ARGS = -Wall -Wextra -Wpedantic -Wformat=2 -Wcast-qual -Wcast-align \
               -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith \
               -Wstrict-prototypes -Wmissing-prototypes -Wstringop-overflow \
               -Wswitch-enum -Wundef -Wuninitialized -Wdouble-promotion \
               -Wnull-dereference -Walloc-zero -Walloca -Wvla \
               -Werror -O2 -march=native -s
### even stronger:
               -Wlogical-op -Wduplicated-cond -Wduplicated-branches -Wno-unused-parameter

## or switch compiler to clang
RELEASE_ARGS = -Weverything -Werror -Wno-c++98-compat -O2 -march=native -s

### Weverything is noisy so use:
-Weverything \
-Werror \
-Wno-c++98-compat \
-Wno-c++98-compat-pedantic \
-Wno-padded \
-Wno-gnu-zero-variadic-macro-arguments \
-O2 -march=native -s

## max warning for both gcc & clang
-Wall -Wextra -Wpedantic -Wformat=2 -Wconversion -Wsign-conversion -Wcast-qual \
-Wshadow -Wstrict-overflow=5 -Wdouble-promotion -Wundef -Wswitch-enum \
-Wuninitialized -Wnull-dereference -Werror -O2 -march=native -s

## brutal warning treatment for debug
-fsanitize=address,undefined
-g


# top:
- min max temp from wttr.in BRUH no values in j1 j2 formats
- make new TEMPERATURE COLORS ???
    - maybe no bg color?
- add solar anim, when on, next to temp (like check/warn)

# find in files:
// TODO: request_send_quick can fail --> print/handle that

# not important:
- js web ui code is ugly clean it up

