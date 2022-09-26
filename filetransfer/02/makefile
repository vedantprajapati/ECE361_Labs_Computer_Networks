CC=gcc
all:
	server deliver

server:
	server.o
	
deliver: 
	deliver.o

clean: 
	rm -f *.o server deliver

test_file:
	dd if=/dev/zero of=./test.file bs=4k iflag=fullblock,count_bytes count=$(count)