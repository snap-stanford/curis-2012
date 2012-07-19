#include "stdafx.h"

int main(int argc, char *argv[]) {
  TIntV QuoteIds;
  QuoteIds.Add(TInt(1));
  QuoteIds.Add(TInt(42));
  TCluster Cluster(TInt(3), TInt(1), QuoteIds);

  { TFOut FOut("tmp.bin"); Cluster.Save(FOut); }

  TFIn FIn("tmp.bin");
  TCluster NewCluster;
  NewCluster.Load(FIn);

  fprintf(stderr, "Representative Quote Id: %d", NewCluster.GetRepresentativeQuoteId().Val);
  
  return 0;
}
