#include "stdafx.h"
#include "clustering.h"
#include "quotegraph.h"
#include "logoutput.h"

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

void Clustering::SetGraph(PNGraph QGraph) {
  this->QGraph = QGraph;
}

void Clustering::GetRootNodes(TIntSet& RootNodes) {
  TNGraph::TNodeI EndNode = QGraph->EndNI();
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < EndNode; Node++) {
    if (Node.GetOutDeg() == 0) {
      RootNodes.AddKey(Node.GetId());
    }
  }
}

void Clustering::BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB, TDocBase *DB, LogOutput& log) {
  // currently deletes all edges but the one leading to phrase that is most frequently cited.
  // TODO: Make more efficient? At 10k nodes this is ok

  TIntV OrigQGraphNIdV;
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < QGraph->EndNI(); Node++) {
    OrigQGraphNIdV.Add(Node.GetId());
  }
  PNGraph OrigQGraphP;
  OrigQGraphP = TSnap::GetSubGraph(QGraph, OrigQGraphNIdV);
  
  int NumEdgesOriginal = QGraph->GetEdges();
  log.LogValue(LogOutput::NumOriginalEdges, TInt(NumEdgesOriginal));
  int NumNodes = QGraph->GetNodes();
  log.LogValue(LogOutput::NumQuotes, TInt(NumNodes));

  printf("Deleting extra graph edges...\n");
  TNGraph::TNodeI EndNode = QGraph->EndNI();
  int count = 0;
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < EndNode; Node++) {
    TQuote SourceQuote;
    if (QB->GetQuote(Node.GetId(), SourceQuote)) {
      TInt NodeDegree = Node.GetOutDeg();
      if (NodeDegree == 0) {
        RootNodes.AddKey(Node.GetId());
        //printf("ROOT NODE\n");
      } else if (NodeDegree > 1) {
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
            QGraph->DelEdge(Node.GetId(), NodeV[i]);
          }
        }
        //printf("Out degree: %d out of %d\n", Node.GetOutDeg(), NodeDegree.Val);
      } else {
        count++;
      }
    }
  }
  printf("%d nodes with out degree 1 found.\n", count);
  log.LogValue(LogOutput::NumRemainingEdges, TInt(QGraph->GetEdges()));
  log.LogValue(LogOutput::PercentEdgesDeleted, TFlt(100 - QGraph->GetEdges() * 100.0 / NumEdgesOriginal));

  // find weakly connected components. these are our clusters. largely taken from memes.h
  printf("Finding weakly connected components\n");
  TCnComV Components;
  TSnap::GetWccs(QGraph, Components);
  Components.Sort(false);
  Clusters.Clr(false);
  TIntSet SeenSet;
  printf("%d weakly connected components discovered.", Components.Len());
  log.LogValue(LogOutput::NumClusters, TInt(Components.Len()));
  for (int i = 0; i < Components.Len(); i++) {
    for (int n = 0; n < Components[i].NIdV.Len(); n++) {
      IAssert(! SeenSet.IsKey(Components[i].NIdV[n]));
      SeenSet.AddKey(Components[i].NIdV[n]);
    }
    Clusters.Add(Components[i].NIdV);
  }

  // calculate percent edges deleted from induced subgraphs of these WCC's
  TInt NumEdgesInducedSubgraphs = 0;
  for (int i = 0; i < Clusters.Len(); i++) {
    PNGraph SubgraphP;
    SubgraphP = TSnap::GetSubGraph(OrigQGraphP, Clusters[i]);
    NumEdgesInducedSubgraphs += SubgraphP->GetEdges();
  }

  //fprintf(stderr, "percent edges deleted, not from subgraphs: %f\n", TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal).Val);
  log.LogValue(LogOutput::PercentEdgesDeletedNotFromSubgraphs, TFlt(100 - NumEdgesInducedSubgraphs * 100.0 / NumEdgesOriginal));
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
  else
    printf("GAH!");

  // adhoc function between frequency and edit distance and peak diff.
  // TODO: learn this! haha :)
  return NumSources * 2 * 3600.0 /(EditDistance + 1.0)/(PeakDistanceInSecs + 1);
}

/// Sorts clusters in decreasing order, and finds representative quote for each cluster
//  RepQuotesAndFreq is a vector of cluster results, where each TTriple<TInt, TInt, TIntV>
//  contains the reference quote id, the total frequency of quotes in the cluster, and a
//  vector of ids of the quotes in the cluster, in that order
void Clustering::SortClustersByFreq(TVec<TTriple<TInt, TInt, TIntV> >& RepQuotesAndFreq, TVec<TIntV>& Clusters, TQuoteBase *QuoteBase) {
  printf("Sorting clusters by frequency\n");
  for (int i = 0; i < Clusters.Len(); i++) {
    TIntV Cluster = Clusters[i];
    TTriple<TInt, TInt, TIntV> ClusterRepQuoteAndFreq;
    for (int j = 0; j < Cluster.Len(); j++) {
      TInt QId = Cluster[j];
      TQuote Q;
      QuoteBase->GetQuote(QId, Q);
      ClusterRepQuoteAndFreq.Val2 += Q.GetNumSources();
      TNGraph::TNode Node = QGraph->GetNodeC(Q.GetId());
      if (Node.GetOutDeg() == 0) {
        //Q.GetContentString(ClusterRepQuoteAndFreq.Val1);
        ClusterRepQuoteAndFreq.Val1 = QId;
      }
    }
    ClusterRepQuoteAndFreq.Val3 = Cluster;
    RepQuotesAndFreq.Add(ClusterRepQuoteAndFreq);
  }

  RepQuotesAndFreq.SortCmp(TCmpTripleByVal2<TInt, TInt, TIntV>(false));
  printf("Sorted: %d\n", RepQuotesAndFreq.Len());
}
