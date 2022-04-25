out:main.o mysqldb.o handlePackets.o
	gcc -g -o out main.o mysqldb.o handlePackets.o -lpcap -lmysqlclient
main.o:main.c mysqldb.h handlePackets.h
	gcc -g -c main.c
mysqldb.o:mysqldb.c mysqldb.h
	gcc -g -c mysqldb.c
handlePackets.o:mysqldb.h handlePackets.h head_struct.h head_file.h
	gcc -g -c handlePackets.c
clean:
	rm -rf *.o
	rm -rf ./out
