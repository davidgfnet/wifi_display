
OBJFILES = startup.o main.o except.o gde043a2.o sram.o uart.o imgdec.o

CFLAGS = -ffunction-sections -fdata-sections -DSTM32F10X_XL -O2 -ggdb -I../stm32f10x/include/ -mcpu=cortex-m3 -mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float -mfix-cortex-m3-ldrd -I./images

all:	$(OBJFILES)
	arm-none-eabi-gcc -o firmware.elf $(OBJFILES) -mthumb -T link.ld -L../stm32f10x -lstm32f10x -static -nostartfiles -Wl,--gc-sections -Wl,-Map=firmware.map
	arm-none-eabi-objcopy -O binary firmware.elf firmware.bin 

%.o:	%.c
	arm-none-eabi-gcc -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJFILES) firmware.elf firmware.bin
