g++ example/randomized/query.cpp -O3
prg=$(cat < example/randomized/query.gym)
./a.out "${prg}"
