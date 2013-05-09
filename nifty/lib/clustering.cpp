#include "stdafx.h"
#include "clustering.h"
#include "edgesdel.h"
#include "quotegraph.h"
#include "logoutput.h"

/** Pass in TQuoteBase and TDocBase if you want to run
 *  edge score baseline later
 */
Clustering::Clustering(PNGraph QGraph, TQuoteBase *QB, TDocBase *DB) {
  this->QGraph = QGraph;
  if (QB != NULL && DB != NULL) {
    PrepForEdgeScoreBaseline(QB, DB);
  }
}

Clustering::~Clustering() {
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

void Clustering::BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TSecTm& PresentTime, TStr method, bool CheckEdgesDel) {
  BuildClusters(CB, QB, DB, log, PresentTime, NULL, method, CheckEdgesDel);
}

void Clustering::BuildClusters(TClusterBase *CB, TQuoteBase *QB, TDocBase *DB, LogOutput& log, TSecTm& PresentTime, TClusterBase *OldCB, TStr method, bool CheckEdgesDel) {
  // currently deletes all edges but the one leading to phrase that is most frequently cited.
  // TODO: Make more efficient? At 10k nodes this is ok

  TIntV AllNIdsV;
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < QGraph->EndNI(); Node++) {
    AllNIdsV.Add(Node.GetId());
  }
  PNGraph OrigQGraphP;
  OrigQGraphP = TSnap::GetSubGraph(QGraph, AllNIdsV);

  if (CheckEdgesDel) {
    // TODO: Remove this after done with edge score experiments
    fprintf(stderr, "Saving QBDBGraph\n");
    TFOut FOut("QBDBGraph_foredgedel.bin");
    QB->Save(FOut);
    DB->Save(FOut);
    QGraph->Save(FOut);
    fprintf(stderr, "Done saving QBDBGraph!\n");
  }
  
  int NumEdgesOriginal = QGraph->GetEdges();
  log.LogValue(LogOutput::NumOriginalEdges, TInt(NumEdgesOriginal));
  int NumNodes = QGraph->GetNodes();
  log.LogValue(LogOutput::NumQuotes, TInt(NumNodes));

  printf("Deleting extra graph edges...\n");

  if (method == "incremental") {
    IncrementalEdgeDeletion(QGraph, QB, DB, OldCB);
  } else {
    KeepAtMostOneChildPerNode(QGraph, QB, DB);
  }

  fprintf(stderr, "edges deleted!\n");

  log.LogValue(LogOutput::NumRemainingEdges, TInt(QGraph->GetEdges()));
  log.LogValue(LogOutput::PercentEdgesDeleted, TFlt(100 - QGraph->GetEdges() * 100.0 / NumEdgesOriginal));

  // find weakly connected components. these are our clusters. largely taken from memes.h
  fprintf(stderr, "Finding weakly connected components\n");
  TVec<TIntV> Clusters;
  GetAllWCCs(QGraph, Clusters);
  log.LogValue(LogOutput::NumClusters, TInt(Clusters.Len()));

  if (CheckEdgesDel) {
    TEdgesDel::ComparePartitioningMethods(QB, DB, QGraph, log);
    //TEdgesDel::CompareEdgeScores(QB, DB, QGraph, log);
  }
  
  // Add clusters to CB. keep track of cluster birth time (aka today)
  for (int i = 0; i < Clusters.Len(); i++) {
    TCluster Cluster;
	Cluster.DeathDate = PresentTime;
    Cluster.AddQuote(QB, DB, Clusters[i]);
    TQuote ClusterRepQuote;
    CalcRepresentativeQuote(ClusterRepQuote, Clusters[i], QB);
    TIntV ClusterRepQuoteIds;
    ClusterRepQuoteIds.Add(ClusterRepQuote.GetId());
    Cluster.SetRepresentativeQuoteIds(ClusterRepQuoteIds);
    CB->AddCluster(Cluster, OldCB, PresentTime);
  }
  fprintf(stderr, "cluster generation complete!\n");
}

TFlt Clustering::ComputeEdgeScoreGeneral(TQuote& Source, TQuote& Dest, TDocBase *DB, TStr Type, TRnd *RandomGenerator) {
  TInt NumSources = Dest.GetNumSources();
  TStrV Content1, Content2;
  Source.GetParsedContent(Content1);
  Dest.GetParsedContent(Content2);
  TInt EditDistance = TStringUtil::WordLevenshteinDistance(Content1, Content2);

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
  if (Type == "Division") {
    return NumSources * 2 * 3600.0 /(EditDistance + 1.0)/(PeakDistanceInSecs + 1);
  } else if (Type == "Weighted") {
    return NumSources * (1.0 / 10) - 10.0 * EditDistance - PeakDistanceInSecs * (1.0 / (3600 * 24));
  } else if (Type == "Old") {
    return NumSources * 1.0;
  }
  IAssert(false);  // this point should never be reached
  return 1;
}

