# hw-radiosity #

CPU-based [Radiosity](https://en.wikipedia.org/wiki/Radiosity_(computer_graphics)) (computer graphics)

## Usage ##

* Compiler: g++ 4.8.5 - support C++14
* Hardware: multi-core processors
* OS: Unix-like system
* [optional] python 2.x for demo

## Build ##

```bash
git clone https://github.com/morris821028/hw-radiosity
cd hw-radiosity && make
```

## Demo (WebGL) ##


### For server ###

```bash
cd hw-radiosity && ./server.sh
```

link http://127.0.0.1:8888/demo/public/

### For localhost ###

```bash
cd hw-radiosity/demo/public
```

#### Windows ####

```
./chrome.exe -allow-file-access-from-files
```

#### Macbook ####

```bash
/Applications/Google Chrome.app/Contests/MacOS/Google Chrome -allow-file-access-from-files
```

