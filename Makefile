# Make sure that make is operating with a bash shell
SHELL		:= bash

# Product name and library definitions
SPIKE		:= main
LIBRARY		:= liborbit
TESTS		:= tests-vm
COMPILER	:= orbitc
RUNTIME		:= orbit
STATIC_LIB	:= $(LIBRARY).a
DYNAMIC_LIB := $(LIBRARY).dylib

TOOLS_DIR	:= /usr/bin
PREFIX		:= 

# Toolchain, change this for other platforms
CXX			:= $(PREFIX)gcc
AR			:= $(PREFIX)ar
RANLIB		:= $(PREFIX)ranlib

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
CFLAGS		:= -std=c11 -Wall $(addprefix -I,$(HEADERS_DIR))
LDFLAGS		:= -L$(LIB_OUT)

# Object and headers lists
SOURCES_COM := $(wildcard $(SOURCE_DIR)/compiler/*.c)
SOURCES_RT  := $(wildcard $(SOURCE_DIR)/runtime/*.c)
SOURCES_LIB := $(wildcard $(SOURCE_DIR)/orbit/*.c)
SOURCES_TEST:= $(wildcard $(SOURCE_DIR)/tests-vm/*.c)

OBJECTS_COM := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES_COM))
OBJECTS_RT  := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES_RT))
OBJECTS_LIB := $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES_LIB))
OBJECTS_TEST:= $(patsubst $(SOURCE_DIR)/%.c, $(OBJECTS_OUT)/%.o, $(SOURCES_TEST))

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG -ggdb -g -O0
else
CFLAGS += -DNDEBUG -O3 -flto
LDFLAGS += -flto
endif

.PHONY: clean install spike tests

# Build everything
all: $(STATIC_LIB) $(DYNAMIC_LIB)

clean:
	@rm -rf $(BUILD_DIR)

spike: CFLAGS += -I$(SOURCE_DIR)
spike: $(SPIKES_DIR)/$(SPIKE).c $(STATIC_LIB) 
	@mkdir -p $(SPIKES_OUT)
	@echo "compiling spike $(SPIKE)"
	@$(CXX) $(SPIKES_DIR)/$(SPIKE).c $(LIB_OUT)/$(STATIC_LIB) $(CFLAGS) $(ARCHS) $(LDFLAGS) -o $(SPIKES_OUT)/$(SPIKE)
	@echo "running spike $(SPIKE)"
	@orbitasm -c -o $(SPIKES_DIR)/demo.omf $(SPIKES_DIR)/demo.il
	@$(SPIKES_OUT)/$(SPIKE) $(SPIKES_DIR)/demo

spike-%:
	make spike SPIKE=%

install: $(STATIC_LIB) $(DYNAMIC_LIB)
	@echo "installing headers"
	@cp -R $(HEADERS_DIR)/$(PRODUCT) $(INC_INSTALL)/$(PRODUCT)
	@echo "installing libraries"
	@cp -R $(LIB_OUT)/$(STATIC_LIB) $(LIB_INSTALL)/$(STATIC_LIB)
	@cp -R $(LIB_OUT)/$(DYNAMIC_LIB) $(LIB_INSTALL)/$(DYNAMIC_LIB)

compiler: CFLAGS += -I$(SOURCE_DIR)
compiler: $(OBJECTS_COM) $(STATIC_LIB)
	@mkdir -p $(PRODUCT_OUT)
	@echo "linking $(COMPILER)"
	@$(CXX) $(OBJECTS_COM) $(LIB_OUT)/$(STATIC_LIB) $(LDFLAGS) -o $(PRODUCT_OUT)/$(COMPILER)

runtime: $(OBJECTS_RT) $(STATIC_LIB)
	@mkdir -p $(PRODUCT_OUT)
	@echo "linking $(RUNTIME)"
	@$(CXX) $(OBJECTS_RT) $(LIB_OUT)/$(STATIC_LIB) $(LDFLAGS) -o $(PRODUCT_OUT)/$(RUNTIME) 

$(STATIC_LIB): $(OBJECTS_LIB)
	@mkdir -p $(LIB_OUT)
	@echo "linking static library $@"
	@rm -rf $(LIB_OUT)/$@
	@$(AR) rc $(LIB_OUT)/$@ $(OBJECTS_LIB)
	@$(RANLIB) $(LIB_OUT)/$@

$(DYNAMIC_LIB): $(OBJECTS_LIB)
	@mkdir -p $(LIB_OUT)
	@echo "linking dynamic library $@"
	@$(CXX) -dynamiclib $(LDFLAGS) $(ARCHS) -o $(LIB_OUT)/$@ $(OBJECTS_LIB)

tests: CFLAGS += -I$(SOURCE_DIR)
tests: $(STATIC_LIB) $(OBJECTS_TEST)
	@mkdir -p $(TESTS_OUT)
	@echo "linking tests"
	@$(CXX) $(OBJECTS_TEST) $(LIB_OUT)/$(STATIC_LIB)  $(LDFLAGS) -o $(TESTS_OUT)/$(TESTS)
	@echo "running tests"
	@$(TESTS_OUT)/$(TESTS)

$(OBJECTS_OUT)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "compiling $(notdir $<)"
	@$(CXX) $(CFLAGS) $(ARCHS) -c $< -o $@
