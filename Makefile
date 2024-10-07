# Nome do compilador
CC = gcc

# Flags do compilador
CFLAGS = -Wall -Wextra -pthread

# Arquivos fonte
SRCS = main.c functions.c

# Arquivos objeto (gerados a partir dos fontes)
OBJS = $(SRCS:.c=.o)

# Nome do executável
TARGET = programa

# Regra padrão para compilar tudo
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regra para compilar os arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa os arquivos objeto e o executável
clean:
	rm -f $(OBJS) $(TARGET)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET)
