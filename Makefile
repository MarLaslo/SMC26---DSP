# Project Name
TARGET = main

# Sources
CPP_SOURCES = main.cpp

# Library Locations
LIBDAISY_DIR = libs/libDaisy
DAISYSP_DIR  = libs/DaisySP

# Pulls in the LGPL-licensed DaisySP modules (needed for Fold, used by the wavefolder effect)
USE_DAISYSP_LGPL = 1

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
