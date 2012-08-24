#include "stdafx.h"

int MaxWords = 50;
int MaxLifespan = 200; // half a year

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

void SetZero(int *Arr, int Size) {
  for (int i = 0; i < Size; ++i) {
    Arr[i] = 0;
  }
}

void PlotGraph(TStr FileName, TStr Title, TStr XStr, TStr YStr, int *Arr1, int *Arr2, int NumPoints, bool normalize = false, bool log = true) {
  TGnuPlot Plot = TGnuPlot(FileName, Title, false);
  Plot.SetXYLabel(XStr, YStr);
  if (log) Plot.SetScale(gpsLog2XY);

  if (!normalize) {
    TIntPrV Coordinates;
    for (int i = 0; i < NumPoints; ++i) {
      if (Arr1 == NULL)
        Coordinates.Add(TIntPr(i, Arr2[i]));
      else
        Coordinates.Add(TIntPr(Arr1[i], Arr2[i]));
    }
    Plot.AddPlot(Coordinates, gpwPoints);
  } else {
    TVec<TPair<TInt, TFlt> > Coordinates;
    for (int i = 0; i < NumPoints; ++i) {
      if (Arr1[i] != 0) {
        Coordinates.Add(TPair<TInt, TFlt>(i, Arr2[i] * 1.0 / Arr1[i]));
      }
    }
    Plot.AddPlot(Coordinates, gpwPoints);
  }

  Plot.SavePng(FileName + ".png");
}

void GetHistogram(TVec<DCluster>& Clusters, int *Arr, int Size, int (*Fn)(DCluster& Cluster)) {
  SetZero(Arr, Size);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    int Value = (*Fn)(Clusters[i]);
    if (Value < Size) {
      Arr[Value]++;
    }
  }
}

void GetHistogramRelation(TVec<DCluster>& Clusters, int *Arr, int Size, int (*KeyFn)(DCluster& Cluster), int (*ValueFn)(DCluster& Cluster)) {
  SetZero(Arr, Size);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    int Key = (*KeyFn)(Clusters[i]);
    if (Key < Size) {
      Arr[Key]+= (*ValueFn)(Clusters[i]);
    }
  }
}


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

void ReadDeletedClusters(TVec<DCluster> &Clusters) {
  TStr FileName = "clusters.txt";
  PSIn FileLoader = TFIn::New(FileName);
  TStr CurLn;

  while (FileLoader->GetNextLn(CurLn)) {
    DCluster Cluster(CurLn);
    Clusters.Add(Cluster);
  }

  FileName = "clusters.bin";
  TFOut FOut(FileName);
  Clusters.Save(FOut);
}

void ReadDeletedClustersDetailed(TVec<DCluster> &Clusters) {
  TStr FileName = "detailed_clusters.txt";
  PSIn FileLoader = TFIn::New(FileName);
  TStr CurLn;

  DCluster Cluster;
  bool IsCluster = true;
  while (FileLoader->GetNextLn(CurLn)) {
    if (CurLn == "") {
      Clusters.Add(Cluster);
      Cluster = DCluster(Cluster);
      IsCluster = true;
    } else {
      if (IsCluster) {
        IsCluster = false;
        Cluster = DCluster(CurLn);
      } else {
        DQuote Quote(CurLn);
        Cluster.Quotes.Add(Quote);
      }
    }
  }

  FileName = "deleted_clusters.bin";
  TFOut FOut(FileName);
  Clusters.Save(FOut);
}

void LoadDeletedClustersDetailed(TVec<DCluster> &Clusters) {
  TFIn CurFile("deleted_clusters.bin");
  Clusters.Load(CurFile);
}

void GetRepLenHistogram(TVec<DCluster> &Clusters, int *RepLenHist) {
  SetZero(RepLenHist, MaxWords);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    if (Clusters[i].RepStrLen < MaxWords) {
      RepLenHist[Clusters[i].RepStrLen]++;
    }
  }
}

