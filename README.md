# CRUD of a `.db` file in C

## What it does
### Purpose
A simple console program that reads different flags  which creates a  `.db` file and performs basic CRUD to it.

This simple project is to study in C programming how ***memory reallocation*** is instructed manually with maintainable code (such as using double pointer `T**` to let a function create dynamically sized list).


## How to use it

- Build it via executing
  ```
  make
  ```
  which executes the default rule named `$(TARGET)` listed in `Makefile`

- The default rule has already seeded some data by:

  ```makefile
  default: $(TARGET)
	./$(TARGET) -f ./my_new_db.db -n
	./$(TARGET) -f ./my_new_db.db -a  "James Lee,Hong Kong1,30"
	./$(TARGET) -f ./my_new_db.db -a  "James Bon,Hong Kong2,30"
	./$(TARGET) -f ./my_new_db.db -a  "Enoch Kung1,Hong Kong3,30"
	./$(TARGET) -f ./my_new_db.db -a  "Enoch Kung2,Hong Kong4,30"
  ```

- To list all records, run
  ```
  ./bin/dbview -f my_new_db.db -l 
  ```
- To delete a record, run
  ```
  ./bin/dbview -f my_new_db.db -d "Enoch Kung1
  ```
- To add a record, run
  ```
  ./bin/dbview -f ./my_new_db.db -a  "Enoch Kung3,Hong Kong4,40"
  ```