#ifndef clustering_h
#define clustering_h

#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "logoutput.h"

class Clustering {
protected:
  THash<TInt, TInt> visit;
  THash<TIntPr, TFlt> EdgeScores;
  THash<TIntPr, TIntPr> EdgeToRandomEdge;

  PNGraph QGraph;
  LogOutput log;
  int GetCluster(TInt CurNode, PNGraph& G, TQuoteBase *QB, TDocBase *DB,
                 TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), bool ConstantEdgeScore = false,
                 bool RandomEdgeScore = false);
  void PrepForEdgeScoreBaseline(TQuoteBase *QB, TDocBase *DB);

public:
  Clustering(PNGraph QGraph, TQuoteBase *QB = NULL, TDocBase *DB = NULL);
  virtual ~Clustering();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void GetRootNodes(TIntSet& RootNodes);
  void BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TSecTm& PresentTime, TStr method, bool CheckEdgesDel = false);
  void BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TSecTm& PresentTime, TClusterBase *OldCB, TStr method, bool CheckEdgesDel = false);
  static TInt CalcRepresentativeQuote(TQuote& RepQuote, TIntV& Cluster, TQuoteBase *QuoteBase);
  //static void SortClustersByFreq(TVec<TCluster>& ClusterSummaries, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase);
  virtual void KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB);
  virtual void KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB, TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator),
                                         TRnd *RandomGenerator = NULL);
  void IncrementalEdgeDeletion(PNGraph& G, TQuoteBase *QB, TDocBase *DB, bool ConstantEdgeScore = false);
  void IncrementalEdgeDeletion(PNGraph& G, TQuoteBase *QB, TDocBase *DB,
                               TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), bool ConstantEdgeScore = false,
                               bool RandomEdgeScore = false);
  static void GetAllWCCs(const PNGraph& G, TVec<TIntV>& Clusters);
  static TFlt ComputeEdgeScoreGeneral(TQuote& Source, TQuote& Dest, TDocBase *DB, TStr Type, TRnd *RandomGenerator = NULL);
  static TFlt ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
  static TFlt ComputeEdgeScoreRandom(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
  static TFlt ComputeEdgeScoreWeighted(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
  static TFlt ComputeEdgeScoreOld(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
  TFlt ComputeEdgeScoreBaseline(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator = NULL);
};

#endif
