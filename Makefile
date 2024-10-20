# Program details
NAME=pong

# Paths to TMS9900 compilation tools
BASE_PATH=../../toolchain/
AS=$(BASE_PATH)tms9900/bin/tms9900-as
LD=$(BASE_PATH)tms9900/bin/tms9900-ld
CC=$(BASE_PATH)tms9900/bin/tms9900-gcc
XGA=$(BASE_PATH)xdt99/xga99.py

# includes and libraries
INCLUDE_PATH=.																					# The path to libti99 header files
LIB_PATH=persistent
LIBRARIES=																							\

# List of compiled objects used in executable
# All resources and source files get added automatically. Do not fudge around with this
# To add .c or .asm files, just add them in one of the bank<?> directories
# To add source code banks, just create a new dir with the name bank<?>
# To add resources, just drop a binary file in the folder "resources" and give
# it a .dat extension. You can then find it in the ROM image at the memory location
# <filename>_bank, <filename>_offset with a length of <filename>_length
# Extra banks for data files are created as needed.
OBJECT_LIST=                                            \
  $(patsubst %.c,%.o,$(wildcard src/*.c))               \
  $(patsubst %.asm,%.o,$(wildcard src/*.asm))

# List of all files needed in executable
PREREQUISITES= $(OBJECT_LIST)

# Compiler flags
CCFLAGS= $(LIBRARIES) -std=c99 -Werror -Wall -O1 -s -fno-function-cse -Iinclude -I$(INCLUDE_PATH) -c

# Linker flags for flat cart binary, most of this is defined in the linker script
LDFLAGS= -L$(LIB_PATH) --no-check-sections 

# Recipe to compile the executable
all: check_dependencies $(PREREQUISITES)
	@echo
	@echo "\t[LD] $(NAME)c.bin"
	@$(LD) --script cart.ld $(LDFLAGS) $(OBJECT_LIST) $(LIBRARIES) -o $(NAME)c.bin -M > link.map
	@echo "\t[ZP] $(NAME).rpk"
	@zip $(NAME).rpk layout.xml $(NAME)c.bin $(NAME)g.bin >> /dev/null
	@./mem_usage.sh $(NAME).bin
	@echo

# Check if dependencies can be found
check_dependencies:
ifeq (,$(wildcard $(CC)))
	@echo "ERROR - tms9900-gcc not found!"
	@echo
	@echo "Please make sure you've installed the gcc tms9900 cross compiler."
	@echo "Download and install the compiler from here: https://github.com/mburkley/tms9900-gcc"
	@echo "and make sure the toolchain path at the top of this Makefile is defined correctly\n\t (\$$BASE_PATH=$(BASE_PATH))\n\t (\$$CC=$(CC))"
	@echo
	@exit 1
endif

# Recipe to clean all compiled objects
.phony clean:
	@echo "\tremoving compiled and intermediary files..."
	@rm -f src/*.o
	@rm -f *.map
	@rm -f $(NAME).rpk
	@rm -f $(NAME)c.bin
	@rm -f $(NAME)g.bin
	@echo

# Recipe to convert binary files to elf objects
# for inclusion by the linker
%.o: %.dat
	@echo "\t[LD] $<..."
	@$(LD) -r -b binary $< -o $@

# Recipe to compile all assembly files
%.o: %.asm
	@echo "\t[AS] $<..."
	@$(AS) $< -o $@

# Recipe to compile all C files
%.o: %.c
	@echo "\t[CC] $<..."
	@$(CC) $(CCFLAGS) $< -o $@
