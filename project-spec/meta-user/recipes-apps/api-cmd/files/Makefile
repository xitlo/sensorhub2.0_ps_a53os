APP = api_cmd

# Add any other object files to this list below
APP_OBJS = $(APP).o

LDFLAGS += -I . -L .

LDLIBS += -lpthread

# MYCC = aarch64-linux-gnu-gcc

# SRC_FILE = $(APP).c
LOCAL_PATH := $(shell pwd)
LOCAL_SRCS	:= $(wildcard $(LOCAL_PATH)/*.c)

all: build

build: $(APP)

# $(APP_OBJS):
#	$(MYCC) -c -o $@ $(LOCAL_SRCS) $(LDFLAGS)

# $(APP): $(APP_OBJS)
#	$(MYCC) -o $@ $(APP_OBJS) $(LDFLAGS) $(LDLIBS)

$(APP):
	$(CC) -o $@ $(LOCAL_SRCS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP) *.o

