Solver.o: Solver.cpp Common.h Classifier.h Optimizer.h
	g++ -Wall -c Solver.cpp -lX11 -lpthread -o Solver.o
