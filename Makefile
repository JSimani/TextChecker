CXX      = g++
CXXFLAGS = -g3 -Wall -Wextra -O3
LDFLAGS  = -g3

# Compiles the program. You just have to type "make"
check: checker.o wordWrap.o
	${CXX} ${LDFLAGS} -o check checker.o wordWrap.o
checker.o: checker.cpp
wordWrap.o: wordWrap.cpp wordWrap.h


# Cleans the current folder of all compiled files
clean:
	rm -rf check *.o *.dSYM
	