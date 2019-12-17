IDIR = include
LDIR = /usr/lib/mysql
CC = g++
CFLAGS = -I$(IDIR) -L$(LDIR)

ODIR = obj
SDIR = src

LIBS = -lmysqlclient -lpthread

_DEPS = MysqlQuery.h Server.h
DEPS = $(patsubst %, $(IDIR)/%, $(_DEPS))

_OBJ = MysqlQuery.o Server.o main.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ server $(INCDIR)/*~
