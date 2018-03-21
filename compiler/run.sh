#!/bin/bash

num_lines="$(cat libraries.h | wc -l)"

input_file=$1

optimize="0"
CLANG=clang


if echo $* | grep -e "-o" -q
then
    optimize="1"
fi

if echo $* | grep -e "-f" -q
then
    echo ">>>> Running with flag f"
    echo "ENTER PROGRAM <ctrl+D> to stop and print final code"
    input_file="-"
    cat libraries.h $input_file | ./dana $num_lines "stdin" $optimize > a.imm || exit 1
    llc a.imm -o -
    rm a.imm
else
    if echo $* | grep -e "-i" -q
    then
        echo ">>>> Running with flag i" 
        echo "ENTER PROGRAM <ctrl+D> to stop and print indermidiate code"
        input_file="-"
        cat libraries.h $input_file | ./dana $num_lines "stdin" $optimize > a.imm || exit 1
        cat a.imm
    	rm a.imm
    else
        input_base=${input_file%.*}
        cat libraries.h $input_file | ./dana $num_lines $input_file $optimize > $input_base.imm || exit 1
        llc $input_base.imm -o $input_base.asm
        $CLANG $input_base.asm lib.a -o $input_base.out

    fi

fi
