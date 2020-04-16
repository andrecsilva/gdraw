import sys
from itertools import chain

class Node:

    count=0;

    def __init__(self,coordinate,name=None,style='draw,circle,fill,radius=0.5pt,scale=0.2'):
        if name is None:
            self.name = f'v{self.count}'
        else:
            self.name = name
        self.coordinate = coordinate
        self.style = style
        Node.count +=1

    def nodeCommand(self):
        return f'\\node[{self.style}] at ({self.name}) {{}};'
    
    def coordinateCommand(self):
        return f'\\coordinate ({self.name}) at {self.coordinate};'

class Path:

    def __init__(self,node,style='draw'):
        self.style=style
        self.command = [f'\path[{style}] ({node.name})']

    def to(self,node,type='to',style=''):
        if style != '':
            style = '[' + style + ']'
        if isinstance(node, Node):
            node = "(" + node.name + ")"
        self.command.append(f'{type}{style} {node}')
        return self

    def close(self,type='to',style=''):
        if style == '':
            self.command.append(f'{type} cycle')
        else:
            self.command.append(f'{type}[{style}] cycle')
        return self

    def compile(self):
        return ' '.join(chain(self.command,[';']))

    def curveTo(self,node,control1,control2=None):
        if isinstance(node, Node):
            node = "(" + node.name + ")"
        if control2 is None:
            self.command.append(f' .. controls {control1} .. {node}')
        else:
            self.command.append(f' .. controls {control1} and {control2} .. {node}')
        return self

        
class Figure:


    def __init__(self):
        self.coordinates = []
        self.paths = []
        self.nodes = []

    def add(self,p):
        lines.extend(p.compile())

    def addCoordinate(self,coordinate):
        self.coordinates.append(node.coordinateCommand())

    def addNode(self,node):
        self.coordinates.append(node.coordinateCommand())
        self.nodes.append(node.nodeCommand())

    def preamble(self):
        return [\
        "\documentclass[tikz]{standalone}"\
        ,"\\usepackage{tikz}"\
        ,"\\begin{document}"\
        ,"\\begin{tikzpicture}[]"\
        ]

    def epilogue(self):
        return [\
        "\end{tikzpicture}"\
        ,"\end{document}"\
        ]

    def addPath(self,path):
        self.paths.append(path.compile())

    def compile(self):
        return '\n'.join(chain(self.preamble(),self.coordinates,self.paths,self.nodes,self.epilogue()))

def main():
    #a circular drawing of V_8
    k = 8
    points = [(math.cos(i*(2*math.pi)/k),math.sin(i*2*math.pi/k)) for i in range(k)]

    f = Figure()
    h,*tail = [Node(p) for p in points]

    f.addNode(h)
    rim = Path(h)
    for n in tail:
        f.addNode(n)
        rim.to(n)
    rim.close()
    f.addPath(rim)

    #spokes
    f.addPath(Path(h).to(tail[3]))
    for i in range(1,k//2):
        f.addPath(Path(tail[i-1]).to(tail[i-1+k//2]))
    
    with open('test.tex','w') as texfile:
        texfile.write(f.compile())

if __name__ == "__main__":
    main()
