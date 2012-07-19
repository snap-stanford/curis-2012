#ifndef clusterplot_h
#define clusterplot_h

#include "stdafx.h"
#include "clustering.h"

class ClusterPlot {
private:
  TStr RootDirectory;
public:
  ClusterPlot(TStr RootDirectory);
  void PlotClusterSizeUnique(TVec<TIntV>& Clusters);
  void PlotClusterSize(TVec<TCluster>& ClusterSummaries);
  void PlotQuoteFrequencies(TQuoteBase* QB);
  //void PlotClusterPeaks(TTriple<TInt, TInt, TIntV>& Cluster);

  class TCmpIntV {
  private:
    bool IsAsc;
  public:
    TCmpIntV(const bool& AscSort=true) : IsAsc(AscSort) { }
    bool operator () (const TIntV& P1,
                      const TIntV& P2) const {
      if (IsAsc) {
        return P1.Len() < P2.Len();
      } else {
        return P2.Len() < P1.Len();
      }
    }
  };
};

#endif
