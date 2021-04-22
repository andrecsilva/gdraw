#!/bin/sh
#When using the -n2 option, neato ignores any kind of scales (-s or inputscale parameter).
#The following awk script essentially scale the points.
awk '
match($0,/pos="[^"]*"/){
	scale = 128
	head = substr($0,0,RSTART-1)
	tail = substr($0,RSTART,RLENGTH)
	rest = substr($0,RSTART+RLENGTH)
	while(match(tail,/[0-9]+\.?[0-9]*/)){
		n = substr(tail,RSTART,RLENGTH)
		n = n * scale
		head = head  substr(tail,0,RSTART-1)  n
		tail = substr(tail,RSTART+RLENGTH)
	}
	print head tail rest
	next
}
{print}
' | neato -n2 -Nxlabel="\N" -Nshape=point -Tpdf
