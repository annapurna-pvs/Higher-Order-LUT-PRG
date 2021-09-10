# Set project directory one level above of Makefile directory. $(CURDIR) is a GNU make variable containing the path to the current working directory
PROJDIR := $(realpath $(CURDIR))
SOURCEDIR := $(PROJDIR)/
BUILDDIR := $(PROJDIR)/Build

# Name of the final executable
TARGET = higherPRG.exe

# Create the list of directories
SOURCEDIRS = . AES PRESENT Util

# Generate the GCC includes parameters by adding -I before each source folder
INCLUDES = $(foreach dir, $(SOURCEDIRS), $(addprefix -I, $(dir)))

# Add this list to VPATH, the place make will look for the source files
VPATH = $(SOURCEDIRS)

# Create a list of *.c sources in SOURCEDIRS
SOURCES = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.c))

# Define objects for all sources
OBJS := $(SOURCES:.c=.o)
BLD_OBJS := $(foreach file,$(SOURCES),Build/$(file))
ALLOBJS := $(BLD_OBJS:.c=.o)

# Define dependencies files for all objects
# DEPS = $(OBJS:.o=.d)

# Name the compiler
CC = gcc

# Tools
ifeq ($(OS),Windows_NT)
	MKDIR = -mkdir
	RM = del /F /Q
	SEP = /
	ERRIGNORE = 2>/dev/null
else
	MKDIR = mkdir -p
	RM = rm -rf
	SEP = /
	ERRIGNORE = 2>/dev/null
endif

# Remove space after separator
PSEP = $(strip $(SEP))

# Define the function that will generate each rule
define generateObjs
$(1)/%.o: %.c
	@echo Building $$@ $(1)
	$(CC) -c $$(INCLUDES) -o Build/$$@ $$< 
#	$(CC) -c $$(INCLUDES) -o $$(subst /,$$(PSEP),$$@) $$(subst /,$$(PSEP),$$<) -MMD
endef

.PHONY: all clean directories 

all: directories $(TARGET)

$(TARGET): $(OBJS)
	echo Linking $@
	$(CC) $(ALLOBJS) -o Build/$(TARGET)

# Include dependencies
# -include $(DEPS)

# Generate rules
$(foreach targetdir, $(SOURCEDIRS), $(eval $(call generateObjs, $(targetdir))))

directories:
	-@$(MKDIR) $(BUILDDIR) $(BUILDDIR)/AES $(BUILDDIR)/PRESENT $(BUILDDIR)/Util

# Remove all objects, dependencies and executable files generated during the build
clean:
	@$(RM) Build
	@echo Cleaning done ! 

