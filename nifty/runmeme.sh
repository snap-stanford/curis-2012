#!/bin/sh
begdate=`/bin/date --date=yesterday '+%Y-%m-%d'`
enddate=`/bin/date '+%Y-%m-%d'`
./memefilter -date ${begdate} -qbdb /lfs/1/tmp/curis/QBDB/ -spinn3r /lfs/1/tmp/curis/spinn3r/
./memeincrement -start ${begdate} -end ${enddate} -qbdb /lfs/1/tmp/curis/QBDB/ -qbdbc /lfs/1/tmp/curis/QBDBC/ -director outputdir

