#include "stdafx.h"
#include "edgesdel.h"

const TStr TEdgesDel::PercentEdgesDeleted = "Percent of edges deleted: ";
const TStr TEdgesDel::NumEdgesRemaining = "Number of edges remaining: ";
const TStr TEdgesDel::NumClusters = "Number of clusters: ";
const TStr TEdgesDel::PercentEdgesDeletedNFS = "Percent of edges deleted, not from subgraphs: ";

/** This is the old baseline, when we randomize the graph.
 *  QGraph must be the *original* graph, before edges are deleted.
 *  Just saving this around if we need to use it again
 */
void TEdgesDel::CalcAndLogOldBaseline(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log) {
  // TODO: Repeat this 10 times and take the average
  fprintf(stderr, "generating random graph for comparison\n");
  // Calculate baseline percentage of edges deleted
  PNGraph RandomOrigQGraph;
  RandomOrigQGraph = TSnap::GenRewire(QGraph);
  int NumEdgesOriginal = QGraph->GetEdges();
  IAssert(NumEdgesOriginal == RandomOrigQGraph->GetEdges());

  TIntV AllNIdsV;
  RandomOrigQGraph->GetNIdV(AllNIdsV);
  PNGraph RandomQGraph = TSnap::GetSubGraph(RandomOrigQGraph, AllNIdsV);
  //RandomQGraph = TSnap::GetSubGraph(RandomOrigQGraph, AllNIdsV);

  PNGraph Filler;
  Clustering ClusterJob(Filler);
  ClusterJob.KeepAtMostOneChildPerNode(RandomQGraph, QB, DB, Clustering::ComputeEdgeScore);

  TVec<TIntV> ClustersRandom;
  Clustering::GetAllWCCs(RandomQGraph, ClustersRandom);

  int NumEdgesInducedSubgraphs = 0;
  int NumEdgesInducedSubgraphsFromOrig = 0;
  for (int i = 0; i < ClustersRandom.Len(); i++) {
    PNGraph SubgraphP;
    SubgraphP = TSnap::GetSubGraph(RandomOrigQGraph, ClustersRandom[i]);
    NumEdgesInducedSubgraphs += SubgraphP->GetEdges();
    PNGraph SubgraphFromOrigP = TSnap::GetSubGraph(QGraph, ClustersRandom[i]);
    NumEdgesInducedSubgraphsFromOrig += SubgraphFromOrigP->GetEdges();
  }

  fprintf(stderr, "percent edges deleted, not from subgraphs (BASELINE): %f\n", TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal).Val);
  fprintf(stderr, "percent edges deleted, not from subgraphs in original (BASELINE): %f\n", TFlt(100 - NumEdgesInducedSubgraphsFromOrig * 100.0 / NumEdgesOriginal).Val);
  fprintf(stderr, "percent edges deleted (BASELINE): %f\n", TFlt(100 - RandomQGraph->GetEdges() * 100.0 / NumEdgesOriginal).Val);
  Log.LogValue(LogOutput::PercentEdgesDeletedNFSBaseline, TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal));
  Log.LogValue("PercentEdgesDeletedNFSOrig-Baseline", TFlt(100 - NumEdgesInducedSubgraphsFromOrig * 100.0 / NumEdgesOriginal));
  Log.LogValue("PercentEdgesDeletedBaseline", TFlt(100 - RandomQGraph->GetEdges() * 100.0 / NumEdgesOriginal));
  fprintf(stderr, "random graph generation complete!\n");
}

/** For calculating the percent edges deleted for the actual graph (the one that's passed in)
 *  QGraph must be the *original* graph, before edges are deleted.
 */
void TEdgesDel::CalcForEdgeScore(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log, TStr& ToPrepend,
                                 TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), TInt NumTrials,
                                 TRnd *RandomGenerator) {
  int NumEdgesOriginal = QGraph->GetEdges();
  TIntV AllNIdsV;
  QGraph->GetNIdV(AllNIdsV);
  PNGraph OrigQGraphP = TSnap::GetSubGraph(QGraph, AllNIdsV);

  TFlt AvgPercentEdgesDeletedNFS = 0;
  TFlt AvgPercentEdgesDeleted = 0;
  for (int i = 0; i < NumTrials; i++) {
    fprintf(stderr, "Trial %d: Number of edges in QGraph - %d\n", i, QGraph->GetEdges());
    PNGraph Filler;
    Clustering ClusterJob(Filler);
    PNGraph QGraphCopy = TSnap::GetSubGraph(QGraph, AllNIdsV);
    ClusterJob.KeepAtMostOneChildPerNode(QGraphCopy, QB, DB, Fn, RandomGenerator);

    Log.LogValue(ToPrepend + TEdgesDel::NumEdgesRemaining, TInt(QGraphCopy->GetEdges()));

    TVec<TIntV> Clusters;
    Clustering::GetAllWCCs(QGraphCopy, Clusters);
    Log.LogValue(ToPrepend + TEdgesDel::NumClusters, TInt(Clusters.Len()));

    // calculate percent edges deleted from induced subgraphs of these WCC's
    TInt NumEdgesInducedSubgraphs = 0;
    for (int i = 0; i < Clusters.Len(); i++) {
      PNGraph SubgraphP;
      SubgraphP = TSnap::GetSubGraph(OrigQGraphP, Clusters[i]);
      NumEdgesInducedSubgraphs += SubgraphP->GetEdges();
    }

    AvgPercentEdgesDeletedNFS += TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal);
    AvgPercentEdgesDeleted += TFlt(100 - QGraphCopy->GetEdges() * 100.0 / NumEdgesOriginal);
  }

  AvgPercentEdgesDeletedNFS /= NumTrials;
  AvgPercentEdgesDeleted /= NumTrials;

  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgesDeletedNFS, AvgPercentEdgesDeletedNFS);
  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgesDeleted, AvgPercentEdgesDeleted);
}

void TEdgesDel::CalcAndLogPercentEdgesDel(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log) {
  Log.LogValue("Original number of edges: ", TInt(QGraph->GetEdges()));
  Log.LogValue("Original number of nodes: ", TInt(QGraph->GetNodes()));

  TIntV AllNIdsV;
  QGraph->GetNIdV(AllNIdsV);
  PNGraph QGraphCopy = TSnap::GetSubGraph(QGraph, AllNIdsV);
  PNGraph QGraphCopy2 = TSnap::GetSubGraph(QGraph, AllNIdsV);

  TStr ToPrepend("(Clustering::ComputeEdgeScore) ");
  fprintf(stderr, "Trial %d: Number of edges in QGraphCopy - %d\n", 0, QGraphCopy->GetEdges());
  CalcForEdgeScore(QB, DB, QGraphCopy, Log, ToPrepend, Clustering::ComputeEdgeScore);

  TStr ToPrepend2("(Clustering::ComputeEdgeScoreRandom) ");
  fprintf(stderr, "Trial %d: Number of edges in QGraphCopy2 - %d\n", 0, QGraphCopy2->GetEdges());
  CalcForEdgeScore(QB, DB, QGraphCopy, Log, ToPrepend, Clustering::ComputeEdgeScore);
  PNGraph Filler;
  Clustering ClusterJob(Filler);
  int RandomSeed = (int) ((TSecTm::GetCurTm()).GetAbsSecs() % TInt::Mx);
  TRnd RandomGenerator(RandomSeed);
  CalcForEdgeScore(QB, DB, QGraphCopy2, Log, ToPrepend2, Clustering::ComputeEdgeScoreRandom, 100, &RandomGenerator);
}
