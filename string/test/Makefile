all:
	g++ -ggdb -o test.o string_test.cpp -I../..

test:
	rm *.o
	g++ -o test.o string_test.cpp -I../..
	./test

clean:
	rm *.o
