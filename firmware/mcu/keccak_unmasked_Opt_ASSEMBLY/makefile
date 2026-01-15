TARGET = KeccakFW

# List C source files
SRC += KeccakFW.c

VPATH += ../../hal/stm32f3
SRC += stm32f3_hal.c
SRC += stm32f3xx_hal_msp.c
SRC += stm32f3xx_hal_gpio.c
SRC += stm32f3xx_it.c

# List ASM sources
ASRC += Keccak1600NoOpt.S

# Include paths
CFLAGS += -I./../../hal/stm32f3

# ---------------------------------------------------------------------------

ifeq ($(CRYPTO_OPTIONS),)
CRYPTO_OPTIONS = AES128C
endif

# Add simpleserial project
include ../simpleserial/Makefile.simpleserial

# Firmware include
FIRMWAREPATH = ../.
include $(FIRMWAREPATH)/Makefile.inc

# ---------------------------------------------------------------------------
# Opcional: regla personalizada si tu ASM no termina en mayúscula .S
# (Makefile.inc espera .S, así que si tu archivo es .s, renómbralo o compila explícitamente)
$(OBJDIR)/%.o : %.s $(OBJDIR)
	@echo Assembling: $<
	arm-none-eabi-gcc -c -mcpu=cortex-m4 -mthumb -I. $< -o $@
