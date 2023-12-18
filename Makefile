

RM=rm -f
MAKE=make

CROSS_COMPILE=#loongarch64-linux-gnu-
GCC=$(CROSS_COMPILE)gcc 

CP=cp
SOURCE=source


VERSION="const char version[]=\"version: 0.0.0.1\""
BUILDTIME="const char buildtime[]=\"buildtime: `date +%Y-%-m-%d-%H:%M`\""

INCLUDES=-I./lib
LOADDIR=-L./lib
LIBS=-lmath -lLibPrb0400Drv 
STATIC_LIBS=lib/libLibPrb0400Drv_dbg.a lib/libmath.a

DLL_FLAGS=-fPIC -shared
GCC_FLAGS=-g -DLED_NUM=2 -DDDP_NUM=2 -DOS_LINUX -DDEBUG -O2 -Wall -funsigned-char -DVERSION=$(VERSION) -DBUILDTIME=$(BUILDTIME)

SYS_LIBS=-ldl -lm -lpthread

export OBJSDIR = $(shell pwd)

TARGET = rpiotest
TCPSERVER = server
TCPCLIENT = client

OBJS=main.o 

CLINETOBJS=clientMain.o tcp_client.o rpio_read.o run_mutex.o
SERVEROBJS=serverMain.o tcp_server.o rpio_write.o run_mutex.o linklist.o

$(OBJS):%.o:%.c
	$(GCC) -c $< -o $@ $(GCC_FLAGS) $(INCLUDES) $(LOADDIR) $(LIBS)

$(CLINETOBJS):%.o:%.c
	$(GCC) -c $< -o $@ $(GCC_FLAGS) $(INCLUDES) $(LOADDIR) $(LIBS)

$(SERVEROBJS):%.o:%.c
	$(GCC) -c $< -o $@ $(GCC_FLAGS) $(INCLUDES) $(LOADDIR) $(LIBS)
################################################

$(TARGET): $(CLINETOBJS)
	$(GCC) -o $@ $? $(GCC_FLAGS) $(SYS_LIBS) $(STATIC_LIBS)

$(TCPCLIENT): $(CLINETOBJS)
	$(GCC) -o $@ $? $(GCC_FLAGS) $(SYS_LIBS) $(STATIC_LIBS)

$(TCPSERVER): $(SERVEROBJS)
	$(GCC) -o $@ $? $(GCC_FLAGS) $(SYS_LIBS) $(STATIC_LIBS)

################################################
clean:
	$(RM) *.o $(TARGET) $(TCPCLIENT) $(TCPSERVER)

all:clean $(TARGET) $(TCPCLIENT) $(TCPSERVER)

