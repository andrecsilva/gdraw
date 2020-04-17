#!/usr/bin/env python3
import sys
import pydot

from tikz import Figure
from tikz import Node
from tikz import Path

def posToList(s):
    f = lambda s: tuple([float(k) for k in s.split(',')])
    return [f(i) for i in s[1:-1].split(' ')]

def tikzFromPos(g):
    f = Figure()
    tikzNodes = dict()
    for n in g.get_nodes():
        tn = Node(posToList(n.get_attributes()['pos'])[0], n.get_name())
        tikzNodes[n.get_name()] = tn
        f.addNode(tn)

    #Graphviz pos attribute requires 3n+1 points, drawing a cubic splines for each triple of points
    for e in g.get_edges():
        p = Path(tikzNodes[e.get_source()])
        if 'pos' in e.get_attributes():
            controls = posToList(e.get_attributes()['pos'])
            p.to(controls[0])
            for i in range(1, len(controls), 3):
                p.curveTo(controls[i+2], controls[i], controls[i+1])
        p.to(tikzNodes[e.get_destination()])
        f.addPath(p)
    return f

def main():
    with sys.stdin as dot:
        g = pydot.graph_from_dot_data(dot.read())
    print(tikzFromPos(g[0]).compile())

if __name__ == "__main__":
    main()
