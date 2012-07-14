#include "stdafx.h"

int main(int argc, char *argv[]) {
  TStr BaseString = "/lfs/1/tmp/curis/week/QBDB.bin";
  TFIn BaseFile(BaseString);
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  QB->Load(BaseFile);
  DB->Load(BaseFile);

  TIntV QuoteIds;
  QB->GetAllQuoteIds(QuoteIds);

  int NumQuotes = QuoteIds.Len();
  THash<TInt, TStrSet> PeakCounts;
  for (int i = 0; i < NumQuotes; i++) {
    TQuote CurQuote;
    if (QB->GetQuote(QuoteIds[i], CurQuote)) {
      TVec<TSecTm> Peaks;
      CurQuote.GetPeaks(DB, Peaks);
      TStr QuoteString;
      CurQuote.GetParsedContentString(QuoteString);
      TStrSet StringSet;
      if (PeakCounts.IsKey(Peaks.Len())) {
        StringSet = PeakCounts.GetDat(Peaks.Len());
      }
      StringSet.AddKey(QuoteString);
      PeakCounts.AddDat(Peaks.Len(), StringSet);
    }
  }

  TIntV PeakCountKeys;
  PeakCounts.GetKeyV(PeakCountKeys);
  PeakCountKeys.Sort(true);
  for (int i = 0; i < PeakCountKeys.Len(); i++) {
    TStrSet CurSet = PeakCounts.GetDat(PeakCountKeys[i]);
    if (CurSet.Len() > 0) {
      printf("QUOTES WITH %d PEAKS\n", PeakCountKeys[i].Val);
      printf("#########################################\n");
      THashSet<TStr> StringSet = PeakCounts.GetDat(PeakCountKeys[i]);
      for (THashSet<TStr>::TIter l = StringSet.BegI(); l < StringSet.EndI(); l++) {
        printf("%s\n", l.GetKey().CStr());
      }
      printf("\n");
    }
  }
  delete QB;
  delete DB;
  return 0;
}
