kadai1: kadai1.c
	gcc -L ./lib -I ./include kadai1.c -lX11 -ltiff -o kadai1 -lm
	./kadai1
