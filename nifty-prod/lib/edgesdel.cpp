#include "stdafx.h"
#include "edgesdel.h"

const TStr TEdgesDel::PercentEdgesDeleted = "Percent of edges deleted: ";
const TStr TEdgesDel::NumEdgesRemaining = "Number of edges remaining: ";
const TStr TEdgesDel::NumClusters = "Number of clusters: ";
const TStr TEdgesDel::PercentEdgesDeletedNFS = "Percent of edges deleted, not from subgraphs: ";
const TStr TEdgesDel::PercentEdgeScoreDeletedNFS = "Percent of edge score deleted, not from subgraphs: ";
const TStr TEdgesDel::PercentEdgeScoreDeleted = "Percent of edge score deleted: ";

const TStr TEdgesDel::IncEdgeDelMaxNumEdges = "(Clustering::IncrementalEdgeDeletion_max-num-edges) ";
const TStr TEdgesDel::IncEdgeDelMaxEdgeScore = "(Clustering::IncrementalEdgeDeletion_max-edge-score) ";

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

/** Returns the sum of the edge scores in the graph */
TFlt TEdgesDel::CalcSumEdgeScoreOfGraph(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph) {
  TFlt SumEdgeScore;
  for (TNGraph::TEdgeI Edge = QGraph->BegEI(); Edge != QGraph->EndEI(); Edge++) {
    TQuote SourceQuote;
    QB->GetQuote(Edge.GetSrcNId(), SourceQuote);
    TQuote DestQuote;
    QB->GetQuote(Edge.GetDstNId(), DestQuote);
    SumEdgeScore += Clustering::ComputeEdgeScore(SourceQuote, DestQuote, DB);
  }
  return SumEdgeScore;
}

/** Calculates the percent edges deleted and percent edge score deleted, updating the
 *  TFlt parameter values.
 */
void TEdgesDel::UpdatePercentages(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, const PNGraph& QGraphPartitioned,
                                TFlt& AvgPercentEdgeScoreDeletedNFS, TFlt& AvgPercentEdgeScoreDeleted,
                                TFlt& AvgPercentEdgesDeletedNFS, TFlt& AvgPercentEdgesDeleted, LogOutput& Log, const TStr& ToPrepend) {

  Log.LogValue(ToPrepend + TEdgesDel::NumEdgesRemaining, TInt(QGraphPartitioned->GetEdges()));
  int NumEdgesOriginal = QGraph->GetEdges();
  TFlt SumEdgeScoreOriginal = CalcSumEdgeScoreOfGraph(QB, DB, QGraph);

  TVec<TIntV> Clusters;
  Clustering::GetAllWCCs(QGraphPartitioned, Clusters);
  Log.LogValue(ToPrepend + TEdgesDel::NumClusters, TInt(Clusters.Len()));

  TFlt SumEdgeScoreInducedSubgraphs;

  // calculate percent edges deleted from induced subgraphs of these WCC's
  // also calculate percent edge score deleted from whole graph vs. induced subgraphs
  TInt NumEdgesInducedSubgraphs = 0;
  for (int i = 0; i < Clusters.Len(); i++) {
    PNGraph SubgraphP;
    SubgraphP = TSnap::GetSubGraph(QGraph, Clusters[i]);
    NumEdgesInducedSubgraphs += SubgraphP->GetEdges();
    SumEdgeScoreInducedSubgraphs += CalcSumEdgeScoreOfGraph(QB, DB, SubgraphP);
  }

  TFlt SumEdgeScore = CalcSumEdgeScoreOfGraph(QB, DB, QGraphPartitioned);

  AvgPercentEdgeScoreDeletedNFS += TFlt(100 - SumEdgeScoreInducedSubgraphs * 100.0 / SumEdgeScoreOriginal);
  AvgPercentEdgeScoreDeleted += TFlt(100 - SumEdgeScore * 100.0 / SumEdgeScoreOriginal);

  AvgPercentEdgesDeletedNFS += TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal);
  AvgPercentEdgesDeleted += TFlt(100 - QGraphPartitioned->GetEdges() * 100.0 / NumEdgesOriginal);
}

