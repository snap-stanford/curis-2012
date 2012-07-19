#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "peaks.h"

PSwSet TQuote::StopWordSet = new TSwSet(swstEnMsdn);

TQuote::TQuote() {
}

TQuote::TQuote(TInt Id, const TStrV& Content) {
  this->Id = Id;
  this->Content = Content; // TODO: deep copy?
  Init();
}

TQuote::TQuote(TInt Id, const TStr &ContentString) {
  this->Id = Id;
	ParseContentString(ContentString, Content);
	Init();
}

void TQuote::Init() {
  TStr FilteredContentString;
  GetContentString(FilteredContentString);
  FilterSpacesAndSetLowercase(FilteredContentString);
  TStrV FilteredContent;
  ParseContentString(FilteredContentString, FilteredContent);
  StemAndStopWordsContentString(FilteredContent, ParsedContent);
  for (int i = 0; i < ParsedContent.Len(); ++i) {
    if (i > 0) this->ParsedContentString.InsStr(ParsedContentString.Len()," ");
    this->ParsedContentString.InsStr(ParsedContentString.Len(), ParsedContent[i]);
  }
}

void TQuote::Save(TSOut& SOut) const {
  Id.Save(SOut);
  Content.Save(SOut);
  ParsedContent.Save(SOut);
  ParsedContentString.Save(SOut);
  Sources.Save(SOut);
}

void TQuote::Load(TSIn& SIn) {
  Id.Load(SIn);
  Content.Load(SIn);
  ParsedContent.Load(SIn);
  ParsedContentString.Load(SIn);
  Sources.Load(SIn);
}


void TQuote::GetContent(TStrV &Ref) {
  Ref = Content;
}

void TQuote::GetContentString(TStr &Ref) {
  for (int i = 0; i < Content.Len(); ++i) {
    if (i > 0)  Ref.InsStr(Ref.Len()," ");
    Ref.InsStr(Ref.Len(), Content[i]);
  }
}

void TQuote::GetParsedContent(TStrV &Ref) {
  Ref = ParsedContent;
}

void TQuote::GetParsedContentString(TStr &Ref) {
  Ref = ParsedContentString;
}

TInt TQuote::GetContentNumWords() {
  return Content.Len();
}

TInt TQuote::GetParsedContentNumWords() {
  return ParsedContent.Len();
}

TInt TQuote::GetId() {
  return Id;
}

TInt TQuote::GetNumDomains(TDocBase *DocBase) {
  THashSet<TStr> DomSet;
  for (int u = 0; u < Sources.Len(); u++) {
    TDoc Doc;
    if(DocBase->GetDoc(Sources[u], Doc)) {
      TStr DocUrl;
      Doc.GetUrl(DocUrl);
      DomSet.AddKey(TStrUtil::GetDomNm(DocUrl));
    }
  }
  return DomSet.Len();
}

TInt TQuote::GetNumSources() {
  return Sources.Len();
}

void TQuote::AddSource(TInt SourceId) {
  Sources.Add(SourceId); // TODO: check to see if adding duplicate source?
  //printf("Source added. Source ID: %d. Num sources: %d", SourceId, Sources.Len().Val());
}

void TQuote::GetSources(TIntV &RefS) {
  RefS = Sources;
}

void TQuote::ParseContentString(const TStr &ContentString, TStrV &ParsedString) {
  ContentString.SplitOnAllAnyCh(" ", ParsedString);
}

void TQuote::FilterSpacesAndSetLowercase(TStr &QtStr) {
  // Three passes...hopefully this isn't too slow.
  TChA QtChA = TChA(QtStr);
  for (int i = 0; i < QtChA.Len(); i++) {
    if (!(isalpha(QtChA[i]) || QtChA[i] == '\'')) {
      QtChA[i] = ' ';
    } else {
      QtChA[i] = tolower(QtChA[i]);
    }
  }
  QtStr = TStr(QtChA);
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}

void TQuote::StemAndStopWordsContentString(const TStrV &ContentV, TStrV &NewContent) {
  for (int i = 0; i < ContentV.Len(); ++i) {
    if (!StopWordSet->IsIn(ContentV[i], false)) {
      NewContent.Add(TPorterStemmer::StemX(ContentV[i]).GetLc());
    }
  }
}

void TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV) {
  return GetPeaks(DocBase, PeakTimesV, TInt(1), TInt(1));
}

void TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize) {
  TFreqTripleV PeakV;
  Peaks::GetPeaks(DocBase, Sources, PeakV, BucketSize, SlidingWindowSize);
  for (int i = 0; i < PeakV.Len(); ++i) {
    PeakTimesV.Add(PeakV[i].Val3);
  }
}

void TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename) {
  return GraphFreqOverTime(DocBase, Filename, TInt(1), TInt(1));
}

/// If BucketSize is > 1, a sliding window average will not be calculated
//  Otherwise, if BucketSize = 1, a sliding window average of size SlidingWindowSize will be calculated
void TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize) {
  TFreqTripleV PeakTimesV;
  TFreqTripleV FreqTripleV;
  Peaks::GetPeaks(DocBase, Sources, PeakTimesV, FreqTripleV, BucketSize, SlidingWindowSize);

  TVec<TIntFltPr> PeakV;
  for (int i = 0; i < PeakTimesV.Len(); ++i) {
    PeakV.Add(TIntFltPr(PeakTimesV[i].Val1, PeakTimesV[i].Val2));
  }

  TVec<TIntFltPr> FreqV;
  for (int i = 0; i < FreqTripleV.Len(); ++i) {
    FreqV.Add(TIntFltPr(FreqTripleV[i].Val1, FreqTripleV[i].Val2));
  }

  TStr ContentStr;
  GetContentString(ContentStr);
  TGnuPlot GP(Filename, "Frequency of Quote " + Id.GetStr() + " Over Time: " + ContentStr);
  GP.SetXLabel(TStr("Hour Offset"));
  GP.SetYLabel(TStr("Frequency of Quote"));
  GP.AddPlot(FreqV, gpwLinesPoints, "Frequency");
  if (PeakV.Len() > 0) {
    GP.AddPlot(PeakV, gpwPoints, "Peaks");
  }
  GP.SavePng();
}

TQuoteBase::TQuoteBase() {
  QuoteIdCounter = 0;
}

/// Adds quote string to quote base; returns quote's quote id
TQuote TQuoteBase::AddQuote(const TStr &ContentString) {
  TStrV ContentVectorString;
  TQuote::ParseContentString(ContentString, ContentVectorString);
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

TQuote TQuoteBase::AddQuote(const TStr &ContentString, TInt SourceId) {
  //TQuote NewQuote = AddQuote(ContentString);
  //NewQuote.AddSource(SourceId);
  //return NewQuote;

  TStrV ContentVectorString;
  TQuote::ParseContentString(ContentString, ContentVectorString);
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
    TStrV CurContent;
    CurQuote.GetContent(CurContent);
    if (QuoteToId.IsKey(CurContent)) {
      QuoteToId.DelKey(CurContent);
    }
    IdToTQuotes.DelKey(QuoteId);
  }
}

TInt TQuoteBase::GetNewQuoteId(const TStrV& Content) {
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

TInt TQuoteBase::GetQuoteId(const TStrV &Content) {
  if (QuoteToId.IsKey(Content)) {
    return QuoteToId.GetDat(Content);
  } else {
    return -1;
  }
}

bool TQuoteBase::GetQuote(TInt QuoteId, TQuote &RefQ) {
  if (IdToTQuotes.IsKey(QuoteId)) {
    RefQ = IdToTQuotes.GetDat(QuoteId);
    return true;
  }
  return false;
}

int TQuoteBase::Len() {
  return IdToTQuotes.Len();
}

void TQuoteBase::GetAllQuoteIds(TIntV &KeyV) {
  IdToTQuotes.GetKeyV(KeyV);
}

void TQuoteBase::Save(TSOut &SOut) const {
  QuoteIdCounter.Save(SOut);
  IdToTQuotes.Save(SOut);
  QuoteToId.Save(SOut);
}

void TQuoteBase::Load(TSIn& SIn) {
  QuoteIdCounter.Load(SIn);
  IdToTQuotes.Load(SIn);
  QuoteToId.Load(SIn);
}