void GetPopLenHistogram(TVec<DCluster> &Clusters, int *PopLenHist) {
  SetZero(PopLenHist, MaxWords);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    TInt MaxSize = -1;
    TInt MaxIndex = -1;
    for (int j = 0; j < Clusters[i].Quotes.Len(); ++j) {
      if (Clusters[i].Quotes[j].Size > MaxSize) {
        MaxSize = Clusters[i].Quotes[j].Size;
        MaxIndex = j;
      }
    }

    if (MaxIndex >= 0)
      PopLenHist[Clusters[i].Quotes[MaxIndex].StrLen]++;
  }
}

void GetRepLenLifespan(TVec<DCluster> &Clusters, int *RepLenLifespan) {
  SetZero(RepLenLifespan, MaxWords);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    if (Clusters[i].RepStrLen < MaxWords) {
      RepLenLifespan[Clusters[i].RepStrLen]+= Clusters[i].DiffDay;
    }
  }
}

void GetPopLenLifespan(TVec<DCluster> &Clusters, int *PopLenLifespan) {
  SetZero(PopLenLifespan, MaxWords);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    TInt MaxSize = -1;
    TInt MaxIndex = -1;
    for (int j = 0; j < Clusters[i].Quotes.Len(); ++j) {
      if (Clusters[i].Quotes[j].Size > MaxSize) {
        MaxSize = Clusters[i].Quotes[j].Size;
        MaxIndex = j;
      }
    }
    if (MaxIndex >= 0)
      PopLenLifespan[Clusters[i].Quotes[MaxIndex].StrLen]+= Clusters[i].DiffDay;
  }
}

int GetLifespan(DCluster &Cluster) {
  return Cluster.DiffDay;
}

void AnalyzeClustersDetailed(TVec<DCluster> &Clusters) {
  int RepLenHist[MaxWords];
  int RepLenLifespan[MaxWords];
  int PopLenHist[MaxWords];
  int PopLenLifespan[MaxWords];
  int LifespanHist[MaxLifespan];

  GetRepLenHistogram(Clusters, RepLenHist);
  GetRepLenLifespan(Clusters, RepLenLifespan);
  GetPopLenHistogram(Clusters, PopLenHist);
  GetPopLenLifespan(Clusters, PopLenLifespan);
  GetHistogram(Clusters, LifespanHist, MaxLifespan, GetLifespan);

  // GRAPH PLOTTING - Histogramming
  PlotGraph("plot_replen_histogram", "Representative Quote Length Histogram", "representative quote length",
      "number of clusters", NULL, RepLenHist, MaxWords, false, false);
  PlotGraph("plot_poplen_histogram", "Most Popular Quote Length Histogram", "most popular quote length",
      "number of clusters", NULL, PopLenHist, MaxWords, false, false);
  PlotGraph("plot_lifespan_histogram", "Lifespan Histogram", "lifespan (in days)",
      "number of clusters", NULL, LifespanHist, MaxLifespan, false, true);

  // GRAPH PLOTTING - Interesting
  PlotGraph("plot_replen_vs_lifespan", "Representative Quote Length vs. Lifespan", "representative quote length",
      "lifespan (in days)", RepLenHist, RepLenLifespan, MaxWords, true, false);
  PlotGraph("plot_poplen_vs_lifespan", "Most Popular Quote Length vs. Lifespan", "most popular quote length",
      "lifespan (in days)", PopLenHist, PopLenLifespan, MaxWords, true, false);
}

int main(int argc, char *argv[]) {
  /*TStr FileName = "all_deleted_clusters.txt";
  TVec<FCluster> Deleted;
  BuildClusterVec(FileName, Deleted);
  Err("Number of clusters: %d\n", Deleted.Len());*/
  TVec<DCluster> Clusters;
  //ReadDeletedClustersDetailed(Clusters);
  LoadDeletedClustersDetailed(Clusters);
  AnalyzeClustersDetailed(Clusters);
  return 0;
}
