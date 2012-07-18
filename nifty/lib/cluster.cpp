#include "stdafx.h"
#include "cluster.h"
#include "quote.h"
#include "peaks.h"

TCluster::TCluster() {
}

TCluster::TCluster(TInt RepresentativeQuoteId, TInt NumQuotes, const TIntV QuoteIds) {
  // TODO: Check that URLs are not repeated
  this->RepresentativeQuoteId = RepresentativeQuoteId;
  this->NumQuotes = NumQuotes;
  this->QuoteIds = QuoteIds;
}

void TCluster::Save(TSOut& SOut) const {
  RepresentativeQuoteId.Save(SOut);
  NumQuotes.Save(SOut);
  QuoteIds.Save(SOut);
}

void TCluster::Load(TSIn& SIn) {
  RepresentativeQuoteId.Load(SIn);
  NumQuotes.Load(SIn);
  QuoteIds.Load(SIn);
}

TInt TCluster::GetRepresentativeQuoteId() const {
  return RepresentativeQuoteId;
}

TInt TCluster::GetNumQuotes() const {
  return NumQuotes;
}

TInt TCluster::GetNumUniqueQuotes() const {
  return QuoteIds.Len();
}

void TCluster::GetQuoteIds(TIntV &QuoteIds) const {
  QuoteIds = this->QuoteIds;
}

void TCluster::AddQuote(TQuoteBase *QB, const TIntV &QuoteIds) {
  for (int i = 0; i < QuoteIds.Len(); i++) {
    AddQuote(QB, QuoteIds[i]);
  }
}

void TCluster::AddQuote(TQuoteBase *QB,TInt QuoteId) {
  this->QuoteIds.Add(QuoteId);
  TQuote q;
  QB->GetQuote(QuoteId, q);
  this->NumQuotes += q.GetNumSources();
}

void TCluster::SetRepresentativeQuoteId(TInt QuoteId) {
  this->RepresentativeQuoteId = QuoteId;
}

TInt TCluster::GetId() {
  return Id;
}

void TCluster::SetId(TInt Id) {
  this->Id = Id;
}

void TCluster::GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize) {
  TIntV Sources;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Quote;
    if (QuoteBase->GetQuote(QuoteIds[i], Quote)) {
      TIntV CurSources;
      Quote.GetSources(CurSources);
      Sources.AddV(CurSources);
    }
  }

  Peaks::GetPeaks(DocBase, Sources, PeakTimesV, FreqV, BucketSize, SlidingWindowSize);
}

void TCluster::GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename) {
  GraphFreqOverTime(DocBase, QuoteBase, Filename, TInt(1), TInt(1));
}

/// If BucketSize is > 1, a sliding window average will not be calculated
//  Otherwise, if BucketSize = 1, a sliding window average of size SlidingWindowSize will be calculated
void TCluster::GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize) {
  TFreqTripleV PeakTimesV;
  TFreqTripleV FreqTripleV;
  GetPeaks(DocBase, QuoteBase, PeakTimesV, FreqTripleV, BucketSize, SlidingWindowSize);

  TVec<TIntFltPr> PeakV;
  for (int i = 0; i < PeakTimesV.Len(); ++i) {
    PeakV.Add(TIntFltPr(PeakTimesV[i].Val1, PeakTimesV[i].Val2));
  }

  TVec<TIntFltPr> FreqV;
  for (int i = 0; i < FreqTripleV.Len(); ++i) {
    FreqV.Add(TIntFltPr(FreqTripleV[i].Val1, FreqTripleV[i].Val2));
  }

  TStr ContentStr;
  TQuote RepQuote;
  if (QuoteBase->GetQuote(RepresentativeQuoteId, RepQuote)) {
    RepQuote.GetContentString(ContentStr);
  }
  TGnuPlot GP(Filename, "Frequency of Cluster " + Id.GetStr() + " Over Time: " + ContentStr);
  GP.SetXLabel(TStr("Hour Offset"));
  GP.SetYLabel(TStr("Frequency of Cluster"));
  GP.AddPlot(FreqV, gpwLinesPoints, "Frequency");
  if (PeakV.Len() > 0) {
    GP.AddPlot(PeakV, gpwPoints, "Peaks");
  }
  fprintf(stderr, "saving png");
  TStr SetXTic = TStr("set xtics 12\nset terminal png small size 1000,800");
  GP.SavePng(Filename + ".png", 1000, 800, TStr(), SetXTic);
}
