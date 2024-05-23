########################################################################
# LIBRARY SOURCES - MUST BE IN THE SAME FOLDER as main.c (DO NOT CHANGE)
########################################################################

# Set mylib folder path.
# Do not change the MYLIB_PATH variable name.
# ONLY use relative file paths that start with $(SOURCELIB_ROOT)../
# DO NOT USE absolute file paths (e.g. /home/users/myuser/mydir)
# e.g. MYLIBPATH=$(SOURCELIB_ROOT)/../mylib
MYLIB_PATH=$(SOURCELIB_ROOT)/../repo/mylib

# Set folder path with header files to include.
# ONLY use relative file paths that start with $(SOURCELIB_ROOT)../
# DO NOT USE absolute file paths (e.g. /home/users/myuser/mydir)
CFLAGS += -I$(MYLIB_PATH)

# List all c files locations that must be included (use space as separate e.g. LIBSRCS += path_to/file1.c path_to/file2.c)
# ONLY use relative file paths that start with $(SOURCELIB_ROOT)../
# DO NOT USE absolute file paths (e.g. /home/users/myuser/mydir)
LIBSRCS += $(MYLIB_PATH)/s4375116_lta1000g.c
LIBSRCS += $(MYLIB_PATH)/s4375116_joystick.c
LIBSRCS += $(MYLIB_PATH)/s4375116_pantilt.c
LIBSRCS += $(MYLIB_PATH)/s4375116_hamming.c
LIBSRCS += $(MYLIB_PATH)/s4375116_irremote.c
LIBSRCS += $(MYLIB_PATH)/s4375116_oled.c
LIBSRCS += $(MYLIB_PATH)/s4375116_CAG_display.c
LIBSRCS += $(MYLIB_PATH)/s4375116_CAG_simulator.c
LIBSRCS += $(MYLIB_PATH)/s4375116_CAG_grid.c
LIBSRCS += $(MYLIB_PATH)/s4375116_CAG_joystick.c
LIBSRCS += $(MYLIB_PATH)/s4375116_cli_mnemonic.c
LIBSRCS += $(MYLIB_PATH)/s4375116_cli_task.c
LIBSRCS += $(MYLIB_PATH)/s4375116_CAG_mnemonic.c


# Including memory heap model
LIBSRCS += $(FREERTOS_PATH)/portable/MemMang/heap_4.c
