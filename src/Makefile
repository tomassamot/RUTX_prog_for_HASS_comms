BIN:=prog_for_mint
SRCS:=$(wildcard *.c)
CPPFLAGS:=
LDFLAGS:= -largp -lmosquitto -lubus -lubox -lblobmsg_json -llua -llualib 

.PHONY: all clean debug

all: $(BIN)

$(BIN): $(SRCS)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(SRCS)
