prg=$(cat < example/nn.gym)
./gymbo "${prg}" -v 2 -a 0.001 -e 0.000001

