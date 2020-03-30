.PHONY: clean, mrproper
CC = gcc
CFLAGS = -g -Wall

APP:=test_audio_outside
#APP:=test_audio

LIBS += $(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --libs gstreamer-1.0)
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CFG_PATH) pkg-config --cflags gstreamer-1.0)

all: $(APP)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(APP): $(APP).o
	$(CC) $(CFLAGS) -o $@ $+ $(LIBS)

clean:
	rm -f *.o core.* $(APP)

mrproper: clean
	rm -f $(APP)
