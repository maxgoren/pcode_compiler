dalgol:
	g++ -g main.cpp -o dalgol

install:
	mv ./dalgol /usr/local/bin

clean:
	rm dalgol