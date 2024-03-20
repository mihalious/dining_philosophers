CC := gcc
CFLAGS := -std=c2x -g -O2 -Wall -Wextra -Wformat=2 -fanalyzer
CPPFLAGS :=
LDFLAGS := -fuse-ld=mold -lpthread

SRC := src
OBJ := target/obj
BIN := target/dining_philosophers

OBJECTS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(wildcard $(SRC)/*.c))

run: $(BIN)
	./$<

build: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJECTS): $(OBJ)/%.o : $(SRC)/%.c
		$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(wildcard $(OBJ)/*.o) $(BIN)

#OBJECTS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(wildcard $(SRC)/*.c))
#
#run: build
#	./$(BIN)
#
#build: $(OBJECTS)
#	$(CC) $(LDFLAGS) $^ -o $(BIN)
#
#$(OBJECTS): $(OBJ)/%.o : $(SRC)/%.c
#		$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

