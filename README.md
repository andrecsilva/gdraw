# GDraw

A small collection of mini-libraries and scripts to draw graphs:

`xnumber.hpp` - Template library over the BOOST Graph Library that calculates the crossing number cr(G) of a graph G and "draws" (i.e. output vertices coordinates) graphs. It uses small improvement of the usual naive algorithm.

`util.hpp` - Has a bunch of useful functions. 

`main.cpp` - Reads from stdin a graph in DOT format as a input and outputs to stdout a graph in DOT format. The vertices of the output have the `pos` attribute with the coordinates. The crossing edges also have a `pos` attribute specifying a cubic spline. 

`tikz.py` - A mini-library to produce TikZ drawings. 

`draw.py` - Reads a graph from stdin with the DOT format and outputs a TikZ document for the drawing. Assumes all the vertices of the input have the `pos` attribute. Edges with a `pos` attribute are drawn as a combination of cubic splines.

`draw.sh` - A tiny awk+bash script that draws the graph with the same format as above using neato from Graphviz.

# Build

Make sure you have BOOST and change `LDIR` in the makefile to point to BOOST's location. Just `make`.

# Example of Usage 

```
echo "3" | cat - in_graph.dot | ./main | ./draw.sh > out_drawing.pdf
```

# Example drawing (an optimal drawing of K<sub>6</sub> with 3 crossings)

![alt text][k6drawing]

[k6drawing]: https://github.com/andrecsilva/gdraw/blob/master/example.svg "Optimal drawing of K6"

# FAQ and Known Problems

### Why?

A pet project to get used to BOOST. If you want something more serious: [OGDF](https://ogdf.uos.de/).


### The drawings are ugly.

It uses the Chobak-Payne algorithm (known for its ugliness) to obtain the coordinates.


### The drawings have more crossings than they should.

For now the crossing edges can cross more than they should due to how the controls points are calculated. Should not occur often. 
