# GDraw

A small collection of mini-libraries and scripts to planarize and draw graphs.

What it does:

* Find drawings of graphs with k crossings or less (naive O(n^k) algorithm with some improvements, see [1]);
* Find embeddings in the projective plane/double planar cover for a given graph (exponential, but somewhat fast);
  * For 3-connected ones, it's possible to list all of them (See [2]);
* Draw graphs using two different methods (Tutte's [3] and Chrobak-Payne via Boost);
* Output the drawings in Tex (Tikz) or Pdf (or more if you're willing to change the parameter in the script);
* A bunch of smaller things not worth mentioning.

# Build

Make sure you have BOOST and change `LDIR` in the makefile to point to BOOST's location. Just `make`.

The `draw.sh` script uses `neato` from graphviz and `awk`, so make sure you have those; `drawTikz.py` uses python3.

# Example of Usage 

```
./xnumber 3 < k6.dot | ./draw.sh > k6.pdf
./finddpc < k6.dot | ./drawTikz.py > k6.tex
./findppembedding < k6.dot
```

# Some Examples (Optimal drawing of K<sub>6</sub> and a double planar cover of it)

![alt text][k6drawing] ![alt text][k6dpc]

[k6drawing]: https://github.com/andrecsilva/gdraw/blob/master/k6opt.svg "Optimal drawing of K6."
[k6dpc]: https://github.com/andrecsilva/gdraw/blob/master/k6dpc.svg "Double planar cover of K6, the red edges are the edges used to generate the cover." 

# FAQ and Known Problems

### Why?

A pet project to test some conjectures and get used to BOOST/C++. If you want something more serious: [OGDF](https://ogdf.uos.de/).

### The drawings are ugly.

I have yet to find a drawing algorithm in the literature that does output "nice" drawings. Tutte's algorithm seems the best I've found.

You should use these drawings as a starting point and then modify them using your favorite program.

# References

[1] Silva, André Carvalho. "Graphs with few crossings and the crossing number of Kp,q in topological surfaces" (2018).

[2] Negami, Seiya. "Enumeration of projective-planar embeddings of graphs." Discrete mathematics 62.3 (1986): 299-306.

[3] Tutte, William Thomas. "How to draw a graph." Proceedings of the London Mathematical Society 3.1 (1963): 743-767.
