all:
	g++ -g -o conversion_test conversion_test.cpp -I../..
	g++ -g -o data_encode_test data_encode_test.cpp -I../..

test:
	./conversion_test
	./data_encode_test

clean:
	rm -rf conversion_test