/** For calculating the percent edges deleted for the actual graph (the one that's passed in)
 *  QGraph must be the *original* graph, before edges are deleted.
 */
void TEdgesDel::TryPartitioningMethod(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, LogOutput& Log, const TStr& ToPrepend,
                                 TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), TInt NumTrials,
                                 TRnd *RandomGenerator) {
  TIntV AllNIdsV;
  QGraph->GetNIdV(AllNIdsV);

  TFlt AvgPercentEdgesDeletedNFS, AvgPercentEdgesDeleted, AvgPercentEdgeScoreDeletedNFS, AvgPercentEdgeScoreDeleted;
  for (int i = 0; i < NumTrials; i++) {
    fprintf(stderr, "Trial %d: Number of edges in QGraph - %d\n", i, QGraph->GetEdges());
    PNGraph QGraphPartitioned = TSnap::GetSubGraph(QGraph, AllNIdsV);
    Clustering ClusterJob(QGraphPartitioned);

    if (ToPrepend == TEdgesDel::IncEdgeDelMaxEdgeScore) {
      fprintf(stderr, "Doing incremental edge deletion, choosing cluster with max edge score\n");
      ClusterJob.IncrementalEdgeDeletion(QGraphPartitioned, QB, DB, false);
    } else if (ToPrepend == TEdgesDel::IncEdgeDelMaxNumEdges) {
      fprintf(stderr, "Doing incremental edge deletion, choosing cluster with max num edges\n");
      ClusterJob.IncrementalEdgeDeletion(QGraphPartitioned, QB, DB, true);
    } else {
      ClusterJob.KeepAtMostOneChildPerNode(QGraphPartitioned, QB, DB, Fn, RandomGenerator);
    }

    UpdatePercentages(QB, DB, QGraph, QGraphPartitioned, AvgPercentEdgeScoreDeletedNFS, AvgPercentEdgeScoreDeleted,
                      AvgPercentEdgesDeletedNFS, AvgPercentEdgesDeleted, Log, ToPrepend);
  }

  AvgPercentEdgeScoreDeletedNFS /= NumTrials;
  AvgPercentEdgeScoreDeleted /= NumTrials;

  AvgPercentEdgesDeletedNFS /= NumTrials;
  AvgPercentEdgesDeleted /= NumTrials;

  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgesDeletedNFS, AvgPercentEdgesDeletedNFS);
  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgesDeleted, AvgPercentEdgesDeleted);

  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgeScoreDeletedNFS, AvgPercentEdgeScoreDeletedNFS);
  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgeScoreDeleted, AvgPercentEdgeScoreDeleted);
}

void TEdgesDel::ComparePartitioningMethods(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log) {
  Log.LogValue("Original number of edges: ", TInt(QGraph->GetEdges()));
  Log.LogValue("Original number of nodes: ", TInt(QGraph->GetNodes()));

  TStr ToPrepend("(Clustering::ComputeEdgeScore) ");
  TryPartitioningMethod(QB, DB, QGraph, Log, ToPrepend, Clustering::ComputeEdgeScore);
  TryPartitioningMethod(QB, DB, QGraph, Log, TEdgesDel::IncEdgeDelMaxEdgeScore, Clustering::ComputeEdgeScore);
  TryPartitioningMethod(QB, DB, QGraph, Log, TEdgesDel::IncEdgeDelMaxNumEdges, Clustering::ComputeEdgeScore);

  TStr ToPrepend4("(Clustering::ComputeEdgeScoreRandom, ");
  TInt NumTrials = 1;  // TODO: Change back to 100
  ToPrepend4 += NumTrials.GetStr() + " trials) ";
  int RandomSeed = (int) ((TSecTm::GetCurTm()).GetAbsSecs() % TInt::Mx);
  TRnd RandomGenerator(RandomSeed);
  TryPartitioningMethod(QB, DB, QGraph, Log, ToPrepend4, Clustering::ComputeEdgeScoreRandom, NumTrials, &RandomGenerator);
}

