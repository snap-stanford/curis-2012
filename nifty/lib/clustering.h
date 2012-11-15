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

public:
  Clustering(PNGraph QGraph);
  virtual ~Clustering();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TSecTm& PresentTime, bool CheckEdgesDel = false);
  void BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TSecTm& PresentTime, TClusterBase *OldCB, bool CheckEdgesDel = false);
  static TInt CalcRepresentativeQuote(TQuote& RepQuote, TIntV& Cluster, TQuoteBase *QuoteBase);
  //static void SortClustersByFreq(TVec<TCluster>& ClusterSummaries, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
  virtual void KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB, TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator),
                                         TRnd *RandomGenerator = NULL);
  static void GetAllWCCs(PNGraph& G, TVec<TIntV>& Clusters);
  static TFlt ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
  static TFlt ComputeEdgeScoreRandom(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
};

#endif
