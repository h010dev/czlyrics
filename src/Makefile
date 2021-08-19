CC ?= gcc
DEPS = external/mongoose/mongoose.c external/mjson/mjson.c
FLAGS = -DMG_ENABLE_OPENSSL=1
INC = -I/external/mongoose -I/external/mjson -I/usr/include/openssl
LIBS = -L/usr/lib/
LDFLAGS = -lssl -lcrypto

SRC = main.c czlyrics-api.c czlyrics-spider.c czlyrics-parser.c
OBJS = $(SRC:.c=.o)

all: czlyrics

czlyrics: $(OBJS) 
	$(CC) $(OBJS) -o $(@F) $(DEPS) $(FLAGS) $(INC) $(LIBS) $(LDFLAGS) -Wall -Wpedantic -Wextra

main.o: main.c
	$(CC) -c main.c

czlyrics-api.o: czlyrics-api.c czlyrics-api.h
	$(CC) -c czlyrics-api.c

czlyrics-parser.o: czlyrics-parser.c czlyrics-parser.h
	$(CC) -c czlyrics-parser.c

czlyrics-spider.o: czlyrics-spider.c czlyrics-spider.h
	$(CC) -c czlyrics-spider.c

clean:
	rm -f $(OBJS)
	rm -f czlyrics
