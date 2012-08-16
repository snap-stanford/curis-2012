#include "stdafx.h"

class DCluster {
public:
  TSecTm Start;
  TSecTm End;
  TStr RepStr;
  TStr RepURL;
  TInt Size;
  TInt Unique;
};

void PlotUniqueClusterSize(int* Counts) {
  TGnuPlot Plot = TGnuPlot("plot_unique_clusters", "Cluster Size Frequency - Unique Quotes", false);
  Plot.SetXYLabel("number of unique quotes", "number of clusters");
  Plot.SetScale(gpsLog2XY);

  TIntPrV Coordinates;
  for (int i = 0; i < 100; ++i) {
    Coordinates.Add(TIntPr(i, Counts[i]));
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng("plot_unique_clusters.png");
}

void PlotClusterLifespan(int* Counts) {
  TGnuPlot Plot = TGnuPlot("plot_cluster_lifespan", "Cluster Lifespan", false);
  Plot.SetXYLabel("number of days", "number of clusters");
  Plot.SetScale(gpsLog2XY);

  TIntPrV Coordinates;
  for (int i = 0; i < 100; ++i) {
    Coordinates.Add(TIntPr(i, Counts[i]));
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng("plot_cluster_lifespan.png");
}

void PlotClusterLength(int *LenCounts) {
  TGnuPlot Plot = TGnuPlot("plot_cluster_quote_length", "Cluster Quote Length Distribution", false);
  Plot.SetXYLabel("number of words", "number of clusters");
  //Plot.SetScale(gpsLog2XY);

  TIntPrV Coordinates;
  for (int i = 5; i < 31; ++i) {
    Coordinates.Add(TIntPr(i, LenCounts[i]));
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng("plot_cluster_quote_length.png");
}

void PlotClusterLengthVsLifespan(int *LenCounts, int *LenUnique) {
  TGnuPlot Plot = TGnuPlot("plot_cluster_length_lifespan", "Cluster Quote Length vs. Lifespan", false);
  Plot.SetXYLabel("number of words", "average lifespan");
  //Plot.SetScale(gpsLog2XY);

  TVec<TPair<TInt, TFlt> > Coordinates;
  for (int i = 5; i < 31; ++i) {
    Coordinates.Add(TPair<TInt, TFlt>(i, LenUnique[i] * 1.0 / LenCounts[i]));
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng("plot_cluster_length_lifespan.png");
}

void BuildClusterVec(TStr FileName, TVec<DCluster>& Deleted) {
  PSIn FileLoader = TFIn::New(FileName);
  TStr CurLn;
  int Counts[100];
  for (int i = 0; i < 100; i++) {
    Counts[i] = 0;
  }
  int DayCounts[50];
  for (int i = 0; i < 50; i++) {
    DayCounts[i] = 0;
  }
  int LenCounts[31];
  for (int i = 0; i < 31; i++) {
    LenCounts[i] = 0;
  }
  int LenUnique[31];
  for (int i = 0; i < 31; i++) {
    LenUnique[i] = 0;
  }
  while (FileLoader->GetNextLn(CurLn)) {
    DCluster NewCluster;
    TStrV Params;
    CurLn.SplitOnStr("\t", Params);
    if (Params.Len() >= 6) {
      NewCluster.Start = TSecTm::GetDtTmFromYmdHmsStr(Params[0]);
      NewCluster.End = TSecTm::GetDtTmFromYmdHmsStr(Params[1]);
      uint StartDay = NewCluster.Start.GetInUnits(tmuDay);
      uint EndDay = NewCluster.End.GetInUnits(tmuDay);
      int Diff = EndDay - StartDay + 1;
      if (Diff < 50) DayCounts[Diff]++;
      NewCluster.Unique = TInt(Params[2].GetInt());
      if (NewCluster.Unique < 100) Counts[NewCluster.Unique.Val]++;
      NewCluster.Size = TInt(Params[3].GetInt());
      NewCluster.RepStr = Params[4];
      TStrV StrV;
      NewCluster.RepStr.SplitOnStr(" ", StrV);
      if (StrV.Len() <= 30 && Diff > 1) {
        LenCounts[StrV.Len()]++;
        LenUnique[StrV.Len()] += Diff;
      }
      NewCluster.RepURL = Params[5];
      Deleted.Add(NewCluster);
    }
  }

  for (int i = 0; i < 100; i++) {
    if (Counts[i] > 0) Err("%d: %d\n", i, Counts[i]);
  }
  Err("--------------------------------\n");
  for (int i = 0; i < 50; i++) {
    if (DayCounts[i] > 0) Err("%d: %d\n", i, DayCounts[i]);
  }

  //PlotUniqueClusterSize(Counts);
  //PlotClusterLifespan(DayCounts);
  PlotClusterLength(LenCounts);
  PlotClusterLengthVsLifespan(LenCounts, LenUnique);

}

int main(int argc, char *argv[]) {
  TStr FileName = "all_deleted_clusters.txt";
  TVec<DCluster> Deleted;
  BuildClusterVec(FileName, Deleted);
  Err("Number of clusters: %d\n", Deleted.Len());
  return 0;
}
