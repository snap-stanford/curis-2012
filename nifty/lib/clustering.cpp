#include "stdafx.h"
#include "clustering.h"
#include "quotegraph.h"
#include "logoutput.h"

Clustering::Clustering(PNGraph QGraph) {
  this->QGraph = QGraph;
}

void Clustering::Save(TSOut &SOut) const {
  //QuoteIdCounter.Save(SOut);
  //IdToTQuotes.Save(SOut);
  //QuoteToId.Save(SOut);
}

void Clustering::Load(TSIn& SIn) {
  //QuoteIdCounter.Load(SIn);
  //IdToTQuotes.Load(SIn);
  //QuoteToId.Load(SIn);
}

void Clustering::GetRootNodes(TIntSet& RootNodes) {
  TNGraph::TNodeI EndNode = QGraph->EndNI();
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < EndNode; Node++) {
    if (Node.GetOutDeg() == 0) {
      RootNodes.AddKey(Node.GetId());
    }
  }
}

void Clustering::BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TClusterBase *OldCB) {
  // currently deletes all edges but the one leading to phrase that is most frequently cited.
  // TODO: Make more efficient? At 10k nodes this is ok

  TIntV AllNIdsV;
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < QGraph->EndNI(); Node++) {
    AllNIdsV.Add(Node.GetId());
  }
  PNGraph OrigQGraphP;
  OrigQGraphP = TSnap::GetSubGraph(QGraph, AllNIdsV);
  
  int NumEdgesOriginal = QGraph->GetEdges();
  log.LogValue(LogOutput::NumOriginalEdges, TInt(NumEdgesOriginal));
  int NumNodes = QGraph->GetNodes();
  log.LogValue(LogOutput::NumQuotes, TInt(NumNodes));

  printf("Deleting extra graph edges...\n");
  KeepAtMostOneChildPerNode(QGraph, QB, DB);
  fprintf(stderr, "edges deleted!\n");

  log.LogValue(LogOutput::NumRemainingEdges, TInt(QGraph->GetEdges()));
  log.LogValue(LogOutput::PercentEdgesDeleted, TFlt(100 - QGraph->GetEdges() * 100.0 / NumEdgesOriginal));

  // find weakly connected components. these are our clusters. largely taken from memes.h
  fprintf(stderr, "Finding weakly connected components\n");
  TVec<TIntV> Clusters;
  GetAllWCCs(QGraph, Clusters);
  log.LogValue(LogOutput::NumClusters, TInt(Clusters.Len()));

  // calculate percent edges deleted from induced subgraphs of these WCC's
  TInt NumEdgesInducedSubgraphs = 0;
  for (int i = 0; i < Clusters.Len(); i++) {
    PNGraph SubgraphP;
    SubgraphP = TSnap::GetSubGraph(OrigQGraphP, Clusters[i]);
    NumEdgesInducedSubgraphs += SubgraphP->GetEdges();
  }

  fprintf(stderr, "percent edges deleted, not from subgraphs: %f\n", TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal).Val);
  log.LogValue(LogOutput::PercentEdgesDeletedNotFromSubgraphs, TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal));

  // Calculate baseline percentage of edges deleted
  PNGraph RandomQGraph;
  RandomQGraph = TSnap::GenRewire(QGraph);
  PNGraph OrigRandomQGraph;
  OrigRandomQGraph = TSnap::GetSubGraph(RandomQGraph, AllNIdsV);

  KeepAtMostOneChildPerNode(RandomQGraph, QB, DB);

  TVec<TIntV> ClustersRandom;
  GetAllWCCs(RandomQGraph, ClustersRandom);

  NumEdgesInducedSubgraphs = 0;
  for (int i = 0; i < ClustersRandom.Len(); i++) {
    PNGraph SubgraphP;
    SubgraphP = TSnap::GetSubGraph(OrigRandomQGraph, ClustersRandom[i]);
    NumEdgesInducedSubgraphs += SubgraphP->GetEdges();
  }
  
  log.LogValue(LogOutput::PercentEdgesDeletedNFSBaseline, TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal));
  
  // Add clusters to CB
  for (int i = 0; i < Clusters.Len(); i++) {
    TCluster Cluster;
    Cluster.AddQuote(QB, Clusters[i]);
    TQuote ClusterRepQuote;
    CalcRepresentativeQuote(ClusterRepQuote, Clusters[i], QB);
    TIntV ClusterRepQuoteIds;
    ClusterRepQuoteIds.Add(ClusterRepQuote.GetId());
    Cluster.SetRepresentativeQuoteIds(ClusterRepQuoteIds);
    CB->AddCluster(Cluster, OldCB);
  }
}

