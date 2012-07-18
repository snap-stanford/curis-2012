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
  static TFlt ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB);
  static void KeepAtMostOneChildPerNode(PNGraph& G, TIntSet& RootNodes, TQuoteBase *QB, TDocBase *DB);
  static void GetAllWCCs(PNGraph& G, TVec<TIntV>& Clusters);

public:
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void SetGraph(PNGraph QGraph);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB, TDocBase *DB, LogOutput& log);
  void SortClustersByFreq(TVec<TCluster>& ClusterSummaries, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
};

#endif

// GEE GEE GEE GEE BABY BABY BABY
