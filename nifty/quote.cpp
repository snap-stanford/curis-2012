#include "stdafx.h"
#include "quote.h"

TQuote::TQuote(TInt Id, TStrVP Content) {
  this->Id = Id;
  this->Content = Content;
}

TQuote::TQuote(TInt Id, TStr ContentString) {
  this->Id = Id;
	Content = TQuote::ParseContentString(ContentString);
}

void TQuote::AddSource(TInt SourceId) {
  Sources.Add(SourceId); // TODO: check to see if adding duplicate source?
}

TIntV TQuote::GetSources() {
  return Sources;
}

TStrVP TQuote::ParseContentString(TStr ContentString) {
  TStrVP NewStr;
  return NewStr;
}

TQuoteBase::TQuoteBase() {
  QuoteIdCounter = 0;
  //IdToTQuotes = PHash<TInt, TQuote*>::New();
  //QuoteToId = PHash<TStrVP, TInt>::New();
}

/// Adds quote string to quote base; returns quote's quote id
void TQuoteBase::AddQuote(TStr ContentString) {
  TStrVP ContentVectorString = TQuote::ParseContentString(ContentString);
  TInt QuoteId = GetQuoteId(ContentVectorString);
  if (IdToTQuotes.H.IsKey(QuoteId)) {
    return; // nothing to do here; quote is already in database
  } else {
    // otherwise, create the new TQuote and proceed.
    TQuote* NewQuote = new TQuote(QuoteId, ContentVectorString);
    IdToTQuotes.H.AddDat(QuoteId, NewQuote);
  }
}

void TQuoteBase::RemoveQuote(TInt QuoteId) {

}

TInt TQuoteBase::GetQuoteId(TStrVP Content) {
  if (QuoteToId.H.IsKey(Content)) {
    return QuoteToId.H.GetDat(Content);
  } else {
    // this will increment QuoteIdCounter after returning.
    TInt NewId = QuoteIdCounter;
    QuoteIdCounter++;
    QuoteToId.H.AddDat(Content, NewId);
    return NewId;
  }
}

TQuote* TQuoteBase::GetQuote(TInt QuoteId) {
  if (IdToTQuotes.H.IsKey(QuoteId)) {
    return IdToTQuotes.H.GetDat(QuoteId);
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
