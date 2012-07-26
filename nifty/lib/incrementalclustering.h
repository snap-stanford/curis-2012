#ifndef incrementalclustering_h
#define incrementalclustering_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TIncrementalClustering {
private:
  static const TInt DayThreshold;
  static const TInt QuoteThreshold;

public:
  static void BuildClusters(TVec<TIntV>& MergedClusters, TVec<TCluster>& ClusterSummaries,
                     TQuoteBase& QB, TDocBase& DB, TIntV& NewQuotes);
  static void RemoveOldClusters(TVec<TIntV>& NewMergedClusters, TVec<TIntV>& MergedClusters,
                                TQuoteBase& QB, TDocBase& DB, TSecTm PresentTime);

};

#endif
