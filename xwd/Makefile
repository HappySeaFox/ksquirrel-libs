NAME=read_xwd
NAME_C=$(NAME).c
NAME_H=$(NAME).h
NAME_OBJ=$(NAME).o
NAME_LIB=libSQ_$(NAME).so
CC=gcc
GENERAL_FLAGS=-Wall -O2
INSTALL_DIR=/usr/lib/squirrel/

all: $(NAME_OBJ) $(NAME_LIB)

$(NAME_LIB): $(NAME_OBJ)
	$(CC) $(GENERAL_FLAGS) -shared $(NAME_OBJ) -o $(NAME_LIB)
	rm -f $(NAME_OBJ)

$(NAME_OBJ): $(NAME_C) $(NAME_H) ../defs.h ../err.h
	$(CC) $(GENERAL_FLAGS) -fPIC -c $(NAME_C)

clean:
	rm -f $(NAME_OBJ)
	rm -f $(NAME_LIB)
  
install: all
	cp $(NAME_LIB) $(INSTALL_DIR)
