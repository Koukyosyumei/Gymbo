prg=$(cat < example/nn.gym)
./gymbo "${prg}" -v 1 -a 0.001 -e 0.000001

