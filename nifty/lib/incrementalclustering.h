#ifndef incrementalclustering_h
#define incrementalclustering_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TIncrementalClustering : public Clustering {
private:
  static const TInt DayThreshold;
  static const TInt QuoteThreshold;
  TQuoteBase *QB;
  TIntSet NewQuotes;
  TIntSet AffectedNodes;

public:
  TIncrementalClustering(TQuoteBase *QB, TIntSet& NewQuotes, PNGraph QGraph, TIntSet& AffectedNodes);
  virtual ~TIncrementalClustering();
  void KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB);
  static void RemoveOldClusters(TVec<TIntV>& NewMergedClusters, TVec<TIntV>& MergedClusters,
                                TQuoteBase& QB, TDocBase& DB, TSecTm PresentTime);

};

#endif
