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
  void SortClustersByFreq(TVec<TPair<TPair<TInt, TInt>, TIntV> >& RepQuotesAndFreq, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
};

// Pair comparator
template <class TVal1, class TVal2, class TVal3>
class TCmpPairByVal2OfVal1 {
private:
  bool IsAsc;
public:
  TCmpPairByVal2OfVal1(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TPair<TPair<TVal1, TVal2>, TVal3>& P1,
                    const TPair<TPair<TVal1, TVal2>, TVal3>& P2) const {
    if (IsAsc) {
      return P1.Val1.Val2 < P2.Val1.Val2;
    } else {
      return P2.Val1.Val2 < P1.Val1.Val2;
    }
  }
};

#endif
