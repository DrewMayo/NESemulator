TARGET=nes
CODEDIR=. lib
INCLUDE=. ./include/
OBJDIR=./build/

CC=gcc
CFLAGS=-Wall -Wextra -g $(foreach D, $(INCLUDE),-I$(D)) $(DEPFLAGS)
DEPFLAGS=-MP -MD -lSDL2 -DDEBUG
LDFLAGS=-lSDL2 -g
CFILES=$(foreach D,$(CODEDIR),$(wildcard $(D)/*.c))
OBJS=$(patsubst %.c,%.o,$(CFILES))
DEPFILES=$(patsubst %.c,%.d,$(CFILES))

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(foreach I, $^, $(OBJDIR)$(I)) $(LDFLAGS)

%.o:%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)$@ $^

clean:
	rm -rf $(TARGET) $(foreach D, $(OBJS), $(OBJDIR)$(D)) $(foreach D, $(DEPFILES), $(OBJDIR)$(D)), $(OBJDIR)
