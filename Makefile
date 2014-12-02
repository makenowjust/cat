CFLAGS = -std=c99

cat: cat.o
	${CC} -o $@ cat.o

clean:
	rm *.o cat
