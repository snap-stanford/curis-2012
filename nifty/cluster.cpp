#include "stdafx.h"
#include "cluster.h"
#include "quote.h"

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

TInt TCluster::GetRepresentativeQuoteId() {
  return RepresentativeQuoteId;
}

TInt TCluster::GetNumQuotes() {
  return NumQuotes;
}

TInt TCluster::GetNumUniqueQuotes() {
  return QuoteIds.Len();
}

void TCluster::GetQuoteIds(TIntV &QuoteIds) {
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
