CC = cc
BIN = pconf

$(BIN):
	$(CC) fio.c main.c proc.c utils.c -o $(BIN) -Wall

all:	$(BIN)

clean:
	rm -f *.o a.out core $(BIN)
