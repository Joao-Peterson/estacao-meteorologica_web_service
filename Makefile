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
L_FLAGS += ./lib/libdoc.a
L_FLAGS += ./lib/libcmdf.a
L_FLAGS += ./lib/libmicrohttpd.dll.a
L_FLAGS += ./lib/libcurl.dll.a
L_FLAGS += ./lib/libmysql.lib
L_FLAGS += -lws2_32

SOURCES := main.c
MAIN_APP := main.exe
BUILD_DIR := build/
RES_DIR := res/

SQL_DIR := sql/

RES_CC := windres
RES_C_FLAGS := -O coff
RES_SCRIPT := $(RES_DIR)resources.rc
RES_OUT = $(RES_DIR)resources.res

# ---------------------------------------------------------------

OBJS := $(SOURCES:.c=.o)

RES_FILES :=
RES_FILES += $(wildcard $(SQL_DIR)*.sql)

OBJS_BUILD := $(addprefix $(BUILD_DIR), $(notdir $(SOURCES:.c=.o)))

# ---------------------------------------------------------------

.PHONY : build

build : C_FLAGS += -g
build : $(MAIN_APP)

release : C_FLAGS += -O3
release : $(MAIN_APP)

$(MAIN_APP) : $(OBJS) $(RES_OUT)
	$(CC) $(OBJS_BUILD) $(RES_OUT) $(L_FLAGS) -o $@

%.o : %.c
	$(CC) $(C_FLAGS) $(I_FLAGS) -c $< -o $(addprefix $(BUILD_DIR), $(notdir $@))

$(RES_OUT) : $(RES_SCRIPT) $(RES_FILES)
	$(RES_CC) $< $(RES_C_FLAGS) -o $@ 

clear : 
	@rm $(MAIN_APP)
	@rm -f $(OBJS_BUILD)
	@rm -f $(RES_OUT)
