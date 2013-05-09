#!/bin/sh

year=2012
ndays=(31 29 31 30 31 30 31 31 30 31 30 31)
months=(1 2 3 4 5 6)

for month in ${months[*]}
do
  for (( day=1; day <= ${ndays[month-1]}; day++ ))
  do
    yearstr=$(printf "%04d" $year)
    monthstr=$(printf "%02d" $month)
  	daystr=$(printf "%02d" $day)
  	./memefilter -date ${yearstr}-${monthstr}-${daystr} -qbdb /lfs/1/tmp/curis/QBDB/ -spinn3r /lfs/1/tmp/curis/spinn3r/ &
  	[[ $((day%5)) -eq 0 ]] && wait
  done
  wait
done
