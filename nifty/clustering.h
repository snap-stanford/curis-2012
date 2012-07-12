#ifndef clustering_h
#define clustering_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class Clustering {
private:
  PNGraph QGraph;
  LogOutput log;
  TFlt ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB);

public:
  Clustering(LogOutput& log);
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void SetGraph(PNGraph QGraph);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB, TDocBase *DB);
  void SortClustersByFreq(TVec<TTriple<TInt, TInt, TIntV> >& RepQuotesAndFreq, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
};

// Pair comparator
template <class TVal1, class TVal2, class TVal3>
class TCmpTripleByVal2 {
private:
  bool IsAsc;
public:
  TCmpTripleByVal2(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TTriple<TVal1, TVal2, TVal3>& P1,
                    const TTriple<TVal1, TVal2, TVal3>& P2) const {
    if (IsAsc) {
      return P1.Val2 < P2.Val2;
    } else {
      return P2.Val2 < P1.Val2;
    }
  }
};

#endif
