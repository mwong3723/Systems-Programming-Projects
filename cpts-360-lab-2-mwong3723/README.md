[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/pqSvN8XG)
#### CPTS 360 Lab 2

Base code and documentation for WSU CPTS360 Lab 2 (Building a Unix Shell)


**Files:**

`Makefile`	      _Compiles your shell program and runs the tests_

`tsh.c`		        _The shell program you will write and hand in_

`tshref`		      _The reference shell binary_



**The remaining files are used to test your shell**

`sdriver.pl`	    _The trace-driven shell driver_

`trace*.txt`	    _The 15 trace files that control the shell driver_


**Little C programs that are called by the trace files**

`myspin.c`	      _Takes argument `<n>` and spins for `<n>` seconds_

`mysplit.c`	      _Forks a child that spins for `<n>` seconds_

`mystop.c`        _Spins for `<n>` seconds and sends SIGTSTP to itself_

`myint.c`         _Spins for `<n>` seconds and sends SIGINT to itself_

