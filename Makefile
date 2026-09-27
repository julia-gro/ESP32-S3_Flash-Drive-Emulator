BUILD_DIR=build
CC = gcc
CFLAGS = -Wall -g

.PHONY: all clean


all: $(BUILD_DIR)/extract_mbr $(BUILD_DIR)/mbr_reader

clean:
	rm -r $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/extract_mbr: | $(BUILD_DIR)
	$(CC) $(CFLAGS) mbr/extract_mbr.c -o $@

$(BUILD_DIR)/mbr_reader: | $(BUILD_DIR)
	$(CC) $(CFLAGS) mbr/mbr_reader.c -o $@
