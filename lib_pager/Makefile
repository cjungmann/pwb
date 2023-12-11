TARGET_ROOT = pager
TARGET_SHARED = lib${TARGET_ROOT}.so
TARGET_STATIC = lib${TARGET_ROOT}.a
TARGET_TEST = test

PREFIX ?= /usr/local

# Change if source files not in base directory:
SRC = .

CFLAGS = -Wall -Werror -std=c99 -pedantic -ggdb -fsanitize=address -fvisibility=hidden
LFLAGS =
LDFLAGS = -fsanitize=address

# Uncomment the following if target is a Shared library
CFLAGS += -fPIC
LFLAGS += --shared

# Build module list (info make -> "Functions" -> "File Name Functions")
MODULES = $(addsuffix .o,$(filter-out test_,$(basename $(wildcard $(SRC)/*.c))))
TEST_MODULES = $(addsuffix .o,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))

# Libraries need header files.  Set the following accordingly:
HEADERS =

# Declare non-filename targets
.PHONY: all preview install uninstall clean help

all: ${TARGET_SHARED} ${TARGET_STATIC}

preview:
	@echo Beginning build of ${TARGET_ROOT} libraries.
	@echo ${TARGET_SHARED} shared library and
	@echo ${TARGET_STATIC} static library, using modules
	@echo modules: ${MODULES}
	@echo test modules: $(TEST_MODULES)

${TARGET_SHARED}: ${MODULES}
	${CC} ${CFLAGS} --shared -o $@ ${MODULES}

${TARGET_STATIC}: ${MODULES}
	ar rcs $@ ${MODULES}

%o : %c
	$(CC) $(CFLAGS) -c -o $@ $<

# For executable target:
# install:
# 	install -D -mode=775 $(TARGET) $(PREFIX)/bin

# For shared library targets:
# install:
# 	install -D -mode=644 $(HEADERS) $(PREFIX)/include
# 	install -D -mode=775 $(TARGET) $(PREFIX)/lib
# 	ldconfig $(PREFIX)/lib

test: $(TEST_MODULES)
	$(CC) $(LDFLAGS) -o $(TARGET_TEST) $^ $(TARGET_STATIC) -ltinfo 

# Remove the ones you don't need:
uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -f $(PREFIX)/include/$(HEADERS)

clean:
	rm -f $(TARGET)
	rm -f $(MODULES)

help:
	@echo "Makefile options:"
	@echo
	@echo "  test       to build test program using library"
	@echo "  install    to install project"
	@echo "  uninstall  to uninstall project"
	@echo "  clean      to remove generated files"
	@echo "  help       this display"
