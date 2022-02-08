TARGET ?= rssfeeder
INC_DIRS ?= /usr/include/libxml2/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LIBS := -lxml2
SRCS := rssfeeder.c

CFLAGS := -Wall -Wextra #-Werror

$(TARGET): $(SRCS)
	$(CC) $(INC_FLAGS) $(CFLAGS) -o $@ $< $(LIBS) 

.PHONY: clean format
clean:
	rm -f $(TARGET)

format:
	clang-format -i $(SRCS)
