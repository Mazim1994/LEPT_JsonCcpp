CC := g++
FLAG := -g -std=c++11
EXECUTBALE := test.exe
SOURCES := test.cpp JsonCpp.cpp
OBJECT := test.o JsonCpp.o

$(EXECUTBALE): $(OBJECT)
	$(CC) $(FLAG) -o $@ $^
test.o: test.cpp
	$(CC) $(FLAG) -c $<
JsonCpp.o: JsonCpp.cpp
	$(CC) $(FLAG) -c $<


.PHONY: clean
clean:
	del test.o JsonCpp.o test.exe