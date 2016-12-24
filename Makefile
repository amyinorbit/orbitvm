# Make sure that make is operating with a bash shell
SHELL		:= bash

# Product name and library definitions
PRODUCT		:= tests-vm
LIBRARY		:= liborbit
COMPILER	:= orbitc
STATIC_LIB	:= $(LIBRARY).a
DYNAMIC_LIB := $(LIBRARY).dylib

# Toolchain, change this for other platforms
CXX			:= clang
AR			:= ar
STRIP		:= strip

# Not the best, change your install dir to where you want it installed
LIB_INSTALL := /Users/cesar/toolchains/lib/osx
INC_INSTALL := /Users/cesar/toolchains/include

# Working directories for compilation
SOURCE_DIR	:= ./src
HEADERS_DIR := ./include
BUILD_DIR	:= ./build
OBJECTS_DIR := $(BUILD_DIR)/intermediate
PRODUCT_DIR := $(BUILD_DIR)/product
LIB_DIR		:= $(PRODUCT_DIR)/lib

# Compiler and linker flags
LIBS		:= orbit
ARCHS		:= -arch x86_64 -arch i386
CFLAGS		:= -std=c11 -Wall -Werror $(addprefix -I,$(HEADERS_DIR)) -fPIC
LIBFLAGS	:= -std=c11 -fPIC
LDFLAGS		:= -std=c11 -L$(LIB_DIR) $(addprefix -l,$(LIBS))

# Object and headers lists
SOURCES		:= $(wildcard $(SOURCE_DIR)/$(PRODUCT)/*.c)
SOURCES_LIB := $(wildcard $(SOURCE_DIR)/$(LIBRARY)/*.c)
OBJECTS		:= $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_DIR)/%.o, $(SOURCES))
OBJECTS_LIB := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_DIR)/%.o, $(SOURCES_LIB))


.PHONY: clean install run

# Build everything
all: $(STATIC_LIB) $(DYNAMIC_LIB)

clean:
	@rm -rf $(BUILD_DIR)

run: $(PRODUCT)
	@echo "running executable $<"
	@$(PRODUCT_DIR)/$(PRODUCT)

$(PRODUCT): $(OBJECTS) $(STATIC_LIB)
	@echo "linking executable $@"
	@mkdir -p $(PRODUCT_DIR)
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $(PRODUCT_DIR)/$(PRODUCT)

install: $(STATIC_LIB) $(DYNAMIC_LIB)
	@echo "installing headers"
	@cp -R $(HEADERS_DIR)/$(PRODUCT) $(INC_INSTALL)/$(PRODUCT)
	@echo "installing libraries"
	@cp -R $(LIB_DIR)/$(STATIC_LIB) $(LIB_INSTALL)/$(STATIC_LIB)
	@cp -R $(LIB_DIR)/$(DYNAMIC_LIB) $(LIB_INSTALL)/$(DYNAMIC_LIB)

$(STATIC_LIB): $(OBJECTS_LIB)
	@mkdir -p $(LIB_DIR)
	@echo "linking static library $@"
	@rm -rf $(LIB_DIR)/$@
	@$(AR) rc $(LIB_DIR)/$@ $(OBJECTS_LIB)
	@ranlib $(LIB_DIR)/$@

$(DYNAMIC_LIB): $(OBJECTS_LIB)
	@mkdir -p $(LIB_DIR)
	@echo "linking dynamic library $@"
	@$(CXX) -dynamiclib $(LIBFLAGS) $(ARCHS) -o $(LIB_DIR)/$@ $(OBJECTS_LIB)

$(OBJECTS_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "compiling $(notdir $<)"
	@$(CXX) $(CFLAGS) $(ARCHS) -c $< -o $@
