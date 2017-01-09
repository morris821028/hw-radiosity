## README ##

### Build Tool ###

```bash
cd hw-radiosity/demo && make
```

### Converter ###


#### Generate Radiosity Example ####

```bash
mkdir -p hw-radiosity/output
./rad model/church.tri  output/test
```

#### Transfer Triangle Format to Json ####

```bash
cp output/test.fin demo
cd demo && ./tri2json -i test.fin -o test.json --format COLOR
```

#### Show Result by Firefox/Chrome ####

```bash
cp test.json demo/public/asset
# Mac 
open demo/public/index.html
# Ubuntu
firefox `pwd`/public/index.html
```

