default: myshell

myshell.o: myshell.c $(HEADERS)
	gcc -c myshell.c -o myshell.o

myshell: myshell.o
	gcc myshell.o -o myshell

clean:
	-rm -f myshell.o
	-rm -f myshell