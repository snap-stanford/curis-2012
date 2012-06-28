#include "stdafx.h"
#include "quote.h"

TQuote::TQuote(TInt Id, TStrV& Content) {
  this->Id = Id;
  this->Content = Content; // TODO: deep copy?
}

TQuote::TQuote(TInt Id, TStr ContentString) {
  this->Id = Id;
	Content = TQuote::ParseContentString(ContentString);
}

TStrV TQuote::GetContent() {
  return Content;
}

TInt TQuote:: GetId() {
  return Id;
}

void TQuote::AddSource(TInt SourceId) {
  Sources.Add(SourceId); // TODO: check to see if adding duplicate source?
}

TIntV TQuote::GetSources() {
  return Sources;
}

TStrV TQuote::ParseContentString(TStr ContentString) {
  TStrV ParsedString;
  ContentString.SplitOnAllAnyCh(" ", ParsedString);
  return ParsedString;
}

TQuoteBase::TQuoteBase() {
  QuoteIdCounter = 0;
  //IdToTQuotes = PHash<TInt, TQuote*>::New();
  //QuoteToId = PHash<TStrVP, TInt>::New();
}

/// Adds quote string to quote base; returns quote's quote id
TQuote* TQuoteBase::AddQuote(TStr ContentString) {
  TStrV ContentVectorString = TQuote::ParseContentString(ContentString);
  TInt QuoteId = GetNewQuoteId(ContentVectorString);
  if (IdToTQuotes.H.IsKey(QuoteId)) {
    return IdToTQuotes.H.GetDat(QuoteId); // nothing to do here; quote is already in database
  } else {
    // otherwise, create the new TQuote and proceed.
    TQuote* NewQuote = new TQuote(QuoteId, ContentVectorString);
    IdToTQuotes.H.AddDat(QuoteId, NewQuote);
    return NewQuote;
  }
}

TQuote* TQuoteBase::AddQuote(TStr ContentString, TInt SourceId) {
  TQuote* NewQuote = AddQuote(ContentString);
  NewQuote->AddSource(SourceId);
  return NewQuote;
}

void TQuoteBase::RemoveQuote(TInt QuoteId) {
  // TODO: memory management
  if (IdToTQuotes.H.IsKey(QuoteId)) {
    TQuote* CurQuote = IdToTQuotes.H.GetDat(QuoteId);
    if (QuoteToId.H.IsKey(CurQuote->GetContent())) {
      QuoteToId.H.DelKey(CurQuote->GetContent());
    }
    IdToTQuotes.H.DelKey(QuoteId);
  }
}

void TQuoteBase::RemoveQuote(TQuote* Quote) {
  RemoveQuote(Quote->GetId());
}

TInt TQuoteBase::GetNewQuoteId(TStrV& Content) {
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

TInt TQuoteBase::GetQuoteId(TStrV& Content) {
  if (QuoteToId.H.IsKey(Content)) {
    return QuoteToId.H.GetDat(Content);
  } else {
    return -1;
  }
}

TQuote* TQuoteBase::GetQuote(TInt QuoteId) {
  if (IdToTQuotes.H.IsKey(QuoteId)) {
    return IdToTQuotes.H.GetDat(QuoteId);
  } else {
    return NULL;
  }
}

TInt TQuoteBase::Len() {
  return IdToTQuotes.H.Len();
}
