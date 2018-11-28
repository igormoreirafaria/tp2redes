server1:	server1.c	sds.c
	gcc server1.c sds.c -o server1

server2:	server2.c	sds.c
	gcc server2.c sds.c -pthread -o server2

clean: server1
	rm -rf server1