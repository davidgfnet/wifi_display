CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls -std=c99 -ffunction-sections -fdata-sections -Os -DICACHE_FLASH
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc 
LDFLAGS = -Teagle.app.v6.ld -Wl,--gc-sections -flto  -Wl,-Map,fw.map

firmware-0x00000.bin: firmware
	esptool.py elf2image $^

firmware: main.o uart.o httpd.o
	$(CC) -o firmware main.o uart.o httpd.o $(LDFLAGS) $(LDLIBS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

flash: firmware-0x00000.bin
	esptool.py write_flash 0 firmware-0x00000.bin 0x40000 firmware-0x40000.bin

clean:
	rm -f firmware main.o uart.o firmware-0x00000.bin firmware-0x400000.bin


