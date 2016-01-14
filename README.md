# cube-puzzle

I have a 4x4x4 wooden cube puzzle at home, which I discovered later is
called a [Snake Cube](https://en.wikipedia.org/wiki/Snake_cube). The
3x3x3 ones are pretty easy, but I got frustrated after a while trying
to solve the 4x4x4 one, so I decided to write a program to find a
solution.

All of the implementations use the same algorithm, a simple
[backtracking](https://en.wikipedia.org/wiki/Backtracking) algorithm
that searches the entire solution space. The C implementation is
orders of magnitude faster (literally milliseconds instead of minutes
in my testing), but that's partially because I implemented the
backtracking differently: instead of creating new state objects at
each level of depth, the same is reused and backtracked, and the small
size of the structure leads to excellent cache performance on the CPU.

Since it was super fast, I also modified the C solution to find all
solutions, not stopping after the first one it encounters. It still
runs in just a few seconds, which is crazy.

One of the other interesting things I discovered is that it's much
faster to find a solution starting on one end of the chain rather than
the other; this is because there are lots more combinations to look at
on the end that has more segments of length 2. This is only relevant
for the python and scala solutions (since they stop after finding a
single solution rather than exploring the entire space). But you'll
notice that the chain length array is reversed between python and
scala, because in scala using head/tail is more natural (pulling
elements off of the front), while in python pop() is more natural
(pulling elements off of the end).

## running the python version

Tested with python 2.7:

```
$ python cube_puzzle.py
```

This will take a number of minutes, then output a solution:

```
solution:
+x (3)
+y (2)
-x (3)
+y (2)
+x (2)
[...]
```

The intent is that +x/-x is left/right, +y/-y is up/down, and +z/-z is
forward/back, but obviously any assignment of the axes and directions
will work. The numbers show the chain length at that spot, which I
found helpful to keep my place as I was applying the solution to the puzzle.

# scala version

Tested with scala 2.10:
```
scalac cube_puzzle.scala
scala org.danob.puzzle.Main
```

Again, this will take a number of minutes and output a solution
similar to above.

# c version

Tested on Mac OS 10.11 with the XCode command-line utilities installed:
```
gcc -O3 -o cube-puzzle cube-puzzle.c
./cube-puzzle
```
