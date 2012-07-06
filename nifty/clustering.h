#ifndef clustering_h
#define clustering_h

#include "stdafx.h"
#include "quote.h"

class Clustering {
private:
  PNGraph QGraph;

public:
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void SetGraph(PNGraph QGraph);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB);
  void SortClustersByFreq(TVec<TPair<TStr, TInt> >& RepQuotesAndFreq, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
};

#endif
