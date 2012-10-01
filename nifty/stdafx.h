#ifndef stdafx_h
#define stdafx_h

#include "../../snap/snap-core/Snap.h"
#include "lib/niftylib.h"

#define Err(format, args...) fprintf(stderr, format, ## args)

#define TWITTER_BOOTSTRAP "../../../../bootstrap/css/bootstrap.css"
#define TWITTER_BOOTSTRAP2 "../../../../../bootstrap/css/bootstrap.css"
#define TWITTER_BOOTSTRAP_JS "../../../../../bootstrap/js/bootstrap.js"

#define PEAK_BUCKET 2
#define PEAK_WINDOW 1

#define LOG_LEVEL 3
#define Print(level, format, args...) if(format<=LOG_LEVEL){fprintf(stderr, format, ## args)}

#endif
