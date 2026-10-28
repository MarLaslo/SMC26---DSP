# Project Name
TARGET = main

# Sources
CPP_SOURCES = main.cpp

# Library Locations
LIBDAISY_DIR = libs/libDaisy
DAISYSP_DIR  = libs/DaisySP

# Makes the LGPL-licensed DaisySP modules (ReverbSc, MoogLadder, ...) available.
# Nothing in dsp/ uses them, so they only end up in your program if you use one.
USE_DAISYSP_LGPL = 1

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
