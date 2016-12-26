# Make sure that make is operating with a bash shell
SHELL		:= bash

# Product name and library definitions
#PRODUCT		:= tests-vm
SPIKE		:= main
LIBRARY		:= liborbit
TESTS		:= tests-vm
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
SPIKES_DIR	:= ./spikes
HEADERS_DIR := ./include
BUILD_DIR	:= ./build

OBJECTS_OUT := $(BUILD_DIR)/intermediate
PRODUCT_OUT := $(BUILD_DIR)/product
SPIKES_OUT	:= $(PRODUCT_OUT)/spikes
LIB_OUT		:= $(PRODUCT_OUT)/lib
TESTS_OUT	:= $(PRODUCT_OUT)/tests

# Compiler and linker flags
LIBS		:= orbit
ARCHS		:= -arch x86_64 -arch i386
CFLAGS		:= -std=c11 -Wall -Werror $(addprefix -I,$(HEADERS_DIR))
LIBFLAGS	:= -L$(LIB_OUT) $(addprefix -l,$(LIBS))
LDFLAGS		:= 

# Object and headers lists
SOURCES		:= $(wildcard $(SOURCE_DIR)/$(PRODUCT)/*.c)
SOURCES_LIB := $(wildcard $(SOURCE_DIR)/$(LIBRARY)/*.c)
SOURCES_TEST:= $(wildcard $(SOURCE_DIR)/$(TESTS)/*.c)

OBJECTS		:= $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES))
OBJECTS_LIB := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES_LIB))
OBJECTS_TEST:= $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES_TEST))


.PHONY: clean install spike tests

# Build everything
all: $(STATIC_LIB) $(DYNAMIC_LIB)

clean:
	@rm -rf $(BUILD_DIR)

$(PRODUCT): LDFLAGS += $(LIBFLAGS)
$(PRODUCT): $(OBJECTS) $(STATIC_LIB)
	@echo "linking executable $@"
	@mkdir -p $(PRODUCT_OUT)
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $(PRODUCT_OUT)/$(PRODUCT)

spike: CFLAGS += -I$(SOURCE_DIR)
spike: LDFLAGS += $(LIBFLAGS)
spike: $(SPIKES_DIR)/$(SPIKE).c $(STATIC_LIB) 
	@mkdir -p $(SPIKES_OUT)
	@echo "compiling spike $(SPIKE)"
	@$(CXX) $(CFLAGS) $(ARCHS) $(LDFLAGS) $(SPIKES_DIR)/$(SPIKE).c -o $(SPIKES_OUT)/$(SPIKE)
	@echo "running spike $(SPIKE)"
	@$(SPIKES_OUT)/$(SPIKE)

spike-%:
	make spike SPIKE=%

install: $(STATIC_LIB) $(DYNAMIC_LIB)
	@echo "installing headers"
	@cp -R $(HEADERS_DIR)/$(PRODUCT) $(INC_INSTALL)/$(PRODUCT)
	@echo "installing libraries"
	@cp -R $(LIB_OUT)/$(STATIC_LIB) $(LIB_INSTALL)/$(STATIC_LIB)
	@cp -R $(LIB_OUT)/$(DYNAMIC_LIB) $(LIB_INSTALL)/$(DYNAMIC_LIB)

$(STATIC_LIB): $(OBJECTS_LIB)
	@mkdir -p $(LIB_OUT)
	@echo "linking static library $@"
	@rm -rf $(LIB_OUT)/$@
	@$(AR) rc $(LIB_OUT)/$@ $(OBJECTS_LIB)
	@ranlib $(LIB_OUT)/$@

$(DYNAMIC_LIB): $(OBJECTS_LIB)
	@mkdir -p $(LIB_OUT)
	@echo "linking dynamic library $@"
	@$(CXX) -dynamiclib $(LDFLAGS) $(ARCHS) -o $(LIB_OUT)/$@ $(OBJECTS_LIB)

tests: CFLAGS += -I$(SOURCE_DIR)
tests: LDFLAGS += $(LIBFLAGS)
tests: $(STATIC_LIB) $(OBJECTS_TEST)
	@mkdir -p $(TESTS_OUT)
	@echo "linking tests"
	@$(CXX) $(OBJECTS_TEST) $(LDFLAGS) $(LIBFLAGS) -o $(TESTS_OUT)/$(TESTS)
	@echo "running tests"
	@$(TESTS_OUT)/$(TESTS)

$(OBJECTS_OUT)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "compiling $(notdir $<)"
	@$(CXX) $(CFLAGS) $(ARCHS) -c $< -o $@
