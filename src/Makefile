CC := g++ -Wall -mfpmath=sse -mieee-fp -mno-fma -mno-fma4
CXX := g++ -Wall -mfpmath=sse -mieee-fp -mno-fma -mno-fma4
COMMON := ACJ.o box.o elimination.o SL2ACJ.o Complex.o roundoff.o

verify: verify.o $(COMMON)
	$(CC) verify.o $(COMMON) -lm -o ../bin/verify

identify: identify.o $(COMMON)
	$(CC) identify.o $(COMMON) -lm -o ../bin/identify

rootcat: rootcat.o
	$(CC) rootcat.o -o ../bin/rootcat

# never optimize here, since the timing of arithmetic is important
tests: test_underflow.o test_overflow.o test_powers.o roundoff.o
	$(CC) test_underflow.o roundoff.o -o ../bin/test_underflow
	$(CC) test_overflow.o roundoff.o -o ../bin/test_overflow
	$(CC) test_powers.o roundoff.o -o ../bin/test_powers

