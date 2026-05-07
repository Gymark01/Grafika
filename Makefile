CC = gcc

CFLAGS = -Wall -Iincludes
LDFLAGS = -lfreeglut -lopengl32 -lglu32 -lwinmm

SRC = \
src/main.c \
src/map.c \
src/car.c \
src/camera.c \
src/model.c \
src/assets.c \
src/config.c \
cJSON.c

OBJ = $(SRC:.c=.o)

TARGET = game.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q src\*.o $(TARGET)