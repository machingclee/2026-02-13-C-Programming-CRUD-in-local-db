TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default

default: $(TARGET)
	./$(TARGET) -f ./my_new_db.db -n
	./$(TARGET) -f ./my_new_db.db -a  "James Lee,Hong Kong1,30"
	./$(TARGET) -f ./my_new_db.db -a  "James Bon,Hong Kong2,30"
	./$(TARGET) -f ./my_new_db.db -a  "Enoch Kung1,Hong Kong3,30"
	./$(TARGET) -f ./my_new_db.db -a  "Enoch Kung2,Hong Kong4,30"
clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude
