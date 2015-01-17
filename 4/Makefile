all:
	yacc -d -v parser.y
	lex scanner.l
	gcc -o parser lex.yy.c y.tab.c symtbl.c gen.c -ly -lfl

clean:
	rm -f lex.yy.c
	rm -f y.tab.c
	rm -f y.tab.h
	rm -f y.output
	rm -f parser

test:
	./parser testfile

makerun:
	yacc -d -v parser.y
	lex scanner.l
	gcc -o parser lex.yy.c y.tab.c symtbl.c -ly -lfl
	./parser testfile

sync:
	scp parser.y lmchen@140.113.89.3:~/CP/
	scp scanner.l lmchen@140.113.89.3:~/CP/
	scp symtbl.h lmchen@140.113.89.3:~/CP/
	scp symtbl.c lmchen@140.113.89.3:~/CP/
	scp Makefile lmchen@140.113.89.3:~/CP/
    
