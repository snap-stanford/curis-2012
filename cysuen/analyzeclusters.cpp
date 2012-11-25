#include "stdafx.h"

TInt ExtractClusters(TStr FileName, THash<TStr, TInt>& Clusters, THash<TInt, TStrSet>& ClusterSet) {
  PSIn FileLoader = TFIn::New(FileName);
  TInt ClusterCount = 0;

  // ###### EXTRACT MEMETRACKER CLUSTERS.
  TStr CurLn;
  while (FileLoader->GetNextLn(CurLn)) {
    TStrV Params;
    CurLn.SplitOnStr("\t", Params);
    if (CurLn == "") continue; // wtf
    //Err("1-%d: \"%s\"\n", Params.Len(), CurLn.CStr());
    TInt ClusterID = TInt(Params[3].GetInt());
    TStrSet StrSet;
    TInt NumClusters = TInt(Params[0].GetInt());
    ClusterCount++;
    for (int i = 0; i < NumClusters; i++) {
    //while(true) {
      if (!FileLoader->GetNextLn(CurLn)) break;
      if (CurLn == "") break;
      TStrV Tokens;
      CurLn.SplitOnStr("\t", Tokens);
      //Err("%d: %s\n", Tokens.Len(), CurLn.CStr());

      StrSet.AddKey(Tokens[3]);
      Clusters.AddDat(Tokens[3], ClusterID);
    }
    ClusterSet.AddDat(ClusterID, StrSet);
    //MemeLoader->GetNextLn(CurLn);
  }
  return ClusterCount;
}

TFlt EvaluateSet(THash<TStr, TInt>& Gold, THash<TInt, TStrSet>& Test, TInt& Correct, TInt& Total, TInt& QuoteCorrect, TInt& QuoteTotal) {
  TIntV TestKeys;
  Test.GetKeyV(TestKeys);
  Correct = 0;
  Total = 0;
  QuoteCorrect = 0;
  QuoteTotal = 0;
  for (int i = 0; i < TestKeys.Len(); i++) {
    TStrSet Cluster = Test.GetDat(TestKeys[i]);
    // F**k it I'm tired. I am the queen of late night terrible/questionable coding practices!!! :D :D :D

    THashSet<TInt> ClusterIds;
    TStrV ClusterStrings;
    Cluster.GetKeyV(ClusterStrings);

    for (int j = 0; j < ClusterStrings.Len(); j++) {
      TInt ClusterId = -1;
      Gold.IsKeyGetDat(ClusterStrings[j], ClusterId);
      ClusterIds.AddKey(ClusterId);
    }

    if (ClusterIds.Len() == 1 && ClusterIds.IsKey(-1)) {
      continue;
    }
    //Err("%s\n", ClusterStrings[0].CStr());
    Total++;
    QuoteTotal += ClusterStrings.Len();
    if (ClusterIds.Len() == 1) {// || ClusterIds.Len() == 2 && ClusterIds.IsKey(-1)) {
      Correct++;
      QuoteCorrect += ClusterStrings.Len();
    }
  }

  return Correct * 1.0 / Total;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    Err("Error: needs 2 arguments: memetracker, nifty");
  }

  PSIn MemeLoader = TFIn::New(TStr(argv[1]));
  PSIn NiftyLoader = TFIn::New(TStr(argv[2]));

  THash<TStr, TInt> MemeClusters;
  THash<TStr, TInt> NiftyClusters;
  THash<TInt, TStrSet> MemeClustersSet;
  THash<TInt, TStrSet> NiftyClustersSet;
  TInt MemeClusterCount = ExtractClusters(TStr(argv[1]), MemeClusters, MemeClustersSet);
  TInt NiftyClusterCount = ExtractClusters(TStr(argv[2]), NiftyClusters, NiftyClustersSet);
  Err("Number of MT clusters: %d\n", MemeClusterCount.Val);
  Err("Number of MT quotes: %d\n", MemeClusters.Len());
  Err("Number of nifty clusters: %d\n", NiftyClusterCount.Val);
  Err("Number of nifty quotes: %d\n", NiftyClusters.Len());

  TInt MTCorrect, MTTotal, NiftyCorrect, NiftyTotal;
  TInt MTQuoteCorrect, MTQuoteTotal, NiftyQuoteCorrect, NiftyQuoteTotal;
  TFlt MTVal = EvaluateSet(NiftyClusters, MemeClustersSet, MTCorrect, MTTotal, MTQuoteCorrect, MTQuoteTotal);
  TFlt NiftyVal = EvaluateSet(MemeClusters, NiftyClustersSet, NiftyCorrect, NiftyTotal, NiftyQuoteCorrect, NiftyQuoteTotal);

  TFlt MTQuoteVal = MTQuoteCorrect * 1.0 / MTQuoteTotal;
  TFlt NiftyQuoteVal = NiftyQuoteCorrect * 1.0 / NiftyQuoteTotal;


  Err("MT: %f (%d/%d)\n", MTVal.Val, MTCorrect.Val, MTTotal.Val);
  Err("Nifty: %f (%d/%d)\n", NiftyVal.Val, NiftyCorrect.Val, NiftyTotal.Val);

  Err("Quote MT: %f (%d/%d)\n", MTQuoteVal.Val, MTQuoteCorrect.Val, MTQuoteTotal.Val);
  Err("Quote Nifty: %f (%d/%d)\n", NiftyQuoteVal.Val, NiftyQuoteCorrect.Val, NiftyQuoteTotal.Val);
  return 0;
}
