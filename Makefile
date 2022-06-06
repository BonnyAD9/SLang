CC:=clang
NAME:=SLang
FILE:=bin/$(NAME).exe
CFLAGS:=-Wall -std=c17 -g
DBGFLG:=-D_DEBUG -DASSERT_EXIT
CFILES:=$(wildcard src/*.c)
OBJS:=$(patsubst src/%.c, obj/%.o, $(CFILES))

debug: $(OBJS)
	$(CC) $(OBJS) -o $(FILE)

release:
	$(CC) $(CFLAGS) -DNDEBUG $(CFILES) -O3 -o bin/release/$(NAME).exe

$(OBJS): $(CFILES)
	$(CC) -c $(CFLAGS) $(DBGFLG) $(patsubst obj/%.o, src/%.c, $@) -o $@

run: debug
	./$(FILE)

clean:
	del $(patsubst obj/%, obj\\%, $(OBJS))