TFlt Clustering::ComputeEdgeScore(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator) {
  TStr Type("Division");
  return ComputeEdgeScoreGeneral(Source, Dest, DB, Type, RandomGenerator);
}

TFlt Clustering::ComputeEdgeScoreRandom(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator) {
  return RandomGenerator->GetUniDev();
}

TFlt Clustering::ComputeEdgeScoreWeighted(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator) {
  TStr Type("Weighted");
  return ComputeEdgeScoreGeneral(Source, Dest, DB, Type, RandomGenerator);
}

TFlt Clustering::ComputeEdgeScoreOld(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator) {
  TStr Type("Old");
  return ComputeEdgeScoreGeneral(Source, Dest, DB, Type, RandomGenerator);
}

TFlt Clustering::ComputeEdgeScoreBaseline(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator) {
  if (EdgeScores.Len() == 0) {
    fprintf(stderr, "ERROR: must pass in QB and DB when calling Clustering constructor\n");
    return 0;
  }
  TIntPr OrigEdge(Source.GetId(), Dest.GetId());
  TIntPr RandomMatchedEdge = EdgeToRandomEdge.GetDat(OrigEdge);
  return EdgeScores.GetDat(RandomMatchedEdge);
}

/// Calculates the representative quote and returns it in RepQuote; returns the
//  total number of sources for the quotes in the cluster
TInt Clustering::CalcRepresentativeQuote(TQuote& RepQuote, TIntV& Cluster, TQuoteBase *QuoteBase) {
  TInt NumQuotes = 0;
  QuoteBase->GetQuote(Cluster[0], RepQuote);  // Initialize RepQuote
  TInt MaxSources = 0;
  for (int j = 0; j < Cluster.Len(); j++) {
    TInt QId = Cluster[j];
    TQuote Q;
    QuoteBase->GetQuote(QId, Q);
    if (!QuoteGraph::EdgeShouldBeFromOneToTwo(Q, RepQuote)) {
      RepQuote = Q;
    }
    /*if (Q.GetNumSources() > MaxSources) {
      MaxSources = Q.GetNumSources();
      RepQuote = Q;
    }*/
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
  KeepAtMostOneChildPerNode(G, QB, DB, ComputeEdgeScore, NULL);
}

void Clustering::KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB,
                                           TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator),
                                           TRnd *RandomGenerator) {
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
            TFlt EdgeScore = (*Fn)(SourceQuote, DestQuote, DB, RandomGenerator);
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
  //fprintf(stderr, "Edge deletion complete - each node should have max one outgoing edge now!\n");
}

void Clustering::IncrementalEdgeDeletion(PNGraph& G, TQuoteBase *QB, TDocBase *DB, TClusterBase *OldCB, bool ConstantEdgeScore) {
  IncrementalEdgeDeletion(G, QB, DB, OldCB, ComputeEdgeScore, ConstantEdgeScore, false);
}

void Clustering::IncrementalEdgeDeletion(PNGraph& G, TQuoteBase *QB, TDocBase *DB, TClusterBase *OldCB, 
                                         TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), bool ConstantEdgeScore, bool RandomEdgeScore) {
  visit.Clr();
  
  if (OldCB != NULL) {
    TIntV ClusterIds;
    OldCB->GetAllClusterIds(ClusterIds);

    for (int i = 0; i < ClusterIds.Len(); i++) {
      TCluster C;
      OldCB->GetCluster(ClusterIds[i], C);

      TIntV QuoteIds;
      C.GetQuoteIds(QuoteIds);

      TInt AssignedClust = QuoteIds[0];
      for (int j = 0; j < QuoteIds.Len(); j++) {
	visit.AddDat(QuoteIds[j], AssignedClust);
      }
    }
  }

  TNGraph::TNodeI EndNode = G->EndNI();
  for (TNGraph::TNodeI Node = G->BegNI(); Node < EndNode; Node++) {
    GetCluster(Node.GetId(), G, QB, DB, Fn, ConstantEdgeScore, RandomEdgeScore);
  }
}

