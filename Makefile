all:	
	gcc schedule.c -o schedule
	gcc time.c -o time.o
clean:
	rm -f schedule time.o
