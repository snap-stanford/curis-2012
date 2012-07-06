#ifndef clustering_h
#define clustering_h

#include "stdafx.h"
#include "quote.h"

class Clustering {
private:
  PNGraph QGraph;
  class TClusterCompareByFreq {
  public:
    TClusterCompareByFreq() { }
    int operator () (const TPair<TStr, TInt>& First, const TPair<TStr, TInt>& Second) const {
      if (First.Val1 < Second.Val1) {
        return -1;
      } else if (First.Val1 == Second.Val1) {
        return 0;
      } else {
        return 1;
      }
    }
  };

public:
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void SetGraph(PNGraph QGraph);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB);
  void SortClustersByFreq(TVec<TPair<TStr, TInt> >& RepQuotesAndFreq, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
};

#endif
