#ifndef clustering_h
#define clustering_h

#include "stdafx.h"
#include "quote.h"

class Clustering {
private:
  PNGraph QGraph;
public:
  void SetGraph(PNGraph QGraph);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB);
};

#endif