int Clustering::GetCluster(TInt CurNodeId, PNGraph& G, TQuoteBase *QB, TDocBase *DB,
                           TFlt (*Fn)(TQuote& Source, TQuote& Dest, TDocBase *DB, TRnd *RandomGenerator), bool ConstantEdgeScore, bool RandomEdgeScore) {
  TNGraph::TNodeI CurNode = G->GetNI(CurNodeId);
  
  TInt AssignedClust;
  if (visit.IsKeyGetDat(CurNodeId, AssignedClust)) {
    return AssignedClust;
  } 

  TQuote SourceQuote;
  if (QB->GetQuote(CurNode.GetId(), SourceQuote)) {
    TInt NodeDegree = CurNode.GetOutDeg();
    if (NodeDegree == 0) {
      AssignedClust = CurNodeId;
    } else {
      THash<TInt, TFlt> ScoreTable;

      TIntV NodeIdV;
      for (int i = 0; i < NodeDegree; i++) {
	TInt NodeId = CurNode.GetOutNId(i);
	NodeIdV.Add(NodeId);

	int RetVal = GetCluster(NodeId, G, QB, DB, Fn, ConstantEdgeScore, RandomEdgeScore);
	if (RetVal != -1) {
	  TQuote DestQuote;
	  QB->GetQuote(NodeId, DestQuote);

	  TFlt TotalScore;
	  if (!ScoreTable.IsKeyGetDat(RetVal, TotalScore)) {
	    TotalScore = 0;
	  }
          if (ConstantEdgeScore) {  // pick cluster with max # of edges, instead of max edge score
            TotalScore += 1;
          } else if (RandomEdgeScore) {
            TotalScore += ComputeEdgeScoreBaseline(SourceQuote, DestQuote, DB);
          } else {
	    TotalScore += (*Fn)(SourceQuote, DestQuote, DB, NULL);
          }
	  ScoreTable.AddDat(RetVal, TotalScore);
	}
      }

      TIntV ScoreTableKeyV;
      ScoreTable.GetKeyV(ScoreTableKeyV);

      TFlt MaxTotalScore = 0;
      TInt MaxCluster = 0;
      for (int i = 0; i < ScoreTableKeyV.Len(); i++) {
	TFlt TotalScore = ScoreTable.GetDat(ScoreTableKeyV[i]);
	if (TotalScore > MaxTotalScore) {
	  MaxTotalScore = TotalScore;
	  MaxCluster = ScoreTableKeyV[i];
	}
      }
      
      for (int i = 0; i < NodeIdV.Len(); i++) {
	if (GetCluster(NodeIdV[i], G, QB, DB, Fn, ConstantEdgeScore, RandomEdgeScore) != MaxCluster) {
	  G->DelEdge(CurNode.GetId(), NodeIdV[i]);
	}
      }

      AssignedClust = MaxCluster;
    }
  } else {
    AssignedClust = -1;
  }

  visit.AddDat(CurNodeId, AssignedClust);
  return AssignedClust;
}

void Clustering::GetAllWCCs(const PNGraph& G, TVec<TIntV>& Clusters) {
  TCnComV Components;
  TSnap::GetWccs(G, Components);
  Components.Sort(false);
  Clusters.Clr(false);
  TIntSet SeenSet;
  //fprintf(stderr, "%d weakly connected components discovered.", Components.Len());
  for (int i = 0; i < Components.Len(); i++) {
    for (int n = 0; n < Components[i].NIdV.Len(); n++) {
      IAssert(! SeenSet.IsKey(Components[i].NIdV[n]));
      SeenSet.AddKey(Components[i].NIdV[n]);
    }
    Clusters.Add(Components[i].NIdV);
  }
}

void Clustering::PrepForEdgeScoreBaseline(TQuoteBase *QB, TDocBase *DB) {
  fprintf(stderr, "Preparing data structures for edge score baseline\n");
  for (TNGraph::TEdgeI Edge = QGraph->BegEI(); Edge != QGraph->EndEI(); Edge++) {
    TQuote SourceQuote;
    QB->GetQuote(Edge.GetSrcNId(), SourceQuote);
    TQuote DestQuote;
    QB->GetQuote(Edge.GetDstNId(), DestQuote);
    TFlt EdgeScore = ComputeEdgeScore(SourceQuote, DestQuote, DB);
    EdgeScores.AddDat(TIntPr(SourceQuote.GetId(), DestQuote.GetId()), EdgeScore);
  }

  TVec<TIntPr> AllEdges;
  EdgeScores.GetKeyV(AllEdges);
  TVec<TIntPr> ShuffledEdges;
  EdgeScores.GetKeyV(ShuffledEdges);
  int RandomSeed = (int) ((TSecTm::GetCurTm()).GetAbsSecs() % TInt::Mx);
  TRnd RandomGenerator(RandomSeed);
  ShuffledEdges.Shuffle(RandomGenerator);

  for (int i = 0; i < AllEdges.Len(); i++) {
    EdgeToRandomEdge.AddDat(AllEdges[i], ShuffledEdges[i]);  // one-to-one mapping
  }
}
