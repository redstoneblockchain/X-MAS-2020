
all:
	g++ -o emu10 -Wall -pedantic -Wextra `sdl2-config --cflags` emu10vm.cpp -lpthread -std=c++17 `sdl2-config --libs`
	g++ -o emu10dasm -Wall -pedantic -Wextra emu10dasm.cpp -std=c++17
	g++ -o emu10asm -Wall -pedantic -Wextra emu10asm.cpp -std=c++17

fast:
	g++ -O3 -o emu10 -Wall -pedantic -Wextra `sdl2-config --cflags` emu10vm.cpp -lpthread -std=c++17 `sdl2-config --libs` -DNDEBUG

clean:
	rm -f emu10
