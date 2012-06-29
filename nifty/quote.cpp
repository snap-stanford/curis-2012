#include "stdafx.h"
#include "quote.h"
//#include "resources/stemming.h"
//#include "../../snap/glib-core/stemming.h"

TQuote::TQuote(TInt Id, TStrV& Content) {
  this->Id = Id;
  this->Content = Content; // TODO: deep copy?
  this->ParsedContent = TQuote::StemAndStopWordsContentString(Content);
}

TQuote::TQuote(TInt Id, TStr ContentString) {
  this->Id = Id;
	Content = TQuote::ParseContentString(ContentString);
	ParsedContent = TQuote::StemAndStopWordsContentString(Content);
}

TStrV TQuote::GetContent() {
  return Content;
}

TStr TQuote::GetContentString() {
  TStr String;
    for (int i = 0; i < Content.Len(); ++i) {
        if (i > 0)  String.InsStr(String.Len()," ");
        String.InsStr(String.Len(), Content[i]);
    }
    return String;
}

TStrV TQuote::GetParsedContent() {
  return ParsedContent;
}

TStr TQuote::GetParsedContentString() {
  TStr ParsedString;
  for (int i = 0; i < ParsedContent.Len(); ++i) {
      if (i > 0)  ParsedString.InsStr(ParsedString.Len()," ");
      ParsedString.InsStr(ParsedString.Len(), ParsedContent[i]);
  }
  return ParsedString;
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

TStrV TQuote::StemAndStopWordsContentString(TStrV ContentV) {
  /*for (int i = 0; i < ContentV.Len(); ++i) {
    TStr CurString = ContentV[i];
    TPorterStemmer TPort;
    TPorterStemmer::TestMethod();
    TInt J = 3;
    TStr test = TPorterStemmer::StemX(CurString);
    //TStr test = TPorterStemmer::StemX(ContentV[i]);
    //ContentV[i] = TPorterStemmer::StemX(ContentV[i]);
  }*/
  return ContentV;
}

TQuoteBase::TQuoteBase() {
  QuoteIdCounter = 0;
}

/// Adds quote string to quote base; returns quote's quote id
TQuote* TQuoteBase::AddQuote(TStr ContentString) {
  TStrV ContentVectorString = TQuote::ParseContentString(ContentString);
  TInt QuoteId = GetNewQuoteId(ContentVectorString);
  if (IdToTQuotes.IsKey(QuoteId)) {
    return IdToTQuotes.GetDat(QuoteId); // nothing to do here; quote is already in database
  } else {
    // otherwise, create the new TQuote and proceed.
    TQuote* NewQuote = new TQuote(QuoteId, ContentVectorString);
    //printf("%d: %s\n", QuoteId.Val, ContentString.CStr());
    IdToTQuotes.AddDat(QuoteId, NewQuote);
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
  if (IdToTQuotes.IsKey(QuoteId)) {
    TQuote* CurQuote = IdToTQuotes.GetDat(QuoteId);
    if (QuoteToId.IsKey(CurQuote->GetContent())) {
      QuoteToId.DelKey(CurQuote->GetContent());
    }
    IdToTQuotes.DelKey(QuoteId);
  }
}

void TQuoteBase::RemoveQuote(TQuote* Quote) {
  RemoveQuote(Quote->GetId());
}

TInt TQuoteBase::GetNewQuoteId(TStrV& Content) {
  if (QuoteToId.IsKey(Content)) {
    return QuoteToId.GetDat(Content);
  } else {
    // this will increment QuoteIdCounter after returning.
    TInt NewId = QuoteIdCounter;
    QuoteIdCounter++;
    QuoteToId.AddDat(Content, NewId);
    return NewId;
  }
}

TInt TQuoteBase::GetQuoteId(TStrV& Content) {
  if (QuoteToId.IsKey(Content)) {
    return QuoteToId.GetDat(Content);
  } else {
    return -1;
  }
}

TQuote* TQuoteBase::GetQuote(TInt QuoteId) {
  if (IdToTQuotes.IsKey(QuoteId)) {
    return IdToTQuotes.GetDat(QuoteId);
  } else {
    return NULL;
  }
}

int TQuoteBase::Len() {
  return IdToTQuotes.Len();
}

void TQuoteBase::GetAllQuoteIds(TIntV& KeyV) {
  IdToTQuotes.GetKeyV(KeyV);
}
