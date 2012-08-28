#
# Quote pertaining to Whitney Houston's death (Fri Aug 24 16:40:21 2012)
#

#set title "Quote pertaining to Whitney Houston's death"
set key bottom right
set autoscale
set terminal png small size 1000,500
set output 'whitney_plot.png'
unset ytics
set xrange [0:19]
set yrange [2.5:10.5]
set grid xtics
set xtics nomirror ("a" 0.5, "small" 1.5, "spoon" 2.5, "with" 3.5, "a" 4.5, "white" 5.5, "crystal" 6.5, "like" 7.5, "substance" 8.5, "in" 9.5, "it" 10.5, "and" 11.5, "a" 12.5, "rolled" 13.5, "up" 14.5, "piece" 15.5, "of" 16.5, "white" 17.5, "paper" 18.5)
plot 	"whitney_plot.tab" using 1:2:3:4 title "" with boxxyerrorbars fill solid 1
#lc rgb "black"
