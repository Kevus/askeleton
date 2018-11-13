#!/bin/bash

clang++ -Xclang -ast-dump -fsyntax-only $1 > ast_ejemplo_auxiliar.txt
sed -r "s/\x1B\\[([0-9]{1,2}(;[0-9]{1,2})*)?[mGK]//g" ast_ejemplo_auxiliar.txt > ast_ejemplo_final.txt
