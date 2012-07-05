#include "stdafx.h"
#include "clustering.h"

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

void Clustering::BuildClusters(TIntSet& RootNodes, TVec<TIntV>& Clusters, TQuoteBase *QB) {
  // currently deletes all edges but the one leading to phrase that is most frequently cited.
  // TODO: Make more efficient? At 10k nodes this is ok
  TNGraph::TNodeI EndNode = QGraph->EndNI();
  for (TNGraph::TNodeI Node = QGraph->BegNI(); Node < EndNode; Node++) {
    TInt NodeDegree = Node.GetOutDeg();
    if (NodeDegree == 0) {
      RootNodes.AddKey(Node.GetId());
    } else if (NodeDegree > 1) {
      TInt MaxCites = 0;
      TInt MaxNodeId = 0;
      // find the node that has the largest number of sources
      for (int i = 0; i < NodeDegree; ++i) {
        TInt CurNode = Node.GetOutNId(i);
        TQuote CurQuote;
        if (QB->GetQuote(CurNode, CurQuote) && CurQuote.GetNumSources() > MaxCites) {
          MaxCites = CurQuote.GetNumSources();
          MaxNodeId = CurNode;
        }
      }
      // remove all other edges
      for (int i = 0; i < NodeDegree; ++i) {
        TInt CurNode = Node.GetOutNId(i);
        if (CurNode != MaxNodeId) {
          QGraph->DelEdge(Node.GetId(), CurNode);
        }
      }
    }
  }

  // find weakly connected components. these are our clusters. largely taken from memes.h
  TCnComV Components;
  TSnap::GetWccs(QGraph, Components);
  Components.Sort(false);
  Clusters.Clr(false);
  TIntSet SeenSet;
  for (int i = 0; i < Components.Len(); i++) {
    for (int n = 0; n < Components[i].NIdV.Len(); n++) {
      IAssert(! SeenSet.IsKey(Components[i].NIdV[n]));
      SeenSet.AddKey(Components[i].NIdV[n]);
    }
    Clusters.Add(Components[i].NIdV);
  }
}

void Clustering::SortClustersByFreq(TVec<TPair<TStr, TInt> >& RepQuotesAndFreq,
                                    TVec<TIntV>& Clusters, TQuoteBase *QuoteBase) {
  for (int i = 0; i < Clusters.Len(); i++) {
    TIntV Cluster = Clusters[i];
    TPair<TStr, TInt> ClusterRepQuoteAndFreq;
    for (int j = 0; j < Cluster.Len(); j++) {
      TInt QId = Cluster[j];
      TQuote Q;
      QuoteBase->GetQuote(QId, Q);
      ClusterRepQuoteAndFreq.Val2 += Q.GetNumSources();
      TNGraph::TNode Node = QGraph->GetNodeC(Q.GetId());
      if (Node.GetOutDeg() == 0) {
        Q.GetContentString(ClusterRepQuoteAndFreq.Val1);
      }
    }
  }

  TClusterCompareByFreq Cmp;
  RepQuotesAndFreq.SortCmp(Cmp);
}
