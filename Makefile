CPP = g++
FLAGS = -std=gnu++17 -Wall -Wextra

webserver: Makefile main.o webserver.o 
		$(CPP) $(FLAGS) -o webserver main.o webserver.o

main.o:  Makefile main.cpp
		$(CPP) $(FLAGS) -c main.cpp -o main.o

webserver.o:  Makefile webserver.cpp
		$(CPP) $(FLAGS) -c webserver.cpp -o webserver.o
		
clean:
	rm -rf *.o 

distclean:
	rm -rf webserver *.o