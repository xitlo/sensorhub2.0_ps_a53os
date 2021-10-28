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

APP5 = ./ampperf/ampperf
APP5_SRC = $(APP5).c

APP6 = ./ampecho/ampecho
APP6_SRC = $(APP6).c

LDFLAGS += -I ./common
LDLIBS += -lpthread -lzlog

all: build

build: $(APP1) $(APP2) $(APP3) $(APP4) $(APP5) $(APP6)

$(APP1): $(APP1_SRC) $(COMMON_SRCS)
	$(CC) -o  $@ $^ $(LDFLAGS) $(LDLIBS) -g

$(APP2): $(APP2_SRC) $(COMMON_SRCS)
	$(CC) -o  $@ $^ $(LDFLAGS) $(LDLIBS) -g

$(APP3): $(APP3_SRC) $(COMMON_SRCS)
	$(CC) -o  $@ $^ $(LDFLAGS) $(LDLIBS) -g

$(APP4): $(APP4_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP5): $(APP5_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP6): $(APP6_SRC) $(COMMON_SRCS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP1) $(APP2) $(APP3) $(APP4) $(APP5) $(APP6) *.o

