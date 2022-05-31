CC:=gcc
NAME:=SLang
FILE:=bin/$(NAME).exe
CFLAGS:=-Wall -std=c17 -g -D_DEBUG -DASSERT_EXIT
CFILES:=$(wildcard *.c)
OBJS:=$(patsubst %.c, obj/%.o, $(CFILES))

debug: $(OBJS)
	$(CC) -lm $(OBJS) -o $(FILE)

release:
	$(CC) $(CFLAGS) $(CFILES) -O3 -o bin/release/$(NAME).exe

$(OBJS): $(CFILES)
	$(CC) -lm -c $(CFLAGS) $(patsubst obj/%.o, %.c, $@) -o $@

run: debug
	./$(FILE)

clean:
	del $(patsubst obj/%, obj\\%, $(OBJS))
