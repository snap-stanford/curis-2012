#include "stdafx.h"
#include "clusterplot.h"

ClusterPlot::ClusterPlot(TStr RootDirectory) {
  this->RootDirectory = RootDirectory;
  this->RootDirectory = ""; // TODO: fix this temporary fix
}

/* x-axis: number of different quotes in the cluster, y-axis: number
 * of clusters with x quotes */
void ClusterPlot::PlotClusterSizeUnique(TVec<TIntV>& Clusters) {
  TGnuPlot Plot = TGnuPlot(RootDirectory + "plot_unique_clusters", "Cluster Size Frequency - Unique Quotes", false);
  Plot.SetXYLabel("number of unique quotes", "number of clusters");
  Plot.SetScale(gpsLog2XY);

  Clusters.SortCmp(TCmpIntV(false));
  TInt ClusterLen = Clusters.Len();
  TIntPrV Coordinates;
  TInt LastFrequency = -1;
  for (int i = 0; i < ClusterLen; ++i) {
    int j = i;
    while (j < ClusterLen && Clusters[j].Len() == Clusters[i].Len()) {
      j++;
    }
    Coordinates.Add(TIntPr(Clusters[i].Len(), j - i));
    i = j - 1;
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng();
}

/*2) x-axis: total frequency quotes in the cluster, y-axis: number of
clusters of frequency x
3) x-axis: frequency of a quote, y-axis: number of quotes with freq. x. */
void ClusterPlot::PlotClusterSize(TVec<TCluster>& ClusterSummaries) {
  TGnuPlot Plot = TGnuPlot(RootDirectory + "plot_cluster_size", "Cluster Size Frequency - Total Quotes", false);
  Plot.SetXYLabel("number of quotes", "number of clusters");
  Plot.SetScale(gpsLog2XY);

  TInt ClusterLen = ClusterSummaries.Len();
  TIntPrV Coordinates;
  TInt LastFrequency = -1;
  for (int i = 0; i < ClusterLen; ++i) {
    int j = i;
    while (j < ClusterLen && ClusterSummaries[j].GetNumQuotes() == ClusterSummaries[i].GetNumQuotes()) {
      j++;
    }
    Coordinates.Add(TIntPr(ClusterSummaries[i].GetNumQuotes(), j - i));
    i = j - 1;
  }

  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng();
}

void ClusterPlot::PlotQuoteFrequencies(TQuoteBase* QB) {
  TGnuPlot Plot = TGnuPlot(RootDirectory + "plot_quote_frequencies", "Quote Frequency", false);
  Plot.SetXYLabel("number of sources", "number of quotes");
  Plot.SetScale(gpsLog2XY);

  TIntV QuoteIds;
  QB->GetAllQuoteIds(QuoteIds);

  THash<TInt, TInt> QuoteFrequencies;
  TInt NumQuotes = QuoteIds.Len();
  for (int i = 0; i < QuoteIds.Len(); ++i) {
    TQuote CurQuote; // TODO: Iterator
    QB->GetQuote(QuoteIds[i], CurQuote);
    TInt NumSources = CurQuote.GetNumSources();
    if (QuoteFrequencies.IsKey(NumSources)) {
      QuoteFrequencies.AddDat(NumSources, QuoteFrequencies.GetDat(NumSources) + 1);
    } else {
      QuoteFrequencies.AddDat(NumSources, 1);
    }
  }

  TVec<TPair<TInt, TInt> > Coordinates;
  QuoteFrequencies.GetKeyDatPrV (Coordinates);
  Plot.AddPlot(Coordinates, gpwPoints);
  Plot.SavePng();
}

/*void ClusterPlot::PlotClusterPeaks(Cluster C) {
  if (Sources.Len() == 0) {
    return false;
  }

  TIntFltPrV FreqV;
  TVec<TSecTm> HourOffsets;
  GetFreqVector(DocBase, FreqV, HourOffsets, BucketSize, SlidingWindowSize);

  // Find peaks and add them to the plot
  // Define a peak as anything that is more than one standard deviation above the mean
  TIntFltPrV PeakV;
  TFltV FreqFltV;
  for (int i = 0; i < FreqV.Len(); ++i) {
    //FreqFltV.Add(TFlt(FreqV[i].Val2));
    TFlt AvgDist = 0;
    TInt Count = 0;
    for (int j = i - K; j < i + K && j < FreqV.Len(); j++) {
      if (j >= 0 && j != i) {
        AvgDist += TFlt(FreqV[i].Val2) - TFlt(FreqV[j].Val2);
        Count++;
      }
    }

    FreqFltV.Add(AvgDist/Count);
  }

  TMom M(FreqFltV);
  TFlt FreqMean = TFlt(M.GetMean());
  TFlt FreqStdDev = TFlt(M.GetSDev());

  bool IsPeaking;
  int CurMax;
  for (int i = 0; i < FreqFltV.Len(); ++i) {
    TFlt Freq = TFlt(FreqV[i].Val2);
    if (FreqFltV[i] > FreqMean + FreqStdDev) {
      if (IsPeaking) {
        if (Freq > TFlt(FreqV[CurMax].Val2)) {
          CurMax = i;
        }
      } else {
        IsPeaking = true;
        CurMax = i;
      }
    } else if (IsPeaking) {
      IsPeaking = false;
      PeakV.Add(FreqV[CurMax]);
    }
  }
  // final check so we don't miss the last peak
  if (IsPeaking) {
    PeakV.Add(FreqV[CurMax]);
  }

  TStr ContentStr;
  GetContentString(ContentStr);
  TGnuPlot GP(Filename, "Frequency of Quote " + Id.GetStr() + " Over Time: " + ContentStr);
  GP.SetXLabel(TStr("Hour Offset"));
  GP.SetYLabel(TStr("Frequency of Quote"));
  GP.AddPlot(FreqV, gpwLinesPoints, "Frequency");
  if (PeakV.Len() > 0) {
    GP.AddPlot(PeakV, gpwPoints, "Peaks");
  }
  GP.SavePng();
  return true;
}*/
