
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

assert 6 "1+2+3;"
assert 1 "1;"
assert 8 "2* (4+1-1);"
assert 0 "42!=42;"
assert 1 "3 >= 2;"
assert 1 "-2------3;"
# 保留最后一条语句的结果
assert 3 "1; 2; 3;"
assert 12 "12+23; 12+99/3; 78-66;"

echo OK
