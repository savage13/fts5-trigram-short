
# Assuming installed within a node framework
#   using better-sqlite3
# Otherwise the path to the version of sqlite3 that you are using

SQLITE_PATH=../../node_modules/better-sqlite3/deps/sqlite3

CFLAGS=-Wall -Wextra \
	-shared -fPIC \
	-I$(SQLITE_PATH)
OBJ=trigram-short.so
SRC=trigram-short.c

all: $(OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OBJ)
clean:
	rm -rf $(OBJ) *~ node_modules
