CC=sdcc
AR=sdar
CFLAGS=-c -mstm8 -DSTM8S103 -I ../inc --opt-code-size -DNOLONG
LDFLAGS=-rc 
#SOURCES=stm8s_flash.c stm8s_uart2.c stm8s_tim4.c stm8s_clk.c stm8s_itc.c
SOURCES=stm8s_exti.c stm8s_uart1_mini.c stm8s_tim1_mini.c stm8s_clk.c stm8s_gpio.c
#stm8s_spi.c stm8s_flash.c 
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_LINK=$(SOURCES:.c=.rel)
EXECUTABLE=stm8s_minilib.lib

all: $(OBJECTS) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR) $(LDFLAGS) $(EXECUTABLE) $(OBJECTS_LINK) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -f *.lib
	rm -f *.rst
	rm -f *.rel
	rm -f *.lst
	rm -f *.ihx
	rm -f *.sym
	rm -f *.asm
	rm -f *.lk
	rm -f *.map
	rm -f $(EXECUTABLE)
