# Add any other object files to this list below
COMMON_LOG = log
COMMON_LOG_OBJ = $(COMMON_LOG).o
COMMON_LOG_SRC = ./dir-common/log.c

COMMON_CJSON = cJSON
COMMON_CJSON_OBJ = $(COMMON_CJSON).o
COMMON_CJSON_SRC = ./dir-common/cJSON.c

COMMON_OBJS = $(COMMON_LOG_OBJ) $(COMMON_CJSON_OBJ)

APP1 = task-data
APP1_OBJ = $(APP1).o
APP1_SRC = ./dir-task-data/task-data.c

APP2 = task-state
APP2_OBJ = $(APP2).o
APP2_SRC = ./dir-task-state/task-state.c

APP3 = timesync-app
APP3_OBJ = $(APP3).o
APP3_SRC = ./dir-timesync-app/timesync-app.c

APP4 = config-parse
APP4_OBJ = $(APP4).o
APP4_SRC = ./dir-config-parse/config-parse.c

LDFLAGS += -I ./dir-common
LDLIBS += -lpthread -lzlog

all: build

build: $(APP1) $(APP2) $(APP3) $(APP4)

$(COMMON_LOG_OBJ): $(COMMON_LOG_SRC)
	$(CC) -c -o $@ $< $(LDFLAGS)

$(COMMON_CJSON_OBJ): $(COMMON_CJSON_SRC)
	$(CC) -c -o $@ $< $(LDFLAGS)

$(APP1_OBJ): $(APP1_SRC)
	$(CC) -c -o $@ $< $(LDFLAGS)

$(APP2_OBJ): $(APP2_SRC)
	$(CC) -c -o $@ $< $(LDFLAGS)

$(APP3_OBJ): $(APP3_SRC)
	$(CC) -c -o $@ $< $(LDFLAGS)

$(APP4_OBJ): $(APP4_SRC)
	$(CC) -c -o $@ $< $(LDFLAGS)

$(APP1): $(APP1_OBJ) $(COMMON_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP2): $(APP2_OBJ) $(COMMON_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP3): $(APP3_OBJ) $(COMMON_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP4): $(APP4_OBJ) $(COMMON_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP1) $(APP2) $(APP3) *.o

