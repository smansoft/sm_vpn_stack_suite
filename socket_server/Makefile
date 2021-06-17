
.PHONY = all init clean bindir objdir
.DEFAULT_GOAL = all

ROOT=.

#SM_FILE_BUILD	= Release
#SM_FILE_BUILD	= Debug

CXX = g++
LD = g++

SSTRING	= sstring_lib-0.0.1.1
SM_LOG	= sm_log_lib-0.0.2.2

ifeq ($(SM_FILE_BUILD),Release)
	CXXFLAGS 	= -O3 -DNDEBUG -DSM_OS_LINUX -DSM_SYNC_LOG -Wall -fmessage-length=0 -std=c++17		# release flags
else
	CXXFLAGS 	= -O0 -g3 -ggdb -DDEBUG -DSM_OS_LINUX -DSM_SYNC_LOG -Wall -fmessage-length=0 -std=c++17	# debug flags
endif

LD_FLAGS  	= '-Wl,-L,$(ROOT)/$(SSTRING),-L$(ROOT)/$(SM_LOG)'

VPATH 		= $(ROOT)/src
OBJDIR 		= $(ROOT)/obj

BIN_OUT := $(ROOT)/_bin

INCLUDES 	= -I$(ROOT)/include -I$(SSTRING)/include -I$(SM_LOG)/include

SOCKET_SERVER_SRCS 		= socket_server.cpp

SOCKET_SERVER_OBJS 		= $(SOCKET_SERVER_SRCS:.cpp=.o)

LIBS = -lpthread -l$(SM_LOG) -l$(SSTRING)

TARGET_SOCKET = socket_server

define CXX17
    $(CXX) $(CXXFLAGS) $(DEFINES) -c $< -o $(OBJDIR)/$(@F) $(INCLUDES)
endef

$(TARGET_SOCKET): $(addprefix $(OBJDIR)/, $(SOCKET_SERVER_OBJS))
	${LD} ${LD_FLAGS} -o $(TARGET_SOCKET) $(addprefix $(OBJDIR)/, $(SOCKET_SERVER_OBJS)) $(LIBS)

$(OBJDIR)/%.o:%.cpp
	$(CXX17)

objdir :
	mkdir -p $(OBJDIR)

bindir :
	mkdir -p $(BIN_OUT)/bin
	mkdir -p $(BIN_OUT)/share

sstring_lib:
	$(MAKE) -C $(SSTRING)

sm_log_lib:
	$(MAKE) -C $(SM_LOG)

all: sstring_lib sm_log_lib objdir $(TARGET_SOCKET)

clean:
	rm -f $(addprefix $(OBJDIR)/, $(OBJS)) $(ROOT)/$(TARGET_SOCKET)
	rm -fr $(OBJDIR)
	$(MAKE) -C $(SM_LOG) clean 
	$(MAKE) -C $(SSTRING) clean 

install: bindir
	cp -f $(ROOT)/$(TARGET_SOCKET) ${BIN_OUT}/bin
