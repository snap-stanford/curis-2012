#include "stdafx.h"

int MaxWords = 50;
int MaxLifespan = 200; // half a year
int MaxClusterSize = 200; // hopefully

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
      if (Arr1 == NULL) {
        if (Arr2[i] != 0) {
          Coordinates.Add(TIntPr(i, Arr2[i]));
        }
      }
      else if (Arr2[i] != 0)
        Coordinates.Add(TIntPr(Arr1[i], Arr2[i]));
    }
    Plot.AddPlot(Coordinates, gpwPoints);
  } else {
    TVec<TPair<TInt, TFlt> > Coordinates;
    for (int i = 0; i < NumPoints; ++i) {
      if (Arr1[i] != 0 && Arr2[i] != 0) {
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

void GetHistogramRelationQuotes(TVec<DCluster>& Clusters, int *Arr, int Size, int (*Fn)(DQuote& Cluster), void (*SortFn)(TVec<DQuote>& Quotes)) {
  SetZero(Arr, Size);
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    (*SortFn)(Clusters[i].Quotes);
    for (int j = 0; j < Clusters[i].Unique; ++j) {
      int Value = (*Fn)(Clusters[i].Quotes[j]);
      Arr[j] += Value;
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
      //if (Cluster.Size > 350) {
        Clusters.Add(Cluster);
      //}
      Cluster = DCluster();
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

int GetLifespan(DCluster &Cluster) {
  return Cluster.DiffDay;
}

int GetPopLen(DCluster &Cluster) {
  TInt MaxSize = -1;
  TInt MaxIndex = -1;
  for (int j = 0; j < Cluster.Quotes.Len(); ++j) {
    if (Cluster.Quotes[j].Size > MaxSize) {
      MaxSize = Cluster.Quotes[j].Size;
      MaxIndex = j;
    }
  }

  return Cluster.Quotes[MaxIndex].StrLen;
}

int GetLongLifespan(DCluster &Cluster) {
  return (Cluster.DiffDay > 2) ? Cluster.DiffDay.Val : 0;
}

int GetLongPopLen(DCluster & Cluster) {
  return (Cluster.DiffDay > 2) ? GetPopLen(Cluster) : MaxWords;
}

void SortQuotesBySize(TVec<DQuote>& Quotes) {
  Quotes.SortCmp(TCmpDQuoteBySize(false));
}

void SortQuotesByPeak(TVec<DQuote>& Quotes) {
  Quotes.SortCmp(TCmpDQuoteByPeak(false));
}

void SortQuotesByFirst(TVec<DQuote>& Quotes) {
  Quotes.SortCmp(TCmpDQuoteByFirst(false));
}

int GetQuoteSize(DQuote& Quote) {
  return Quote.Size;
}

int GetQuoteLength(DQuote& Quote) {
  return Quote.StrLen;
}

int GetNumVariants(DCluster& Cluster) {
  return Cluster.Unique;
}

void ComputeCumulative(int *Arr1, int *Arr2, int size) {
  if (size > 0) Arr2[size-1] = Arr1[size-1];
  for (int i = size - 1; i >= 0; --i) {
    Arr2[i] = Arr2[i+1] + Arr1[i];
  }
}

void AnalyzeClustersDetailed(TVec<DCluster> &Clusters) {
  int RepLenHist[MaxWords];
  int RepLenLifespan[MaxWords];
  int PopLenHist[MaxWords];
  int PopLenLifespan[MaxWords];
  int LifespanHist[MaxLifespan];
  int LongPopLenLifespan[MaxLifespan];
  int LongPopLenHist[MaxLifespan];
  int VariantHist[MaxClusterSize];
  int VariantCumulative[MaxClusterSize];
  int VariantQuoteLen[MaxClusterSize];
  int VariantQuotePeak[MaxClusterSize];
  int VariantQuoteFirst[MaxClusterSize];
  int VariantQuoteSize[MaxClusterSize];

  GetRepLenHistogram(Clusters, RepLenHist);
  GetRepLenLifespan(Clusters, RepLenLifespan);
  GetPopLenHistogram(Clusters, PopLenHist);
  GetHistogramRelation(Clusters, PopLenLifespan, MaxWords, GetPopLen, GetLifespan);
  GetHistogram(Clusters, LifespanHist, MaxLifespan, GetLifespan);
  GetHistogram(Clusters, LongPopLenHist, MaxWords, GetLongPopLen);
  GetHistogramRelation(Clusters, LongPopLenLifespan, MaxWords, GetLongPopLen, GetLifespan);
  GetHistogramRelationQuotes(Clusters, VariantQuoteLen, MaxClusterSize, GetQuoteLength, SortQuotesBySize);
  GetHistogram(Clusters, VariantHist, MaxClusterSize, GetNumVariants);
  ComputeCumulative(VariantHist, VariantCumulative, MaxClusterSize);
  GetHistogramRelationQuotes(Clusters, VariantQuotePeak, MaxClusterSize, GetQuoteLength, SortQuotesByPeak);
  GetHistogramRelationQuotes(Clusters, VariantQuoteFirst, MaxClusterSize, GetQuoteLength, SortQuotesByFirst);
  GetHistogramRelationQuotes(Clusters, VariantQuoteSize, MaxClusterSize, GetQuoteSize, SortQuotesByFirst);

  Err("Number of clusters: %d\n", Clusters.Len());
  // GRAPH PLOTTING - Histogramming
  /*PlotGraph("plot_replen_histogram", "Representative Quote Length Histogram", "representative quote length",
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
  PlotGraph("plot_poplen_vs_lifespan_long", "Most Popular Quote Length vs. Lifespan (for longer clusters)", "most popular quote length",
      "lifespan (in days)", LongPopLenHist, LongPopLenLifespan, MaxWords, true, false);*/
  PlotGraph("plot_variant_quote_length", "Popularity of Individual Quotes in Clusters vs. Quote Length", "quote popularity rank",
        "quote length", VariantCumulative, VariantQuoteLen, MaxClusterSize, true, false);
  PlotGraph("plot_variant_quote_peak", "Peak Time of Individual Quotes in Clusters vs. Quote Length", "order of quotes (by peak)",
        "quote length", VariantCumulative, VariantQuotePeak, MaxClusterSize, true, false);
  PlotGraph("plot_variant_quote_first", "Start Time of Individual Quotes in Clusters vs. Quote Length", "order of quotes (by start time)",
        "quote length", VariantCumulative, VariantQuoteFirst, MaxClusterSize, true, false);
  PlotGraph("plot_quote_start_vs_quote_size", "Start Time of Individual Quotes in Clusters vs. Quote Size", "order of quotes (by start time)",
        "quote size", VariantCumulative, VariantQuoteSize, MaxClusterSize, true, false);
}

void GetWhitneyHoustonQuote(TVec<DCluster>& Clusters) {
  int Len = Clusters.Len();
  for (int i = 0; i < Len; ++i) {
    if (Clusters[i].Size == 517 && Clusters[i].Unique == 15) {
      Err("%s\n", Clusters[i].RepStr.CStr());
      TFOut FOut("whitney.bin");
      Clusters[i].Save(FOut);
    }
  }
}

void LoadWhitney(DCluster& Whitney) {
  TFIn CurFile("whitney.bin");
  Whitney.Load(CurFile);
}

void PrintWhitney(DCluster& Whitney) {
  Whitney.Quotes.SortCmp(TCmpDQuoteByFirst(true));

  TGnuPlot Plot = TGnuPlot("whitney_plot", "Quote pertaining to Whitney Houston's death", false);
  //Plot.SetXYLabel(XStr, YStr);

  /*
  -2 a
  -1 small
  0 spoon
  1 with
  2 a
  3 white
  4 crystal
  5 like
  6 substance
  7 in
  8 it
  9 a
  10 rolled
  11 up
  12 piece
  13 of
  14 white
  15 paper
  16 from
  17 off
  18 the
  19 top
  20 of
  21 a
  22 counter
  23 along
  24 the
  25 east
  26 wall
  27 of
  28 the
  29 bathroom
  */

  TIntPrV Coordinates;
  Coordinates.Add(TIntPr(2, 10));
  Coordinates.Add(TIntPr(1, 17));
  Coordinates.Add(TIntPr(2, 31));
  Coordinates.Add(TIntPr(12, 17));
  Coordinates.Add(TIntPr(0, 17));
  Coordinates.Add(TIntPr(1, 17));
  Coordinates.Add(TIntPr(0, 8));
  Coordinates.Add(TIntPr(0, 17));
  Coordinates.Add(TIntPr(5, 8));
  Coordinates.Add(TIntPr(4, 8));
  Coordinates.Add(TIntPr(12, 17));
  Coordinates.Add(TIntPr(1, 10));

  Plot.AddPlot(Coordinates, gpwBoxes);

  Plot.SavePng("whitney_plot.png");
}

int main(int argc, char *argv[]) {
  /*TStr FileName = "all_deleted_clusters.txt";
  TVec<FCluster> Deleted;
  BuildClusterVec(FileName, Deleted);
  Err("Number of clusters: %d\n", Deleted.Len());*/
  //TVec<DCluster> Clusters;
  //ReadDeletedClustersDetailed(Clusters);
  //LoadDeletedClustersDetailed(Clusters);
  //AnalyzeClustersDetailed(Clusters);

  //GetWhitneyHoustonQuote(Clusters);
  DCluster Whitney;
  LoadWhitney(Whitney);
  PrintWhitney(Whitney);

  return 0;
}
