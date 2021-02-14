# ---------------------------------------------------------------
# C:\msys64\usr\include
# C:\msys64\usr\lib
# C:\Program Files\MySQL\MySQL Server 8.0\include
# C:\Program Files\MySQL\MySQL Server 8.0\lib
# https://www.rapidtables.com/code/linux/gcc/gcc-l.html <- how to link libs

CC := gcc

C_FLAGS :=

I_FLAGS :=
I_FLAGS += -I./inc

L_FLAGS :=
L_FLAGS += -L./lib
# L_FLAGS += -static
L_FLAGS += -lcurl
L_FLAGS += ./lib/libdoc.a
# L_FLAGS += -lbrotlidec-static 

SOURCES := main.c
MAIN_APP := main.exe
BUILD_DIR := build/

# ---------------------------------------------------------------

OBJS := $(SOURCES:.c=.o)

OBJS_BUILD := $(addprefix $(BUILD_DIR), $(notdir $(SOURCES:.c=.o)))

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += -g
build : $(MAIN_APP)

release : C_FLAGS += -Ofast
release : $(MAIN_APP)

$(MAIN_APP) : $(OBJS)
	$(CC) $(OBJS_BUILD) $(L_FLAGS) -o $@

%.o : %.c
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $(addprefix $(BUILD_DIR), $(notdir $@))

clear : 
	@rm $(MAIN_APP)
	@rm -f $(OBJS_BUILD)
