compile:
	mpic++ -std=c++17 -o wtf main.cpp librandomizer.a
run:
	mpirun -np 1 ./wtf data/8717_31525/edges.dat 8717 31525 0.1 5000 30 20 369
clean:
	rm wtf
