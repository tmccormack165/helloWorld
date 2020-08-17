out : main.c
	gcc  main.c electFunctions.c -o opt -lpthread -w
	gcc elect.c -o naive -w

clean:
	rm  naive opt









