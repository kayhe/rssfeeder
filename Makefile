TARGET := rssfeeder

OS != uname

INCDIRS_Linux := /usr/include/libxml2/
INCDIRS_FreeBSD := /usr/local/include/libxml2/
INCDIRS := $(INCDIRS_$(OS))
INCFLAGS := -I$(INCDIRS) 
LDLIBS := -lxml2
LDFLAGS_Linux :=
LDFLAGS_FreeBSD := -L/usr/local/lib/
LDFLAGS := $(LDFLAGS_$(OS))
CFLAGS := -Wall -Wextra -Werror $(INCFLAGS)

SRCS := rssfeeder.c
OBJS := $(SRCS:.c=.o)

all: $(TARGET)

$(OBJS): %.o: %.c

$(TARGET): rssfeeder.o

.PHONY: clean format
clean:
	rm -f $(TARGET) $(OBJS)

format:
	clang-format -i $(SRCS)
