# Add any other object files to this list below
COMMON_LOG = log
COMMON_LOG_OBJ = $(COMMON_LOG).o
COMMON_LOG_SRC = ./dir-common/log.c

APP1 = task-data
APP1_OBJ = $(APP1).o
APP1_SRC = ./dir-task-data/task-data.c

APP2 = task-state
APP2_OBJ = $(APP2).o
APP2_SRC = ./dir-task-state/task-state.c

APP3 = timesync-app
APP3_OBJ = $(APP3).o
APP3_SRC = ./dir-timesync-app/timesync-app.c

LDFLAGS += -I ./dir-common
LDLIBS += -lpthread -lzlog

all: build

build: $(APP1) $(APP2) $(APP3)

$(COMMON_LOG_OBJ):
	$(CC) -c -o $@ $(COMMON_LOG_SRC) $(LDFLAGS)

$(APP1_OBJ):
	$(CC) -c -o $@ $(APP1_SRC) $(LDFLAGS)

$(APP1): $(APP1_OBJ) $(COMMON_LOG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP2_OBJ):
	$(CC) -c -o $@ $(APP2_SRC) $(LDFLAGS)

$(APP2): $(APP2_OBJ) $(COMMON_LOG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(APP3_OBJ):
	$(CC) -c -o $@ $(APP3_SRC) $(LDFLAGS)

$(APP3): $(APP3_OBJ) $(COMMON_LOG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP1) $(APP2) $(APP3) *.o

