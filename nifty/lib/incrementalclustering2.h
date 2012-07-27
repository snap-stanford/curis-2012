#ifndef incrementalclustering2_h
#define incrementalclustering2_h

#include "quote.h"

class IncrementalClustering2 : public Clustering {
public:
  IncrementalClustering2(TQuoteBase *QB, TIntSet& NewQuotes, PNGraph QGraph, TIntSet& AffectedNodes);
  void KeepAtMostOneChildPerNode(PNGraph& G, TIntSet& RootNodes, TQuoteBase *QB, TDocBase *DB);

private:
  TQuoteBase *QB;
  TIntSet NewQuotes;
  TIntSet AffectedNodes;
};

#endif
