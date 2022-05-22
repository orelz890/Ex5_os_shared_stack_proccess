all: server client
client: client.cpp
	g++ -o client client.cpp

server: server.cpp stack.cpp stack.hpp
	g++ -pthread -o server server.cpp stack.cpp

test: Test
	./Test

Test: Test.cpp
	g++ -o Test Test.cpp

clean:
	rm -f *.o *.out server client Test
