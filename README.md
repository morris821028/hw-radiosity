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

## Options ##

```
Usage: ./rad [options] input_file -o output_file
OPTIONS
   Debug Options
       -debug <integer>        Output intermediate result according debug level.
                               Default -debug 0

   Radiosity Options
       -adapt_area <float>     The threahold of adaptive splitting algorithm.
                               Default -area 5.000000
       -sample_area <float>    When the difference of form factor for each vertex is
                               greater than delta form factor, it should try to split.
                               Default by model-dependent
       -converge <float>       The radiosity B is the energy per unit area unit B is too small.
                               Default -converge 1200.000000
       -delta_ff <float>       The difference of delta form factor which is smaller than delta_ff
                               will consider as the same.
                               Default -delta_ff 0.000005
       -write_cycle <integer>  Write the status of radiosity for each write_cycle iterations.
                               Default -write_cycle 10
       -triangle <integer>     The maximum the number of triangles in the model. If you show image
                               on WebGL, set -triangle 30000 is the best resolution
                               Defulat -triangle 1000000
       -light <float>          Adjust the scale of bright light for testing.
                               Defulat -light 1.000000
   Output Options
       -zip                    Compress output file by zip.
                               Defulat false
       -o </<path>/file>       Assign the prefix filename the output file
                               Defulat ./test
       -interactive            Transfer the result into WebGL each write_cycle
                               Defulat false

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

