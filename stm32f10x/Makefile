
OBJFILES = core_cm3.o stm32f10x_dac.o stm32f10x_i2c.o stm32f10x_tim.o \
           misc.o stm32f10x_dbgmcu.o stm32f10x_iwdg.o stm32f10x_usart.o \
           stm32f10x_adc.o stm32f10x_dma.o stm32f10x_pwr.o stm32f10x_wwdg.o \
           stm32f10x_bkp.o stm32f10x_exti.o stm32f10x_rcc.o system_stm32f10x.o \
           stm32f10x_can.o stm32f10x_flash.o stm32f10x_rtc.o \
           stm32f10x_cec.o stm32f10x_fsmc.o stm32f10x_sdio.o \
           stm32f10x_crc.o stm32f10x_gpio.o stm32f10x_spi.o

CFLAGS = -ffunction-sections -fdata-sections -DSTM32F10X_XL -O2 -ggdb -I./include/ -mcpu=cortex-m3 -mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float -mfix-cortex-m3-ldrd

all:	$(OBJFILES)
	arm-none-eabi-ar rcs libstm32f10x.a $(OBJFILES)

%.o:	%.c
	arm-none-eabi-gcc -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJFILES) libstm32f10x.a
