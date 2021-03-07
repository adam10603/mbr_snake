# Terrible makefile

SHELL:=/bin/bash

CXX = g++
CXX_FLAGS = -std=c++17 -Os -m16 -march=i386 -ffreestanding -fmerge-constants -c
LD = ld
LD_FLAGS = -static --nmagic -nostdlib -m elf_i386

EXEC = mbr_snake
SOURCE = $(EXEC).cpp
SOURCE_DIR = src
OBJECT = $(EXEC).o
OBJECT_DIR = obj
OUTPUT_DIR = bin
OUTPUT = $(EXEC).bin
OUTPUT_BS = $(EXEC)_bs.bin
OUTPUT_IMG = $(EXEC).img

# Main target
$(EXEC): $(OBJECT)
	@ld $(LD_FLAGS) -Tlinker_nosig.ld -o $(OUTPUT_DIR)/$(EXEC).elf $(OBJECT_DIR)/$(OBJECT)
	@objcopy -O binary $(OUTPUT_DIR)/$(EXEC).elf $(OUTPUT_DIR)/$(OUTPUT)
	@rm $(OUTPUT_DIR)/$(EXEC).elf
	@SNAKE_EXEC_SIZE=$$(stat -L -c %s $(OUTPUT_DIR)/$(OUTPUT)) ; \
	if [ $$SNAKE_EXEC_SIZE -gt 510 ] ; then \
		printf "Binary is too large: $$SNAKE_EXEC_SIZE bytes\n" ; \
		false ; \
	else \
		printf "Binary created:  $(OUTPUT_DIR)/$(OUTPUT)\n" ; \
		printf "Binary size:     $$SNAKE_EXEC_SIZE bytes\n" ; \
	fi

image:
	@if [ -f $(OUTPUT_DIR)/$(OUTPUT) ] ; then \
		SNAKE_EXEC_SIZE=$$(stat -L -c %s $(OUTPUT_DIR)/$(OUTPUT)) ; \
		if [ $$SNAKE_EXEC_SIZE -gt 510 ] ; then \
			printf "Binary is too large: $$SNAKE_EXEC_SIZE bytes\n" ; \
			false ; \
		else \
			(rm -f $(OUTPUT_DIR)/$(OUTPUT_IMG) $(OUTPUT_DIR)/$(OUTPUT_BS) ) && \
			(cp $(OUTPUT_DIR)/$(OUTPUT) $(OUTPUT_DIR)/$(OUTPUT_BS) ) && \
			(sync $(OUTPUT_DIR)/$(OUTPUT_BS) ) && \
			(dd conv=notrunc if=/dev/zero of=$(OUTPUT_DIR)/$(OUTPUT_BS) bs=1 count=$$(( 510 - $$SNAKE_EXEC_SIZE )) seek=$$SNAKE_EXEC_SIZE > /dev/null 2>&1 & ) && \
			(sync $(OUTPUT_DIR)/$(OUTPUT_BS) ) && \
			(printf "\x55\xAA" >> $(OUTPUT_DIR)/$(OUTPUT_BS) ) && \
			(sync $(OUTPUT_DIR)/$(OUTPUT_BS) ) && \
			(printf "Boot sector data created:       $(OUTPUT_DIR)/$(OUTPUT_BS)\n" ) ; \
			(cp $(OUTPUT_DIR)/$(OUTPUT_BS) $(OUTPUT_DIR)/$(OUTPUT_IMG) ) && \
			(sync $(OUTPUT_DIR)/$(OUTPUT_IMG) ) && \
			(dd conv=notrunc if=/dev/zero of=$(OUTPUT_DIR)/$(OUTPUT_IMG) bs=512 count=2879 seek=1 > /dev/null 2>&1 & ) && \
			(sync $(OUTPUT_DIR)/$(OUTPUT_IMG) ) && \
			(printf "Bootable floppy image created:  $(OUTPUT_DIR)/$(OUTPUT_IMG)\n" ) ; \
		fi \
	else \
		printf "The binary file does not exist. Use 'make' to build it first!\n" ; \
		false ; \
	fi

$(OBJECT):
	@$(CXX) $(CXX_FLAGS) -o $(OBJECT_DIR)/$(OBJECT) $(SOURCE_DIR)/$(SOURCE)
	@strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag $(OBJECT_DIR)/$(OBJECT)

clean:
	@rm -f $(OBJECT_DIR)/$(OBJECT) $(OUTPUT_DIR)/$(OUTPUT) $(OUTPUT_DIR)/$(OUTPUT_BS) $(OUTPUT_DIR)/$(OUTPUT_IMG)