#include "stdafx.h"
#include "quote.h"
//#include "resources/stemming.h"
//#include "../../snap/glib-core/stemming.h"

TQuote::TQuote() {
}

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

void TQuote::Save(TSOut& SOut) const {
  Id.Save(SOut);
  Content.Save(SOut);
  ParsedContent.Save(SOut);
  Sources.Save(SOut);
}

void TQuote::Load(TSIn& SIn) {
  Id.Load(SIn);
  Content.Load(SIn);
  ParsedContent.Load(SIn);
  Sources.Load(SIn);
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
  //printf("Source added. Source ID: %d. Num sources: %d", SourceId, Sources.Len().Val());
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
  TStrV NewContent;
  for (int i = 0; i < ContentV.Len(); ++i) {
    if (!StopWordSet->IsIn(ContentV[i], false)) {
      NewContent.Add(TPorterStemmer::StemX(ContentV[i]).GetLc());
    }
  }
  return NewContent;
}

TQuoteBase::TQuoteBase() {
  QuoteIdCounter = 0;
}

/// Adds quote string to quote base; returns quote's quote id
TQuote TQuoteBase::AddQuote(TStr ContentString) {
  TStrV ContentVectorString = TQuote::ParseContentString(ContentString);
  TInt QuoteId = GetNewQuoteId(ContentVectorString);
  if (IdToTQuotes.IsKey(QuoteId)) {
    return IdToTQuotes.GetDat(QuoteId); // nothing to do here; quote is already in database
  } else {
    // otherwise, create the new TQuote and proceed.
    //printf("%d: %s\n", QuoteId.Val, ContentString.CStr());
    TQuote NewQuote(QuoteId, ContentVectorString);
    IdToTQuotes.AddDat(QuoteId, NewQuote);
    return NewQuote;
  }
}

TQuote TQuoteBase::AddQuote(TStr ContentString, TInt SourceId) {
  //TQuote NewQuote = AddQuote(ContentString);
  //NewQuote.AddSource(SourceId);
  //return NewQuote;

  TStrV ContentVectorString = TQuote::ParseContentString(ContentString);
  TInt QuoteId = GetNewQuoteId(ContentVectorString);
  if (IdToTQuotes.IsKey(QuoteId)) {
    TQuote CurQuote =  IdToTQuotes.GetDat(QuoteId); // nothing to do here; quote is already in database
    CurQuote.AddSource(SourceId);
    IdToTQuotes.AddDat(QuoteId, CurQuote);
    return CurQuote;
  } else {
    // otherwise, create the new TQuote and proceed.
    //printf("%d: %s\n", QuoteId.Val, ContentString.CStr());
    TQuote NewQuote(QuoteId, ContentVectorString);
    NewQuote.AddSource(SourceId);
    IdToTQuotes.AddDat(QuoteId, NewQuote);
    return NewQuote;
  }
}

void TQuoteBase::RemoveQuote(TInt QuoteId) {
  // TODO: memory management
  if (IdToTQuotes.IsKey(QuoteId)) {
    TQuote CurQuote = IdToTQuotes.GetDat(QuoteId);
    if (QuoteToId.IsKey(CurQuote.GetContent())) {
      QuoteToId.DelKey(CurQuote.GetContent());
    }
    IdToTQuotes.DelKey(QuoteId);
  }
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

bool TQuoteBase::GetQuote(TInt QuoteId, TQuote& RefQ) {
  if (IdToTQuotes.IsKey(QuoteId)) {
    RefQ = IdToTQuotes.GetDat(QuoteId);
    return true;
  }
  return false;
}

int TQuoteBase::Len() {
  return IdToTQuotes.Len();
}

void TQuoteBase::GetAllQuoteIds(TIntV& KeyV) {
  IdToTQuotes.GetKeyV(KeyV);
}

void TQuoteBase::Save(TSOut& SOut) const {
  QuoteIdCounter.Save(SOut);
  IdToTQuotes.Save(SOut);
  QuoteToId.Save(SOut);
}

void TQuoteBase::Load(TSIn& SIn) {
  QuoteIdCounter.Load(SIn);
  IdToTQuotes.Load(SIn);
  QuoteToId.Load(SIn);
}
