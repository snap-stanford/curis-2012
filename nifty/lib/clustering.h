#ifndef clustering_h
#define clustering_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class Clustering {
protected:
  PNGraph QGraph;
  LogOutput log;
  static TFlt ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB);
  virtual void KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB);
  static void GetAllWCCs(PNGraph& G, TVec<TIntV>& Clusters);

public:
  Clustering(PNGraph QGraph);
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void GetRootNodes(TIntSet& RootNodes);
  virtual void BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TClusterBase *OldCB);
  static TInt CalcRepresentativeQuote(TQuote& RepQuote, TIntV& Cluster, TQuoteBase *QuoteBase);
  //static void SortClustersByFreq(TVec<TCluster>& ClusterSummaries, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
};

#endif
