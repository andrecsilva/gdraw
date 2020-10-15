# GDraw

A small collection of mini-libraries and scripts to planarize and draw graphs.

What it does:

* Find drawings of graphs with k crossings or less (naive roughly O(n^k) algorithm with some improvements, see [1]);
* Find embeddings in the projective plane/double planar cover for a given graph (exponential, but somewhat fast);
  * For 3-connected ones, it's possible to list all of them (See [2]);
* Draw graphs using two different methods (Tutte's [3] and Chrobak-Payne via Boost);
* Output the drawings in Tex (Tikz) or Pdf (or more if you're willing to change the parameter in the script);
* A bunch of smaller things not worth mentioning.

# Build

Make sure you have [BOOST](https://www.boost.org/) and [cppitertools](https://github.com/ryanhaining/cppitertools). Change `LDIR` in the makefile to point to your system's library paths. Just `make`.

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

# Using the Library

### BOOST Graphs and Indexes

Most of the graph types used in the library are templated wrappers around BOOST's own graph types. They all have copy and move semantics imbued into them.

You're free to use any of BOOST's graph types as long as it has `vertex_index` and `edge_index` as internal properties.

### Functions

Most of the functions in the library uses the pass-by-value-then-move idiom. Those that return graphs will move the (possibly modified) graph parameter to its returned graph.

For example, calling `planeEmbedding(g) -> std::variant<PlanarGraph,NonPlanarGraph>` will copy `g` and the copy will be moved into the returned variant. If instead you do `auto v = planeEmbedding(std::move(g))`, then `g`'s internal structure will be moved into `v`.

You really want to use move semantics as much as possible, as copy operations are quite expensive in comparison.

### Move/Copy Semantics and Edge Descriptors

BOOST's edge descriptors (`edge_t` in the library) have a pointer to the edge's location in the graph internal structures. This means that if you copy a graph, any edge descriptor will continue to point to the original graph and should not be used with the copy.

Any internal edge descriptors used in the members of any graph types (e.g. `PlanarGraph`) are automatically updated in the copy operation itself. 



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
