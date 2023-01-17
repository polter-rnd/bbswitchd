TARGET ?= bbswitch
PREFIX ?= /usr

all:
	$(MAKE) -C src

.PHONY: install
install:
	$(MAKE) -C src install
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

.PHONY: uninstall
uninstall:
	$(MAKE) -C src uninstall
	$(RM) -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(PREFIX)/bin/

.PHONY: clean
clean:
	$(MAKE) -C src clean

-include $(DEPS)
