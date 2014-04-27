EXECUTABLE=lish

SOURCE_DIR=src
OBJECT_DIR=bin

LISH_MAJOR=0
LISH_MINOR=2
LISH_PATCH=0
LISH_VERSION=v$(LISH_MAJOR).$(LISH_MINOR).$(LISH_PATCH)

CC=clang
CDEFINES=-DLISH_VERSION=\"$(LISH_VERSION)\" -DLISH_MAJOR=\"$(LISH_MAJOR)\" -DLISH_MINOR=\"$(LISH_MINOR)\"
CFLAGS=-c -std=c11 -Wall $(CDEFINES)
LDFLAGS=-lm

SOURCES=$(wildcard $(SOURCE_DIR)/*.c)
OBJECTS=$(SOURCES:$(SOURCE_DIR)/%.c=$(OBJECT_DIR)/%.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECT_DIR)/*.o
	rm -f $(EXECUTABLE)

install: all
	cp $(EXECUTABLE) /usr/local/bin
	mkdir -p /usr/local/lib/lish/$(LISH_MAJOR).$(LISH_MINOR)/
	cp -r lib /usr/local/lib/lish/$(LISH_MAJOR).$(LISH_MINOR)/

