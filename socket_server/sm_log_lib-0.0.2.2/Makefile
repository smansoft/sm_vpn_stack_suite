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

TARGET = libsm_log_lib-0.0.2.2.a

SSTRING = sstring_lib-0.0.1.1

VPATH 	= $(ROOT)/src
OBJDIR 	= $(ROOT)/obj

INCLUDES 	= -I$(ROOT)/include -I$(ROOT)/src -I$(ROOT)/../$(SSTRING)/include -I$(ROOT)/../$(SSTRING)/src

SRCS 		= sm_files_tools.c sm_log.c

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
