CXX=g++
CC=gcc

# Name of the shared library
TARGET_LIB := microworm.so

# Directories
BUILD_DIR := ./bin
SRC_DIRS := ./src/main

# Source files
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Include directories
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Compiler flags
CPPFLAGS ?= $(INC_FLAGS) -g -MMD -MP
CFLAGS += -fPIC  # Ensure position-independent code for shared libraries
CXXFLAGS += -fPIC

LDFLAGS ?=
LDLIBS ?= -lpthread           # Libraries for linking

# Rule to build the shared library
$(BUILD_DIR)/$(TARGET_LIB): $(OBJS)
	$(CXX) -shared $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# Assembly files
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# C source files
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# C++ source files
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@ 

.PHONY: clean

# Default target
all: $(BUILD_DIR)/$(TARGET_LIB)

# Clean target
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
