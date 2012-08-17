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
  	./memefilter ${yearstr}-${monthstr}-${daystr} &
  	[[ $((day%10)) -eq 0 ]] && wait
  done
  wait
done
