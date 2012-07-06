#include "stdafx.h"
#include "clustering.h"
#include "quotegraph.h"
#include "quote.h"
#include "doc.h"
#include <stdio.h>

void OutputClusterInformation(TQuoteBase* QuoteBase, TVec<TPair<TStr, TInt> >& RepQuotesAndFreq, TStr FileName) {
  FILE *F = fopen(FileName.CStr(), "wt");
  TFOut ClusterFile(FileName);

  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    fprintf(F, "%d\t%s\n", RepQuotesAndFreq[i].Val2.Val, RepQuotesAndFreq[i].Val1.CStr());
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
  TVec<TPair<TStr, TInt> > RepQuotesAndFreq;
  ClusterJob.SortClustersByFreq(RepQuotesAndFreq, Clusters, QB);
  OutputClusterInformation(QB, RepQuotesAndFreq, OutputString);
  delete QB;
  delete DB;
  printf("Done!\n");
  return 0;
}
