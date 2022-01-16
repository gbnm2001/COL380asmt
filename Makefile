CC=g++ -fopenmp
#g++ -std=c++11 -O2 -mavx2 -c classify.cpp 
CFLAGS=-std=c++11 -O2

sources=main.cpp classify.cpp
objects=$(sources:.cpp=.o)

classify:$(objects)
	$(CC) $(CFLAGS) $^ -o $@

run: classify
	./classify rfile dfile 1009072 4 3

clean:
	rm main.o classify.o classify

valgrind:
	g++ -fopenmp -g -std=c++11 -O2 -c main.cpp
	g++ -fopenmp -g -std=c++11 -O2 -c classify.cpp
	g++ -fopenmp -g -std=c++11 -O2 main.o classify.o -o classify

callgrind:
	valgrind --tool=callgrind ./classify rfile dfile 1009072 4 1
	
massif:
	valgrind --tool=massif ./classify rfile dfile 1009072 4 1
	
cachegrind:
	valgrind --tool=cachegrind ./classify rfile dfile 1009072 4 1

perfStat:
	sudo perf stat ./classify rfile dfile 1009072 4 1

#classify.o: classify.h
#	touch classify.o
#	$(CC) $(CFLAGS) -c $<
#main.o: classify.h
#	$(CC) $(CFLAGS) -c $<

%.o: %.cpp classify.h
	$(CC) $(CFLAGS) -c $<
