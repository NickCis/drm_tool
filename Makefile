CC=gcc
LD=gcc
RM=rm
INSTALL=install

OUTPUT=drm_tool
OUTPUT_PATH=.
INSTALL_PATH=/usr/bin/
OBJECTS=$(SRC_PATH)/drm_tool.o

SRC_PATH=src

CFLAGS=-Wall -D_FILE_OFFSET_BITS=64 `pkg-config --cflags libdrm` 
LDFLAGS=`pkg-config --libs libdrm`

.PHONY: all

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS) $(OUTPUT_PATH)
	$(CC) $(OBJECTS) -o "$(OUTPUT_PATH)/$@" $(LDFLAGS)

clean:
	- $(RM) $(OUTPUT) $(OBJECTS) *~ $(SRC_PATH)/*~

install: $(OUTPUT)
	$(INSTALL) $(OUTPUT) $(INSTALL_PATH)

%.o: %.c
	$(CC) -c $(CFLAGS) -o "$@" "$<"
