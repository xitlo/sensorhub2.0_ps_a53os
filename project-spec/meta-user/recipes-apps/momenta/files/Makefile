# Add any other object files to this list below
APP1 = task-data
APP1_OBJ = ${APP1}.o
APP1_SRC = ./dir-task-data/task-data.c

APP2 = task-state
APP2_OBJ = ${APP2}.o
APP2_SRC = ./dir-task-state/task-state.c

LDLIBS += -lpthread -lzlog

all: build

build: $(APP1) $(APP2)

$(APP1_OBJ):
	$(CC) -c -o $@ $(APP1_SRC) $(LDFLAGS)

$(APP1): $(APP1_OBJ)
	$(CC) -o $@ $(APP1_OBJ) $(LDFLAGS) $(LDLIBS)

$(APP2_OBJ):
	$(CC) -c -o $@ $(APP2_SRC) $(LDFLAGS)

$(APP2): $(APP2_OBJ)
	$(CC) -o $@ $(APP2_OBJ) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP1) $(APP2) *.o

