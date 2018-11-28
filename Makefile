server1:	server1.c	sds.c
	gcc server1.c sds.c -o server1

server2:	server2.c	sds.c
	gcc server2.c sds.c -pthread -o server2

server3:	server3.c	sds.c	fila.c
	gcc server3.c sds.c fila.c -pthread -o server3

server4:	server4.c	sds.c
	gcc server4.c sds.c fila.c -o server4


clean: server1
	rm -rf server1