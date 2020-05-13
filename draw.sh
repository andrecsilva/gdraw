#!/bin/sh
awk '
match($0,/pos=".+"/){
	scale = 72;
	head = substr($0,0,RSTART-1);
	tail = substr($0,RSTART)
	while(match(tail,/[0-9]+\.?[0-9]*/)){
		n = substr(tail,RSTART,RLENGTH);
		n = n * scale;
		head = head  substr(tail,0,RSTART-1)  n;
		tail = substr(tail,RSTART+RLENGTH)
	}
	print head tail
	next;
}
{print}
' | neato -n2 -Nfixedsize=shape -Nxlabel="\N" -Nshape=point -Tpdf
