.PHONY: all clean run

UNAME := $(shell uname -s)

all:
ifeq ($(UNAME),Linux)
	$(MAKE) -C c/linux all
else
	$(MAKE) -C c/windows all
endif

run:
ifeq ($(UNAME),Linux)
	$(MAKE) -C c/linux run
else
	$(MAKE) -C c/windows run
endif

clean:
	$(MAKE) -C c/linux clean
	$(MAKE) -C c/windows clean
