ifeq ($(origin AVR_DX_DFP),undefined)
    $(error Environment variable AVR_DX_DFP is not set. See README)
endif
################################ Project Setup #################################

CC:=avr-gcc
OBJCOPY:=avr-objcopy
OBJDUMP:=avr-objdump

PROJECT_NAME:=application

MCU=avr128da32
INCLUDES= $(AVR_DX_DFP)/include
CFLAGS:= -Os -Wall -std=gnu11 -ffunction-sections -fdata-sections -g
CFLAGS+= -funsigned-char -funsigned-bitfields -fpack-struct
LDFLAGS= -Wl,--gc-sections -g -Wl,-Map=$(EXECUTABLE).map
EXECUTABLE= $(PROJECT_NAME).elf

SRC:= main.c
SRC+= sys.c
SRC+= display/display.c
SRC+= display/display_spi.c
SRC+= display/display_pwm.c
SRC+= display/segment_codes.c
SRC+= utils/uart.c
SRC+= utils/fifo.c
SRC+= utils/event_queue.c
SRC+= utils/cli.c
SRC+= utils/string_ext.c
SRC+= utils/i2c.c
SRC+= utils/rtc.c
SRC+= sensors/sensors.c
SRC+= sensors/ms5611_pressure.c
SRC+= sensors/ltr-329als_light.c
SRC+= cli_commands.c
SRC+= usb_uart.c

################################################################################

.PHONY: all
all:$(EXECUTABLE)

.PHONY: disassemble
disassemble: $(EXECUTABLE)
	$(OBJDUMP) -d -S $< > disassembly.S


################################# AVR Compile ##################################

OBJECTS= $(addsuffix .o,$(basename $(SRC)))
DEPEND= $(OBJECTS:.o=.d)
MCUFLAGS= -mmcu=$(MCU) -B $(AVR_DX_DFP)/gcc/dev/$(MCU)

defs:
	echo | $(CC) -dM -E $(MCUFLAGS) -

# Generate Dependencies
%.d: %.c
	@$(CC) -MM -MT $(@:.d=.o) -MT $@ $(MCUFLAGS) $(addprefix -I,$(INCLUDES)) $(CFLAGS) $< >$@

# C Compile
%.o: %.c
	$(CC) $(MCUFLAGS) $(addprefix -I,$(INCLUDES)) $(CFLAGS) -c -o $@ $<

# Link
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(MCUFLAGS) -o $@ $^ $(LDFLAGS)
	@avr-size $@

ifneq ($(MAKECMDGOALS), clean)
  -include $(DEPEND)
endif

.PHONY: clean
clean:
	@rm -rf $(OBJECTS) $(DEPEND) $(EXECUTABLE) $(EXECUTABLE).map
