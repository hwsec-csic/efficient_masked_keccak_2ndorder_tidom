# ===============================
# Target
# ===============================

TARGET = mykeccak

# ===============================
# Sources
# ===============================

SRC += mykeccak.c

# Assembly sources
ASRC += keccak_f1600.S

# ===============================
# Toolchain
# ===============================

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# ===============================
# Flags
# ===============================

CFLAGS += \
	-mcpu=cortex-m3 \
	-mthumb \
	-g \
	-O2 \
	-ffreestanding \
	-fno-exceptions

# ===============================
# Includes
# ===============================

CFLAGS += -I./

# ===============================
# VPATH (si usas src folders)
# ===============================

VPATH += ./src

# ===============================
# Objects
# ===============================

OBJDIR = obj

OBJS = $(SRC:.c=.o)
OBJS += $(ASRC:.S=.o)

OBJS := $(addprefix $(OBJDIR)/,$(notdir $(OBJS)))

# ===============================
# Rules
# ===============================

all: $(TARGET).bin

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link final ELF
$(TARGET).elf: $(OBJDIR) $(OBJS)
	$(CC) $(CFLAGS) -nostartfiles -Wl,-e,main -o $@ $(OBJS)

# Generate binary
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Compile C
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile ASM
$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ===============================
# Clean
# ===============================

clean:
	rm -rf $(OBJDIR) *.elf *.bin *.lst

.PHONY: all clean