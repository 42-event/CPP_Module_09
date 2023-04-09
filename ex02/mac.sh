#!/bin/zsh
./PmergeMe.out $(jot -r 3000 1 100000 | tr '\n' ' ')
