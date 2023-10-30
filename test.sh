#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 42 "return 42;"
assert 21 "return 5+20-4;"
assert 41 "return 12 + 34 - 5;"
assert 4 "return (3+5)/2;"
assert 1 "return 0<1;"

assert 4 "a = 3; return a + 1;"

assert 11 "a = 1; if (a == 1) return 11; return 17;"
assert 17 "a = 2; if (a == 1) return 11; return 17;"

#assert 42 "b = 42;"
#assert 21 "5+20-4"
#assert 41 " 12 + 34 - 5 "
#assert 47 '5+6*7'
#assert 15 '5*(9-6)'
#assert 4 '(3+5)/2'
#assert 10 '-10+20'
#assert 10 '- -10'
#assert 10 '- - +10'

#assert 0 '0==1'
#assert 1 '42==42'
#assert 1 '0!=1'
#assert 0 '42!=42'

#assert 1 '0<1'
#assert 0 '1<1'
#assert 0 '2<1'
#assert 1 '0<=1'
#assert 1 '1<=1'
#assert 0 '2<=1'

#assert 1 '1>0'
#assert 0 '1>1'
#assert 0 '1>2'
#assert 1 '1>=0'
#assert 1 '1>=1'
#assert 0 '1>=2'

echo OK
