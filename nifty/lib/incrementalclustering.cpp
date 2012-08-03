#include "stdafx.h"
#include "incrementalclustering.h"

const TInt TIncrementalClustering::DayThreshold = 3;
const TInt TIncrementalClustering::QuoteThreshold = 20;

TIncrementalClustering::TIncrementalClustering(TQuoteBase *QB, TIntSet& NewQuotes, PNGraph QGraph, TIntSet& AffectedNodes) : Clustering(QGraph) {
  this->QB = QB;
  this->NewQuotes = NewQuotes;
  this->QGraph = QGraph;
  this->AffectedNodes = AffectedNodes;
}

void TIncrementalClustering::KeepAtMostOneChildPerNode(PNGraph& G, TQuoteBase *QB, TDocBase *DB) {
  TIntSet::TIter EndNode = AffectedNodes.EndI();
  for (TIntSet::TIter NodeId = AffectedNodes.BegI(); NodeId < EndNode; NodeId++) {
    TNGraph::TNodeI Node = G->GetNI(NodeId.GetKey());
    TQuote SourceQuote;
    if (QB->GetQuote(Node.GetId(), SourceQuote)) {
      TInt NodeDegree = Node.GetOutDeg();
      if (NodeDegree > 1) {
        TFlt MaxScore = 0;
        TInt MaxNodeId = 0;
        TIntV NodeV;
        // first pass: check to see if we are pointing to any old nodes - if so, they get higher
        // priority over the new ones for edge selection.
        bool ContainsOldNode = false;
        for (int i = 0; i < NodeDegree; ++i) {
          if (!NewQuotes.IsKey(Node.GetOutNId(i))) {
            ContainsOldNode = true;
          }
        }
        // modified edge selection: filter out new nodes if old ones exist.
        for (int i = 0; i < NodeDegree; ++i) {
          TInt CurNode = Node.GetOutNId(i);
          NodeV.Add(CurNode);
          TQuote DestQuote;
          if (QB->GetQuote(CurNode, DestQuote)) {
            TFlt EdgeScore = 0;
            if (!ContainsOldNode || !NewQuotes.IsKey(Node.GetOutNId(i))) {
              EdgeScore = ComputeEdgeScore(SourceQuote, DestQuote, DB);
            }
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
  fprintf(stderr, "finished deleting edges\n");
}
