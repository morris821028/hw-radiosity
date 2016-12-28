FILE_PREFIX=Mig27
FILE_FORMAT=COLOR
#FILE_FORMAT=SIMPLE

all: tri2json

tri2json: ./model-converter/main.cpp
	g++ -std=c++11 -O2 ./model-converter/main.cpp -o tri2json
test:
	./tri2json -i ./models/$(FILE_PREFIX).tri -o $(FILE_PREFIX).json --format $(FILE_FORMAT)
move:
	mv $(FILE_PREFIX).json ~/Desktop/github/hw-computer-graphics/hw1-shading-cmp/WebGL/asset/
clean:
	rm tri2json
