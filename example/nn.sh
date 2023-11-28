prg=$(cat < example/nn.gym)
./gymbo "${prg}" -v 1 -a 0.01 -e 0.00000001 -l 0 -h 1

