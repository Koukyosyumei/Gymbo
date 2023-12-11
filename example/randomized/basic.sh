g++ example/randomized/basic.cpp -O3
prg=$(cat < example/randomized/basic.gym)
./a.out "${prg}"
