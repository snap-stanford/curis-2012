#include "stdafx.h"

class FCluster {
public:
  TSecTm Start;
  TSecTm End;
  TStr RepStr;
  TStr RepURL;
  TInt Size;
  TInt Unique;
  TStr PopStr;
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

void PlotClusterLifespanVsSize(int *DayCounts, int *LenSize) {
  TGnuPlot Plot = TGnuPlot("plot_lifespan_vs_size", "Cluster Lifespan vs. Average Size", false);
  Plot.SetXYLabel("lifespan (in days)", "average cluster size");
  //Plot.SetScale(gpsLog2XY);

  TVec<TPair<TInt, TFlt> > Coordinates;
  for (int i = 5; i < 31; ++i) {
    if (DayCounts[i] > 0) {
      Coordinates.Add(TPair<TInt, TFlt>(i, LenSize[i] * 1.0 / DayCounts[i]));
    }
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng("plot_lifespan_vs_size.png");
}

void BuildClusterVec(TStr FileName, TVec<FCluster>& Deleted) {
  PSIn FileLoader = TFIn::New(FileName);
  TStr CurLn;
  int Counts[100];
  for (int i = 0; i < 100; i++) {
    Counts[i] = 0;
  }
  int DayCounts[200];
  for (int i = 0; i < 200; i++) {
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

  int LenSize[200];
  for (int i = 0; i < 200; i++) {
    LenSize[i] = 0;
  }
  while (FileLoader->GetNextLn(CurLn)) {
    FCluster NewCluster;
    TStrV Params;
    CurLn.SplitOnStr("\t", Params);
    if (Params.Len() >= 6) {
      NewCluster.Start = TSecTm::GetDtTmFromYmdHmsStr(Params[0]);
      NewCluster.End = TSecTm::GetDtTmFromYmdHmsStr(Params[1]);
      uint StartDay = NewCluster.Start.GetInUnits(tmuDay);
      uint EndDay = NewCluster.End.GetInUnits(tmuDay);
      int Diff = EndDay - StartDay + 1;
      if (Diff < 200) DayCounts[Diff]++;
      NewCluster.Unique = TInt(Params[2].GetInt());
      if (NewCluster.Unique < 100) Counts[NewCluster.Unique.Val]++;
      NewCluster.Size = TInt(Params[3].GetInt());
      if (Diff < 200) LenSize[Diff] += NewCluster.Size;
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
  //PlotClusterLength(LenCounts);
  //PlotClusterLengthVsLifespan(LenCounts, LenUnique);
  PlotClusterLifespanVsSize(DayCounts, LenSize);

}

int main(int argc, char *argv[]) {
  TStr FileName = "all_deleted_clusters.txt";
  TVec<FCluster> Deleted;
  BuildClusterVec(FileName, Deleted);
  Err("Number of clusters: %d\n", Deleted.Len());
  return 0;
}
