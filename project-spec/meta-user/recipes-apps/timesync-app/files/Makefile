APP = timesync-app

# Add any other object files to this list below
# APP_OBJS = timesync-app.o log.o
LOCAL_PATH := $(shell pwd)
LOCAL_SRCS	:= $(wildcard $(LOCAL_PATH)/*.c)

# LDFLAGS += -I . -L ../../zlog/files
LDLIBS += -lpthread -lzlog

all: build

build: $(APP)

# $(APP_OBJS):
#	$(CC) -o $@ $(APP_OBJS) $(LDFLAGS) $(LDLIBS)

$(APP):
	$(CC) -o $@ $(LOCAL_SRCS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP) *.o

