#!/bin/sh
date=1
while [ $date -lt 10 ]
do
  ./memefilter 2012-01-0${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
date=10
while [ $date -lt 32 ]
do
  ./memefilter 2012-01-${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done

date=1
while [ $date -lt 10 ]
do
  ./memefilter 2012-02-0${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
date=10
while [ $date -lt 30 ]
do
  ./memefilter 2012-02-${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done

date=1
while [ $date -lt 10 ]
do
  ./memefilter 2012-03-0${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
date=10
while [ $date -lt 32 ]
do
  ./memefilter 2012-03-${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done

date=1
while [ $date -lt 10 ]
do
  ./memefilter 2012-04-0${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
date=10
while [ $date -lt 31 ]
do
  ./memefilter 2012-04-${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done

date=1
while [ $date -lt 10 ]
do
  ./memefilter 2012-05-0${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
date=10
while [ $date -lt 32 ]
do
  ./memefilter 2012-05-${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done

date=1
while [ $date -lt 10 ]
do
  ./memefilter 2012-06-0${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
date=10
while [ $date -lt 31 ]
do
  ./memefilter 2012-06-${date} &
  date=$(( $date + 1 ))
  [[ $((date%10)) -eq 0 ]] && wait
done
