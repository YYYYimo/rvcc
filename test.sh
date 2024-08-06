
assert()
{
    expected="$1"
    input="$2"

    ./rvcc "$input" > tmp.s || exit

    riscv64-unknown-linux-gnu-gcc -static -o tmp tmp.s

    ~/qemu-6.2.0/build/qemu-riscv64 -L $RISCV/sysroot tmp

    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# assert 6 "return 1+2+3;"
# assert 1 "1; return 1;"
# assert 8 "return 2* (4+1-1);"
# assert 0 "return 42!=42;"
# assert 1 "return 3 >= 2;"
# assert 1 "return -2------3;"
# 保留最后一条语句的结果
# assert 3 "1; 2; return 3;"
# assert 12 "12+23; 12+99/3; return 78-66;"

# [10] 支持单字母变量
# assert 3 "a = 3; return a;"
# assert 8 "a= 3; z = 5; return a + z;"
# assert 6 "a=b=3; return a+b;"
# assert 5 "a=3;b=4;a=1; return a+b;"

# [11] 支持多字母变量
# assert 3 "foo = 3; return foo;"
# assert 74 "foo2=70; bar4=4; a=foo2+bar4; return a;"

# [12]支持return语句
# assert 0 "return 0;"
# assert 42 "return 42;"
# assert 41 "return 43 + 8 - 10;"

# [13] 支持块语句
assert 1 "{ return 1; 2; 3;}"
assert 2 "{1; return 2; 3;}"
assert 3 "{1; 2; return 3;}"
assert 3 "{ {1; {2;} return 3;}}"

# [14] 支持空语句
assert 5 "{;;;return 5;}"

# [15] 支持if语句
assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'

# [16] 支持for语句
assert 55 '{ i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 '{ for (;;) {return 3;} return 5; }'

# [17] 支持while语句
assert 10 '{ i=0; while(i<10) { i=i+1; } return i; }'


echo OK
