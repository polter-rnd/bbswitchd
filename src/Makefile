TARGET ?= bbswitchd
PREFIX ?= /usr

SRCS := logger.c \
        bbswitch.c \
        module.c \
        pci.c \
        server.c \
        main.c
LDLIBS := -lkmod

OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

CFLAGS += -MMD -MP -Wall -Wextra -Werror

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

.PHONY: install
install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/sbin/
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/sbin/

.PHONY: uninstall
uninstall: $(TARGET)
	$(RM) -f $(DESTDIR)$(PREFIX)/sbin/$(TARGET)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(PREFIX)/sbin/

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
