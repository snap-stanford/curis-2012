#include "stdafx.h"
#include "clusterplot.h"

ClusterPlot::ClusterPlot(TStr& RootDirectory) {
  this->RootDirectory = RootDirectory;
}

void ClusterPlot::PlotAllGraphs() {
  PlotClusterSizeUnique();
  PlotClusterSize();
  PlotQuoteFrequencies();
}

void calculate_distribution(const TVec<THash<TInt, TInt> >& list,
TIntPrV& ret) {
  TIntV t;
  for (int i = 0; i < list.Len(); i++) {
    if (list[i].Len() != 0)
      t.Add(list[i].Len());
  }
  t.Sort();

  for (int i = 0; i < t.Len(); i++) {
    int j = i;
    while(j < t.Len() && t[j] == t[i])
      j++;
    ret.Add(TIntPr(t[i], j - i));
    i = j - 1;
  }
}

/* x-axis: number of different quotes in the cluster, y-axis: number
 * of clusters with x quotes */
void ClusterPlot::PlotClusterSizeUnique(TVec<TIntV> Clusters) {
  TGnuPlot Plot = TGnuPlot(RootDirectory + "plot_unique_clusters", "Cluster Size Frequency - Unique Quotes", false);
  Plot.SetXYLabel("number of unique quotes", "number of clusters");
  SetScale(gpsLog2XY);

  RepQuotesAndFreq.SortCmp(TCmp<TIntV>(false));
  TInt ClusterLen = RepQuotesAndFreq.Len();
  TIntPrV Coordinates;
  int count = 0;
  TInt LastFrequency = -1;
  for (int i = 0; i < ClusterLen; ++i) {
    int j = i;
    while (j < ClusterLen && RepQuotesAndFreq[j].Len() == RepQuotesAndFreq[i].Len()) {
      j++;
    }
    Coordinates.Add(TIntPr(RepQuotesAndFreq[i], j - i));
    i = j - 1;
  }

  Plot.AddPlot(Coordinates, gpwLinesPoints);
  Plot.SavePng();
}

/*2) x-axis: total frequency quotes in the cluster, y-axis: number of
clusters of frequency x
3) x-axis: frequency of a quote, y-axis: number of quotes with freq. x. */
void ClusterPlot::PlotClusterSize(TVec<TPair<TQuote, TInt> > RepQuotesAndFreq) {
  TGnuPlot Plot = TGnuPlot(RootDirectory + "plot_cluster_size", "Cluster Size Frequency - Total Quotes", false);
  Plot.SetXYLabel("number of unique quotes", "number of clusters");
  SetScale(gpsLog2XY);

  TInt ClusterLen = RepQuotesAndFreq.Len();
  TIntPrV Coordinates;
  int count = 0;
  TInt LastFrequency = -1;
  for (int i = 0; i < ClusterLen; ++i) {
    int j = i;
    while (j < ClusterLen && RepQuotesAndFreq[j].Val2 == RepQuotesAndFreq[i].Val2) {
      j++;
    }
    Coordinates.Add(TIntPr(RepQuotesAndFreq[i], j - i));
    i = j - 1;
  }

  Plot.AddPlot(Coordinates, gpwLinesPoints);
  Plot.SavePng();
}

void ClusterPlot::PlotQuoteFrequencies(TQuoteBase* QB) {
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
  Plot.AddPlot(Coordinates, gpwLinesPoints);
  Plot.SavePng();
}
