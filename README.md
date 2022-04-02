# Light File Obfuscator

Simple C program for obfuscating and concealing files by simply "encrypting" them with a [Caesar cipher](en.wikipedia.org/wiki/Caesar_cipher) and renaming them.

### Installation 
Clone reposity then run
```
make install
```
*lfo* program is then located in the *bin* directory. 

### Measure execution time
Included in the project there is an additional program *time_exec* which measures the "encryption" time. Simply put some files in a folder *data* in the project and run
```
make
make copy
./time_exec #
```

where # is the number of times you want to scramble and unscramble the files. 
