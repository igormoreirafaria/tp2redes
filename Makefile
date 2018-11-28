all:	server1.c	sds.c
	gcc server1.c sds.c -o server1

clean: server1
	rm -rf server1