PROJECT_NAME:=application

INCLUDES=mock_avr/avr_includes
CFLAGS:= -Os -Wall -std=gnu11
LDFLAGS=
EXECUTABLE= $(PROJECT_NAME)

SRC:= main.c
SRC+= mock_avr/mock_rtc.c
SRC+= ../../firmware/utils/rtc.c


################################################################################

.PHONY: all
all: $(EXECUTABLE)

.PHONY: run
run: $(EXECUTABLE)
	./$<


################################# AVR Compile ##################################

OBJECTS= $(addsuffix .test.o,$(basename $(SRC)))
DEPEND= $(OBJECTS:.o=.d)

defs:
	echo | $(CC) -dM -E -

# Generate Dependencies
%.test.d: %.c
	@$(CC) -MM -MT $(@:.d=.o) -MT $@ $(addprefix -I,$(INCLUDES)) $(CFLAGS) $< >$@

# C Compile
%.test.o: %.c
	@echo "CC: $< --> $@"
	@$(CC) $(addprefix -I,$(INCLUDES)) $(CFLAGS) -c -o $@ $<

# Link
$(EXECUTABLE): $(OBJECTS)
	@echo "LN: $^ --> $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
  -include $(DEPEND)
endif

.PHONY: clean
clean:
	@rm -rf $(OBJECTS) $(DEPEND) $(EXECUTABLE)
