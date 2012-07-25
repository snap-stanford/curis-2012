#ifndef incrementalclustering_h
#define incrementalclustering_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TIncrementalClustering {
private:

public:
  static void BuildClusters(TVec<TIntV>& MergedClusters, TVec<TCluster>& ClusterSummaries,
                     TQuoteBase& QB, TDocBase& DB, TIntV& NewQuotes);

};

#endif
