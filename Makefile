EXECUTABLE=lish

LISH_MAJOR=0
LISH_MINOR=3
LISH_PATCH=1
LISH_LOCAL_LIBS=1

LISH_VERSION=v$(LISH_MAJOR).$(LISH_MINOR).$(LISH_PATCH)
LISH_INSTALL_PATH=/usr/local/lib/lish/$(LISH_MAJOR).$(LISH_MINOR)/

CDEFINES=-DLISH_VERSION=\"$(LISH_VERSION)\"           \
         -DLISH_LOCAL_LIBS=$(LISH_LOCAL_LIBS)         \
         -DLISH_INSTALL_PATH=\"$(LISH_INSTALL_PATH)\"

CC=cc
CFLAGS=-c -g -ggdb -std=gnu99 -Wall $(CDEFINES)
LDFLAGS=-lm

sourcesubdirs=$(shell find src -type d | grep -v "src$$" | awk '{gsub("src/","bin/",$$1); print $$1}'|xargs)
rwildcard=$(foreach d, $(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES=$(call rwildcard, src/, *.c)
OBJECTS=$(SOURCES:src/%.c=bin/%.o)

.PHONY: all $(EXECUTABLE) build_bin_structure clean install test
all: build_bin_structure $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

bin/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

test:
	@./$(EXECUTABLE) test.lish

build_bin_structure:
	@mkdir -p $(sourcesubdirs)

clean:
	@echo "Cleaning project..."
	@rm -rf bin
	@rm -f $(EXECUTABLE)

install: all
	cp $(EXECUTABLE) /usr/local/bin
	mkdir -p /usr/local/lib/lish/$(LISH_MAJOR).$(LISH_MINOR)/
	cp -r lib /usr/local/lib/lish/$(LISH_MAJOR).$(LISH_MINOR)/

