#include "quote.h"
#include "stdafx.h"

Quote::TQuote(TInt Id, TStrVP Content) {
  this->Id = Id;
	this->Content = Content;
}

Quote::TQuote(TInt Id, TStr ContentString) {
  this->Id = Id;
	this->Content = Quote::ParseContentString(ContentString);
}

void Quote::AddSource(TInt SourceId) {
  Sources.Add(SourceId); // TODO: check to see if adding duplicate source?
}

PStrV Quote::GetSources() {
  return Sources;
}

static TStrVP Quote::ParseContentString(TStr ContentString) {
  return "place holder";
}

#pragma mark QuoteBase
TQuoteBase::TQuoteBase() {
  IdToTQuotes = PHash<TInt, TQuote*>::New();
  QuoteToId = PHash<TStrVP, TInt>::New();
}

void TQuoteBase::AddQuote(TStr ContentString) {
  TInt QuoteId = GetQuoteId(ContentString);
}

void TQuoteBase::RemoveQuote(TInt QuoteId) {

}

TInt TQuoteBase::GetQuoteId(TStrVP Content) {
  if (QuoteToId->H.isKey(Content)) {
    return QuoteToId->H.getDat(Content);
  } else {
    // this will increment QuoteIdCounter after returning.
    return QuoteToId++;
  }
}

TQuote TQuoteBase::GetQuote(TInt QuoteId) {
  if (IdToTQuotes->H.isKey(QuoteId)) {
    return IdToTQuotes->H.getDat(QuoteId);
  } else {
    return NULL;
  }
}

// Remove all punctuation in the quotes and replace with space
// TODO: rewrite this to blacklist, not white list
void TQuoteBase::QuoteFilter(TStr &QtStr) {
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ?!()@#=&,.<>/\\:\";{}|", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}
