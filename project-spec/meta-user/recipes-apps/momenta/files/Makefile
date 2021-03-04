# Add any other object files to this list below
COMMON_LOG = log
COMMON_LOG_SRC = ./common/$(COMMON_LOG).c

COMMON_CJSON = cJSON
COMMON_CJSON_SRC = ./common/$(COMMON_CJSON).c

COMMON_MMAP = mem-mmap
COMMON_MMAP_SRC = ./common/$(COMMON_MMAP).c

COMMON_SRCS = $(COMMON_LOG_SRC) $(COMMON_CJSON_SRC) $(COMMON_MMAP_SRC)

APP1 = ./task-data/task-data
APP1_SRC = $(APP1).c

APP2 = ./task-state/task-state
APP2_SRC = $(APP2).c

APP3 = ./timesync-app/timesync-app
APP3_SRC = $(APP3).c

APP4 = ./config-parse/config-parse
APP4_SRC = $(APP4).c

LDFLAGS += -I ./common
LDLIBS += -lpthread -lzlog

all: build

build: $(APP1) $(APP2) $(APP3) $(APP4)

$(APP1): $(APP1_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP2): $(APP2_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP3): $(APP3_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP4): $(APP4_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP1) $(APP2) $(APP3) $(APP4) *.o