void TEdgesDel::TryEdgeScore(TQuoteBase *QB, TDocBase *DB, const PNGraph& QGraph, LogOutput& Log, const TStr& ToPrepend,
                             TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), TInt NumTrials,
                             TRnd *RandomGenerator) {
  TIntV AllNIdsV;
  QGraph->GetNIdV(AllNIdsV);

  TFlt AvgPercentEdgesDeletedNFS, AvgPercentEdgesDeleted, AvgPercentEdgeScoreDeletedNFS, AvgPercentEdgeScoreDeleted;
  for (int i = 0; i < NumTrials; i++) {
    fprintf(stderr, "Trial %d: Number of edges in QGraph - %d\n", i, QGraph->GetEdges());
    PNGraph QGraphPartitioned = TSnap::GetSubGraph(QGraph, AllNIdsV);

    //fprintf(stderr, "Doing incremental edge deletion, choosing cluster with max edge score\n");
    if (ToPrepend.IsStrIn("ComputeEdgeScoreBaseline")) {
      Clustering ClusterJob(QGraphPartitioned, QB, DB);
      fprintf(stderr, "Computing edge score baseline \n");
      ClusterJob.IncrementalEdgeDeletion(QGraphPartitioned, QB, DB, Fn, false, true);
    } else {
      Clustering ClusterJob(QGraphPartitioned);
      ClusterJob.IncrementalEdgeDeletion(QGraphPartitioned, QB, DB, Fn, false);
    }
    UpdatePercentages(QB, DB, QGraph, QGraphPartitioned, AvgPercentEdgeScoreDeletedNFS, AvgPercentEdgeScoreDeleted,
                      AvgPercentEdgesDeletedNFS, AvgPercentEdgesDeleted, Log, ToPrepend);
  }

  AvgPercentEdgeScoreDeletedNFS /= NumTrials;
  AvgPercentEdgeScoreDeleted /= NumTrials;

  AvgPercentEdgesDeletedNFS /= NumTrials;
  AvgPercentEdgesDeleted /= NumTrials;

  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgesDeletedNFS, AvgPercentEdgesDeletedNFS);
  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgesDeleted, AvgPercentEdgesDeleted);

  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgeScoreDeletedNFS, AvgPercentEdgeScoreDeletedNFS);
  Log.LogValue(ToPrepend + TEdgesDel::PercentEdgeScoreDeleted, AvgPercentEdgeScoreDeleted);
}

void TEdgesDel::CompareEdgeScores(TQuoteBase *QB, TDocBase *DB, PNGraph& QGraph, LogOutput& Log) {
  Log.LogValue("Original number of edges: ", TInt(QGraph->GetEdges()));
  Log.LogValue("Original number of nodes: ", TInt(QGraph->GetNodes()));

  TStr ToPrepend("(Clustering::ComputeEdgeScore) ");
  TryEdgeScore(QB, DB, QGraph, Log, ToPrepend, Clustering::ComputeEdgeScore);

  TStr ToPrepend2("(Clustering::ComputeEdgeScoreWeighted) ");
  TryEdgeScore(QB, DB, QGraph, Log, ToPrepend2, Clustering::ComputeEdgeScoreWeighted);

  TStr ToPrepend3("(Clustering::ComputeEdgeScoreOriginal) ");
  TryEdgeScore(QB, DB, QGraph, Log, ToPrepend3, Clustering::ComputeEdgeScoreOld);

  TStr ToPrepend4("(Clustering::ComputeEdgeScoreBaseline, ");
  TInt NumTrials = 100;
  ToPrepend4 += NumTrials.GetStr() + " trials) ";
  TryEdgeScore(QB, DB, QGraph, Log, ToPrepend4, Clustering::ComputeEdgeScore, NumTrials);
}

