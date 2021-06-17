#
#    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
#    Sergey Manoylo <info@smansoft.com>
#

.PHONY = all init clean objdir
.DEFAULT_GOAL = all

ROOT=.

CC=gcc
AR=ar

ifeq ($(SM_FILE_BUILD),Release)
	CFLAGS = -O3 -DNDEBUG -DSM_OS_LINUX -DSM_SYNC_LOG -Wall -fmessage-length=0 -std=c11			# release flags
else
	CFLAGS = -O0 -g3 -ggdb  -DDEBUG -DSM_OS_LINUX -DSM_SYNC_LOG -Wall -fmessage-length=0 -std=c11		# debug flags
endif

TARGET = libsstring_lib-0.0.1.1.a

VPATH 	= $(ROOT)/src
OBJDIR 	= $(ROOT)/obj

INCLUDES 	= -I$(ROOT)/include -I$(ROOT)/src

SRCS 		= safe_abort_handler.c \
        safe_memcpy.c \
		safe_memzero.c \
        safe_strcpy.c \
		safe_strisdigit.c \
        safe_strncat.c \
		safe_strtolowercase.c \
        safe_wmemcpy.c \
		safe_fopen.c \
        safe_memmove16.c \
		safe_snprintf_support.c \
        safe_strcpyfld.c \
		safe_strishex.c \
        safe_strncpy.c \
		safe_strtouppercase.c \
        safe_wmemmove.c \
		safe_gets.c \
        safe_memmove32.c \
		safe_stpcpy.c \
        safe_strcpyfldin.c \
		safe_strislowercase.c \
        safe_strnlen.c \
		safe_strzero.c \
        safe_wmemset.c \
		safe_ignore_handler.c \
        safe_memmove.c \
		safe_stpncpy.c \
        safe_strcpyfldout.c \
		safe_strismixedcase.c \
        safe_strnterminate.c \
        safe_wcpcpy.c \
        safe_memcmp16.c \
        safe_mem_primitives_lib.c \
        safe_strcasecmp.c \
        safe_strcspn.c \
        safe_strispassword.c \
        safe_strpbrk.c \
        safe_wcscat.c \
        safe_memcmp32.c \
        safe_memset16.c \
        safe_strcasestr.c \
        safe_strfirstchar.c \
        safe_strisuppercase.c \
        safe_strprefix.c \
        safe_wcscpy.c \
        safe_memcmp.c \
        safe_memset32.c \
        safe_strcat.c \
        safe_strfirstdiff.c \
        safe_strlastchar.c \
        safe_strremovews.c \
        safe_wcsncat.c \
        safe_mem_constraint.c \
        safe_memset.c \
        safe_strcmp.c \
        safe_strfirstsame.c \
        safe_strlastdiff.c \
        safe_strspn.c \
        safe_wcsncpy.c \
        safe_memcpy16.c \
        safe_memzero16.c \
        safe_strcmpfld.c \
        safe_strisalphanumeric.c \
        safe_strlastsame.c \
        safe_strstr.c \
        safe_wcsnlen.c \
        safe_memcpy32.c \
        safe_memzero32.c \
        safe_str_constraint.c \
        safe_strisascii.c \
        safe_strljustify.c \
        safe_strtok.c \
        safe_wmemcmp.c

OBJS = $(SRCS:.c=.o)

define CC11
    $(CC) $(CFLAGS) -c $< -o $(OBJDIR)/$(@F) $(INCLUDES)
endef

$(TARGET): $(addprefix $(OBJDIR)/, $(OBJS))
	$(AR) -rcs $(TARGET) $(addprefix $(OBJDIR)/, $(OBJS))

$(OBJDIR)/%.o:%.c
	$(CC11)

objdir :
	mkdir -p $(OBJDIR)

all: objdir $(TARGET)

clean:
	rm -f $(addprefix $(OBJDIR)/, $(OBJS)) $(ROOT)/$(TARGET)
	rm -fr $(OBJDIR)
