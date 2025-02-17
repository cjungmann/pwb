TARGET_ROOT = pwb
TARGET = $(TARGET_ROOT)
BUILTIN = $(TARGET_ROOT)
SOURCER = $(addprefix $(TARGET_ROOT),_sources)

PREFIX ?= /usr/local

# Change if source files not in base directory:
SRC = .

# These will be set by ./configure
LIB_FLAGS =
LIB_MODULES =

ifndef LIB_FLAGS
ifndef LIB_MODULES
$(error "Run 'configure' to setup libraries")
endif
endif

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

APATH=$(PREFIX)/bin/ate_sources
PPATH=$(PREFIX)/bin/pwb_sources

# Declare non-filename targets
.PHONY: all install uninstall clean help

all: $(TARGET)

$(TARGET) : $(MODULES)
	$(CC) $(LFLAGS) -o $@ $(MODULES) $(LIB_MODULES) $(LDFLAGS)

*.c: $(HEADERS)
	@echo "Forcing full recompile after any header file change"
	touch *.c

%o: %c
	$(CC) $(CFLAGS) -c -o $@ $<

install:
	install -D --mode=775 $(TARGET) -t $(PREFIX)/lib/bash
	mkdir --mode=755 -p $(PREFIX)/share/man/man1
	mkdir --mode=755 -p $(PREFIX)/share/man/man7
	soelim -r $(TARGET_ROOT).1 | gzip -c - > $(PREFIX)/share/man/man1/$(TARGET_ROOT).1.gz
	soelim -r $(TARGET_ROOT).7 | gzip -c - > $(PREFIX)/share/man/man7/$(TARGET_ROOT).7.gz
# install SOURCER and sources
	rm -f $(PREFIX)/bin/$(SOURCER)
	sed -e s^#PREFIX#^$(PREFIX)^ -e s^#BUILTIN#^$(BUILTIN)^ $(SOURCER) > $(PREFIX)/bin/$(SOURCER)
	chmod a+x $(PREFIX)/bin/$(SOURCER)
# If 'ate' installed, Change link so pwb_sources is used for ate_sources
	@if [ -f $(PREFIX)/bin/ate_sources ]; then \
	   echo "Replacing ate_sources link with line to pwb_sources."; \
	   cp -fs $(PREFIX)/bin/$(SOURCER) $(PREFIX)/bin/ate_sources; \
	fi
	install -D $(BUILTIN)_sources.d/$(BUILTIN)_* -t$(PREFIX)/lib/$(BUILTIN)_sources

uninstall:
	rm -f $(PREFIX)/lib/bash/$(TARGET)
	rm -f $(PREFIX)/share/man/man1/$(TARGET_ROOT).1.gz
	rm -f $(PREFIX)/share/man/man7/$(TARGET_ROOT).7.gz
# uninstall SOURCER stuff:
	rm -rf $(PREFIX)/lib/$(BUILTIN)_sources
	rm -f $(PREFIX)/bin/$(SOURCER)
# If ate still installed, update link to point to original ate_sources_impl
	@if [ -f $(PREFIX)/bin/ate_sources_impl ]; then \
	   echo "Restoring orignal link to ate_sources."; \
		cp -fs $(PREFIX)/bin/ate_sources_impl $(PREFIX)/bin/ate_sources; \
	fi

clean:
	rm -f $(TARGET)
	rm -f $(MODULES)

help:
	@echo "Makefile options:"
	@echo
	@echo "  install    to install project"
	@echo "  uninstall  to uninstall project"
	@echo "  clean      to remove generated files"
	@echo "  help       this display"
