#include "stdafx.h"
#include "spinn3r.h"
#include "memes.h"

int main() {
  PQuoteBs QtBs = TQuoteBs::New();

  TStr InFNm = "/lfs/1/tmp/curis/QuoteList.bin";

  TVec<TSecTm> PubTmV;
  TVec<TStr> PostUrlV;
  TVec<TStr> QuoteV;

  TFIn FIn(InFNm);
  PubTmV.Load(FIn);
  PostUrlV.Load(FIn);
  QuoteV.Load(FIn);

  for (int i = 0; i < QuoteV.Len(); i++) {
    QtBs->AddQuote2(QuoteV[i], PostUrlV[i], PubTmV[i]);
  }

  printf("SAVE: %d quotes\n", QtBs->Len());
  { TFOut FOut("/lfs/1/tmp/curis/memetracker.QtBs"); QtBs->Save(FOut); }
  return 0;
}
