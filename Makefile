TARGET ?= rssfeeder
INC_FLAGS := -I/usr/local/include/libxml2
LIBS := -L/usr/local/lib -lxml2
SRCS := rssfeeder.c

CFLAGS := -Wall -Wextra #-Werror

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INC_FLAGS) -o $@ $< $(LIBS)

.PHONY: clean format
clean:
	rm -f $(TARGET)

format:
	clang-format -i $(SRCS)
