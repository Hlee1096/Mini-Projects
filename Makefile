
CC := gcc
CFLAGS := -Wall -Wextra -lpthread -fsanitize=address 
SOURCE_FILE := source/server.c source/dynamic_command_arr.c source/broadcast_dyn_arr.c source/command_queue.c source/dynamic_queue_arr.c source/client_handler.c source/client_handler_functions.c source/markdown.c source/pthread_list.c source/server_execution.c source/version_queue.c source/version_queue_array.c 

all: server client

server: source/server.c
	$(CC) $(CFLAGS) $(SOURCE_FILE) -o server

client: source/client.c
	$(CC) $(CFLAGS) source/client.c source/markdown.c source/client_execution.c -o client


markdown.o: source/markdown.c
	$(CC) $(CFLAGS) -c source/markdown.c -o markdown.o

clean:
	rm -f server
	rm -f client
	rm -f FIFO_C2S* 
	rm -f FIFO_S2C*

renew: clean all