TFlt Clustering::ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB) {
  TInt NumSources = Dest.GetNumSources();
  TStrV Content1, Content2;
  Source.GetParsedContent(Content1);
  Dest.GetParsedContent(Content2);
  TInt EditDistance = QuoteGraph::WordLevenshteinDistance(Content1, Content2);

  TVec<TSecTm> SourcePeakVectors, DestPeakVectors;
  Source.GetPeaks(DB, SourcePeakVectors);
  Dest.GetPeaks(DB, DestPeakVectors);
  // looks at first peak for now - this should also hopefully be the biggest peak
  TInt PeakDistanceInSecs = 2 * 3600;
  //printf("getting peak..\n");
  if (SourcePeakVectors.Len() > 0 && DestPeakVectors.Len() > 0)
    PeakDistanceInSecs = TInt::Abs(SourcePeakVectors[0].GetAbsSecs() - DestPeakVectors[0].GetAbsSecs());
  //else
    //printf("GAH!");

  // adhoc function between frequency and edit distance and peak diff.
  // TODO: learn this! haha :)
  return NumSources * 2 * 3600.0 /(EditDistance + 1.0)/(PeakDistanceInSecs + 1);
}

/// Calculates the representative quote and returns it in RepQuote; returns the
//  total number of sources for the quotes in the cluster
TInt Clustering::CalcRepresentativeQuote(TQuote& RepQuote, TIntV& Cluster, TQuoteBase *QuoteBase) {
  TInt NumQuotes = 0;
  QuoteBase->GetQuote(Cluster[0], RepQuote);  // Initialize RepQuote
  for (int j = 0; j < Cluster.Len(); j++) {
    TInt QId = Cluster[j];
    TQuote Q;
    QuoteBase->GetQuote(QId, Q);
    if (!QuoteGraph::EdgeShouldBeFromOneToTwo(Q, RepQuote)) {
      RepQuote = Q;
    }
    NumQuotes += Q.GetNumSources();
  }
  return NumQuotes;
}

/// Sorts clusters in decreasing order, and finds representative quote for each cluster
//  RepQuotesAndFreq is a vector of cluster results, represented by TClusters
//  TODO: Pick representative quote to be the most frequent? (rather than the longest)
/*void Clustering::SortClustersByFreq(TVec<TCluster>& ClusterSummaries, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase) {
  fprintf(stderr, "Sorting clusters by frequency\n");
  for (int i = 0; i < Clusters.Len(); i++) {
    TIntV Cluster = Clusters[i];
    Cluster.SortCmp(TCmpQuoteByFreq(false, QuoteBase)); // sort by descending frequency
    TQuote RepQuote;
    TInt NumQuotes = CalcRepresentativeQuote(RepQuote, Cluster, QuoteBase);
    TIntV RepV;
    RepV.Add(RepQuote.GetId());
    TCluster ClusterSummary(RepV, NumQuotes, Cluster, QuoteBase);
    ClusterSummaries.Add(ClusterSummary);
  }

  ClusterSummaries.SortCmp(TCmpTClusterByNumQuotes(false));
  fprintf(stderr, "Sorted: %d\n", ClusterSummaries.Len());
}*/

void Clustering::KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB) {
  TNGraph::TNodeI EndNode = G->EndNI();
  for (TNGraph::TNodeI Node = G->BegNI(); Node < EndNode; Node++) {
    TQuote SourceQuote;
    if (QB->GetQuote(Node.GetId(), SourceQuote)) {
      TInt NodeDegree = Node.GetOutDeg();
      if (NodeDegree > 1) {
        TFlt MaxScore = 0;
        TInt MaxNodeId = 0;
        TIntV NodeV;
        // find the node that has the largest number of sources
        for (int i = 0; i < NodeDegree; ++i) {
          TInt CurNode = Node.GetOutNId(i);
          NodeV.Add(CurNode);
          TQuote DestQuote;
          if (QB->GetQuote(CurNode, DestQuote)) {
            TFlt EdgeScore = ComputeEdgeScore(SourceQuote, DestQuote, DB);
            if (EdgeScore > MaxScore) {
              MaxScore = EdgeScore;
              MaxNodeId = CurNode;
            }
          }
        }

        // remove all other edges, backwards to prevent indexing fail
        for (int i = 0; i < NodeV.Len(); i++) {
          if (NodeV[i] != MaxNodeId) {
            G->DelEdge(Node.GetId(), NodeV[i]);
          }
        }
        //printf("Out degree: %d out of %d\n", Node.GetOutDeg(), NodeDegree.Val);
      }
    }
  }
  fprintf(stderr, "Edge deletion complete - each node should have max one outgoing edge now!\n");
}

void Clustering::GetAllWCCs(PNGraph& G, TVec<TIntV>& Clusters) {
  TCnComV Components;
  TSnap::GetWccs(G, Components);
  Components.Sort(false);
  Clusters.Clr(false);
  TIntSet SeenSet;
  fprintf(stderr, "%d weakly connected components discovered.", Components.Len());
  for (int i = 0; i < Components.Len(); i++) {
    for (int n = 0; n < Components[i].NIdV.Len(); n++) {
      IAssert(! SeenSet.IsKey(Components[i].NIdV[n]));
      SeenSet.AddKey(Components[i].NIdV[n]);
    }
    Clusters.Add(Components[i].NIdV);
  }
}
