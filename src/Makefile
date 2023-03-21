BIN:=prog_for_mint
SRCS:=$(wildcard *.c)
CPPFLAGS:= -Wall #-I/home/studentas/Documents/mosquitto/include
LDFLAGS:= -lmosquitto -lubus -lubox -lblobmsg_json#-L/home/studentas/Documents/mosquitto/lib -lmosquitto

.PHONY: all clean debug

all: $(BIN)

$(BIN): $(SRCS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(SRCS)
