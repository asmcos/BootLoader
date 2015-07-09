

CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++
LD=arm-none-eabi-ld
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump
NM=arm-none-eabi-nm
SIZE=arm-none-eabi-size
A2L=arm-none-eabi-addr2line

BINDIR=bin
BINELF=BootLoader.elf
BIN=BootLoader.bin

INCLUDES= \
   -Iinclude \
   -I../mbed/libraries/mbed/targets/cmsis \
   -I../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4 \
   -I../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/TARGET_NUCLEO_F411RE \

ASOURCES= ../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/TARGET_NUCLEO_F411RE/TOOLCHAIN_GCC_ARM/startup_stm32f411xe.S

CSOURCES= src/main.c \
	 src/stm32f4xx_it.c  src/syscalls.c src/stm32f4xx_hal_msp.c src/boot.c\
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal_uart.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal_gpio.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/TARGET_NUCLEO_F411RE/system_stm32f4xx.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal_rcc.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal_cortex.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal_flash.c \
	../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/stm32f4xx_hal_flash_ex.c \



LDFILE= ../mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/TARGET_NUCLEO_F411RE/TOOLCHAIN_GCC_ARM/STM32F411XE.ld
INCLUDES_LIBS=
LINK_LIBS=

OBJECTS=$(ASOURCES:%.S=%.o)
OBJECTS+=$(CSOURCES:%.c=%.o)
OBJECTS+=$(CXXSOURCES:%.cpp=%.o)

CFLAGS=-c -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -DSTM32F411xE  \
    -Os -flto -ffunction-sections -fdata-sections -fno-builtin -fno-exceptions $(INCLUDES) 

CXXFLAGS=-c -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -DSTM32F411xE \
    -Os -flto -ffunction-sections -fdata-sections -fno-builtin -fno-rtti -fno-exceptions $(INCLUDES) -std=c++11

LDFLAGS=-mcpu=cortex-m4 -mlittle-endian -mthumb -DSTM32F411xE  -T$(LDFILE) \
    -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -Wl,--start-group -lgcc -lc -Wl,--end-group

all: $(SOURCES) $(BINDIR)/$(BIN)

flash: $(BINDIR)/$(BIN)
	~/stlink/st-flash --reset write $(BINDIR)/$(BIN) 0x8000000 
   
$(BINDIR)/$(BIN): $(BINDIR)/$(BINELF)
	$(CP) -O binary $< $@
    
$(BINDIR)/$(BINELF): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -Wl,-Map,$(BINDIR)/BootLoader.map -o $@
	@echo "Linking complete!\n"
	$(SIZE) $(BINDIR)/$(BINELF)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

%.o: %.s
	$(CC) $(CFLAGS) $< -o $@
	@echo "Assembled "$<"!\n"

clean:
	rm -f $(OBJECTS) $(BINDIR)/$(BINELF) $(BINDIR)/$(BIN) $(BINDIR)/BootLoader.map
