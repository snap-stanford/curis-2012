#ifndef stdafx_h
#define stdafx_h

// ###################################################################################################
// ## THIS SECTION OF CODE IS UNIQUE TO EACH PROGRAM. PLEASE CHANGE TO SUIT YOUR NEEDS.
// ###################################################################################################

#include "../../snap/snap-core/Snap.h"
#include "lib/niftylib.h"

// this where daily spinn3r data is stored.
#define SPINNER_DIR_DEFAULT "/lfs/1/tmp/curis/spinn3r/"

// this where filtered output for each day is stored. this includes the quote base and doc base.
// (output for memefilter.cpp)
#define QBDB_DIR_DEFAULT "/lfs/1/tmp/curis/QBDB/"

// this where binary output for each day is stored. this includes the quote base, cluster base,
// doc base, and cluster graph. all information stored in the bases are still "live" on their
// given dates, i.e. the cluster is still being updated nicely.
// (output for memeseed and memeincrement.cpp)
#define QBDBC_DIR_DEFAULT "/lfs/1/tmp/curis/QBDBC/"

// if you start up a new instance of NIFTY (i.e. you're probably running memeseed from scratch again), this is where
// all text or web based output is stored. the system will create a new time stamped folder in this directory.
// (used in lib/logoutput.cpp). Alternatively (and recommended), you can specify your own directory using
// the -directory <directoryname> flag.
#define WEB_DIR_DEFAULT "../../public_html/curis/output/clustering/"


// this variable is where json output will be stored by default if nothing is defined.
// it makes me sad and will hopefully be eventually phased out. this variable isn't even being used (?)
#define JSON_DIR_DEFAULT "../../../public_html/curis/output/clustering/webdata/"


// ##################### EXPERIMENTAL OPTIONS ###########################
#define USE_LSH true

// ###################################################################################################
// ## ANY CODE PAST THIS LINE IS THE SAME FOR EVERY COPY OF THE PROGRAM AND SHOULD NOT BE CHANGED
// ## UNLESS YOU ABSOLUTELY KNOW WHAT YOU'RE DOING. :)
// ###################################################################################################

// default bucket size considered when calculating the peak and displaying individual cluster plots.
// expressed in hours. defaults to 2.
#define PEAK_BUCKET 2
// default sliding window size considered when calculating the peak and displaying individual cluster plots.
// expressed in hours. defaults to 1.
#define PEAK_WINDOW 1

// shortcut method for fprintf to stderr, becuase fprintf is hard to type.
#define Err(format, args...) fprintf(stderr, format, ## args)
#endif
