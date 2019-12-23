VERSION_SIZE=64
VERSION_ADDR=0xFFC0
CC=sdcc
CFLAGS=-c -mstm8 -DSTM8S103 -DVERSION_SIZE=$(VERSION_SIZE) -DVERSION_ADDR=$(VERSION_ADDR) -I inc --debug --opt-code-size
LDFLAGS=-mstm8 -L src stm8s_minilib.lib
SOURCES=switch.c src/stm8s_tim2.c 
#src/stm8s_flash.c src/stm8s_adc1.c
# rx_ringbuffer.c statistics.c eeprom.c vs1053.c uid.c cmd.c 
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_LINK=$(SOURCES:.c=.rel)
EXECUTABLE=out.ihx
EXECUTABLE_HEX=out.hex
EXECUTABLE_BIN=out.bin
HEX_N_PATCH=test.hex
BIN_N_PATCH=test.bin
VERSION_INFO=version.txt

#all: stm8minilib $(SOURCES) $(EXECUTABLE)
all: $(SOURCES) $(EXECUTABLE)
install: all
	/home/cleverfox/bin/stm8flash -c stlinkv2 -p stm8s103 -w out.bin
	echo sudo ~/bin/stm8flash -c stlinkv2 -p stm8s103 -w out.bin
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS_LINK) -o $@
	packihx $(EXECUTABLE) > $(EXECUTABLE_HEX)
	objcopy -I ihex -O binary $(EXECUTABLE_HEX) $(EXECUTABLE_BIN)

stm8minilib:
	make -C src

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm *.rst
	rm *.rel
	rm *.lst
	rm *.ihx
	rm *.sym
	rm *.asm
	rm *.lk
	rm *.map
	rm $(EXECUTABLE)
	make -C src clean

