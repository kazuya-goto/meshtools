#!/bin/sh
nerr=0
ntest=0

do_test() {
    num=$1
    cmd=$2
    arg=$3
    ref=$4
    maxdif=$5
    ntest=$((ntest+1))
    ../$cmd $arg > tmp.out || (echo TEST-$num FAILED; nerr=$((nerr+1)); exit)
    ndiff=`diff $ref tmp.out | wc -l`
    if [ $ndiff -gt $maxdif ]; then
	echo TEST-$num FAILED
        nerr=$((nerr+1))
    fi
    rm -f tmp.out
}

do_test 1 sd342to341 A342.msh A342-sd341.msh 4
do_test 2 rf341to342 A341.msh A341-rf342.msh 4
do_test 3 fstr2adv A341.msh A341-adv.msh 0
do_test 4 fstr2adv A342.msh A342-adv.msh 0
do_test 5 meshcount A341.msh A341-mc.out 0
do_test 6 meshcount A342.msh A342-mc.out 0

if [ $nerr -gt 0 ]; then
    echo "SUMMARY: $nerr of $ntest TEST(S) FAILED"
else
    echo "SUMMARY: ALL $ntest TESTS PASSED"
fi
