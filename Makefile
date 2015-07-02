Solver.o: Solver.cpp Common.h Classifier.h Optimizer.h
	g++ -Wall Solver.cpp Classifier.cpp Optimizer.cpp -lX11 -lpthread -o Solver.o

clean:
	rm -f *.o