#include "stdafx.h"
#include "clustering.h"
#include "quotegraph.h"
#include "quote.h"
#include "doc.h"
#include "clusterplot.h"
#include <stdio.h>

void OutputClusterInformation(TQuoteBase* QB, TVec<TPair<TPair<TInt, TInt>, TIntV> >& RepQuotesAndFreq, TStr FileName) {
  FILE *F = fopen(FileName.CStr(), "wt");
  TFOut ClusterFile(FileName);

  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    TQuote RepQuote;
    QB->GetQuote(RepQuotesAndFreq[i].Val1.Val1, RepQuote);
    TStr RepQuoteStr;
    RepQuote.GetContentString(RepQuoteStr);
    TInt FreqOfAllClusterQuotes = RepQuotesAndFreq[i].Val1.Val2;
    TIntV QuotesInCluster = RepQuotesAndFreq[i].Val2;
    fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());
    for (int j = 0; j < QuotesInCluster.Len(); j++) {
      TQuote Quote;
      QB->GetQuote(QuotesInCluster[j], Quote);
      TStr QuoteStr;
      Quote.GetContentString(QuoteStr);
      fprintf(F, "\t%d\t%s\n", Quote.GetNumSources().Val, QuoteStr.CStr());
    }
  }
  //Save(QuotesFile);
  fclose(F);
}

int main(int argc, char *argv[]) {
  // load QB and DB
  TStr BaseString = "/lfs/1/tmp/curis/QBDB.bin";
  TStr OutputString = "TopClusters.txt";
  if (argc >= 2) {
    BaseString = TStr(argv[1]); // format: ./memecluster basename
  }
  TFIn BaseFile(BaseString);

  if (argc >= 3) {
    OutputString = TStr(argv[2]);
  }
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  QB->Load(BaseFile);
  DB->Load(BaseFile);

  // create clusters and save!
  QuoteGraph GraphCreator(QB);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob;
  ClusterJob.SetGraph(QGraph);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, QB);
  TVec<TPair<TPair<TInt, TInt>, TIntV> > RepQuotesAndFreq;
  ClusterJob.SortClustersByFreq(RepQuotesAndFreq, Clusters, QB);
  OutputClusterInformation(QB, RepQuotesAndFreq, OutputString);

  // plot output
  ClusterPlot Plotter(TStr("/lfs/1/tmp/curis/"));
  Plotter.PlotClusterSizeUnique(Clusters);
  Plotter.PlotClusterSize(RepQuotesAndFreq);
  Plotter.PlotQuoteFrequencies(QB);
  delete QB;
  delete DB;
  printf("Done!\n");
  return 0;
}
