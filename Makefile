TARGET_ROOT = pwb
TARGET = $(TARGET_ROOT)
BUILTIN = $(TARGET_ROOT)
ENABLER = $(addprefix enable_,$(TARGET_ROOT))
SOURCER = $(addprefix $(TARGET_ROOT),_sources)

PREFIX ?= /usr/local

# Change if source files not in base directory:
SRC = .

# These will be set by ./configure
LIB_FLAGS =
LIB_MODULES =

CFLAGS = -Wall -Werror -std=c99 -pedantic -ggdb
LFLAGS =
LDFLAGS = -lm $(LIB_FLAGS)
INCFLAGS =

# Uncomment the following if target is a Shared library
CFLAGS += -fPIC
LFLAGS += --shared

CFLAGS += -I/usr/include/bash -I/usr/include/bash/include $(INCFLAGS)

# Build module list (info make -> "Functions" -> "File Name Functions")
MODULES = $(addsuffix .o,$(basename $(wildcard $(SRC)/*.c)))

HEADERS = $(wildcard $(SRC)/*.h)

UTILITIES = $(filter $(basename $(wildcard *)),$(wildcard pwb_*))


# Declare non-filename targets
.PHONY: all install uninstall clean help

all: $(TARGET) $(ENABLER)

$(TARGET) : $(MODULES)
ifndef LIB_FLAGS
ifndef LIB_MODULES
	$(error "Run 'configure' to setup libraries")
endif
endif
	$(CC) $(LFLAGS) -o $@ $(MODULES) $(LIB_MODULES) $(LDFLAGS)

$(ENABLER):
	@echo "#!/usr/bin/env bash"                         > $(ENABLER)
	@echo "read -n1 -p\"Using enable $$\\\\( pwb_enable \\\\) is deprecated. Please just enable 'pwb'\"" >> $(ENABLER)
	@echo "echo -f $(PREFIX)/lib/bash/$(TARGET) $(BUILTIN)" >> $(ENABLER)
	chmod a+x $(ENABLER)

*.c: $(HEADERS)
	@echo "Forcing full recompile after any header file change"
	touch *.c

%o: %c
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(ENABLER)
	install -D --mode=775 $(ENABLER) $(PREFIX)/bin
	install -D --mode=775 $(TARGET) -t $(PREFIX)/lib/bash
	mkdir --mode=755 -p $(PREFIX)/share/man/man1
	mkdir --mode=755 -p $(PREFIX)/share/man/man7
	soelim $(TARGET_ROOT).1 | gzip -c - > $(PREFIX)/share/man/man1/$(TARGET_ROOT).1.gz
	soelim $(TARGET_ROOT).7 | gzip -c - > $(PREFIX)/share/man/man7/$(TARGET_ROOT).7.gz
# install SOURCER and sources
	rm -f $(PREFIX)/bin/$(SOURCER)
	sed -e s^#PREFIX#^$(PREFIX)^ -e s^#BUILTIN#^$(BUILTIN)^ $(SOURCER) > $(PREFIX)/bin/$(SOURCER)
	chmod a+x $(PREFIX)/bin/$(SOURCER)
	install -D $(BUILTIN)_sources.d/pwb_* -t$(PREFIX)/lib/$(BUILTIN)_sources

install_utilities:
	@echo "UTILITIES are " $(UTILITIES)
	install -D --mode=775 $(UTILITIES) $(PREFIX)/bin

uninstall_utilities:
	rm -f $(PREFIX)/bin/$(UTILITIES)

uninstall:
	rm -f $(PREFIX)/bin/$(ENABLER)
	rm -f $(PREFIX)/lib/bash/$(TARGET)
	rm -f $(PREFIX)/bin/$(UTILITIES)
	rm -f $(PREFIX)/share/man/man1/$(TARGET_ROOT).1.gz
	rm -f $(PREFIX)/share/man/man7/$(TARGET_ROOT).7.gz
# uninstall SOURCER stuff:
	rm -rf $(PREFIX)/lib/$(BUILTIN)_sources
	rm -f $(PREFIX)/bin/$(SOURCER)
# Utilities won't work if builtin is uninstalled:
	rm -f $(PREFIX)/bin/$(UTILITIES)
	rm -f $(ENABLER)

clean:
	rm -f $(TARGET)
	rm -f $(ENABLER)
	rm -f $(MODULES)

help:
	@echo "Makefile options:"
	@echo
	@echo "  install    to install project"
	@echo "  uninstall  to uninstall project"
	@echo "  clean      to remove generated files"
	@echo "  help       this display"
