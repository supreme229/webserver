CPP = g++
FLAGS = -std=c++17 -Wall -Wextra

webserver: Makefile main.o webserver.o utils.o
		$(CPP) $(FLAGS) -o webserver main.o webserver.o utils.o

main.o:  Makefile main.cpp
		$(CPP) $(FLAGS) -c main.cpp -o main.o

webserver.o:  Makefile webserver.cpp
		$(CPP) $(FLAGS) -c webserver.cpp -o webserver.o

utils.o:  Makefile utils.cpp
		$(CPP) $(FLAGS) -c utils.cpp -o utils.o	

clean:
	rm -rf *.o 

distclean:
	rm -rf webserver *.o