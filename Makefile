twitchdrop: main.c
	mkdir -p build
	gcc -O0 -Wextra -Wall -g -ltcl -o build/twitchdrop main.c

