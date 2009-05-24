

all:
	$(VALGRIND) ../../bin/mscgen -T png -i testinput0.msc -o test0.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput0.msc -o test0.svg -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput1.msc -o test1.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput1.msc -o test1.svg -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput2.msc -o test2.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput2.msc -o test2.svg -p
	$(VALGRIND) ../../bin/mscgen -T ismap -i testinput2.msc -o test2.ismap -p
	$(VALGRIND) ../../bin/mscgen -T eps -i testinput2.msc -o test2.eps -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput3.msc -o test3.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput3.msc -o test3.svg -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput4.msc -o test4.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput4.msc -o test4.svg -p
	$(VALGRIND) ../../bin/mscgen -T eps -i testinput4.msc -o test4.eps -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput5.msc -o test5.svg -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput5.msc -o test5.png -p
	$(VALGRIND) ../../bin/mscgen -T eps -i testinput5.msc -o test5.eps -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput6.msc -o test6.svg -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput6.msc -o test6.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput7.msc -o test7.svg -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput7.msc -o test7.png -p
	$(VALGRIND) ../../bin/mscgen -T eps -i testinput7.msc -o test7.eps -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput8.msc -o test8.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput8.msc -o test8.svg -p	
	$(VALGRIND) ../../bin/mscgen -T eps -i testinput8.msc -o test8.eps -p
	$(VALGRIND) ../../bin/mscgen -T png -i testinput9.msc -o test9.png -p
	$(VALGRIND) ../../bin/mscgen -T svg -i testinput9.msc -o test9.svg -p	
	$(VALGRIND) ../../bin/mscgen -T eps -i testinput9.msc -o test9.eps -p

clean:
	rm -f test0.png test0.svg \
	test1.png test1.svg \
	test2.png test2.svg test2.ismap test2.eps \
	test3.png test3.svg \
	test4.png test4.svg test4.eps \
	test5.png test5.svg \
	test5.png test5.svg test5.eps \
	test6.png test6.svg \
	test7.png test7.svg test7.eps \
	test8.png test8.svg test8.eps \
	test9.png test9.svg test9.eps
	
.PHONY: clean all
