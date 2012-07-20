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

TInt TQuote::GetNumDomains(TDocBase &DocBase) {
  THashSet<TStr> DomSet;
  for (int u = 0; u < Sources.Len(); u++) {
    TDoc Doc;
    if(DocBase.GetDoc(Sources[u], Doc)) {
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
  GetPeaks(DocBase, PeakTimesV, TInt(1), TInt(1));
}

void TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize) {
  TFreqTripleV PeakV;
  Peaks::GetPeaks(DocBase, Sources, PeakV, BucketSize, SlidingWindowSize);
  for (int i = 0; i < PeakV.Len(); ++i) {
    PeakTimesV.Add(PeakV[i].Val3);
  }
}

void TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename) {
  GraphFreqOverTime(DocBase, Filename, TInt(1), TInt(1));
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
  TGnuPlot GP("./plots/" + Filename, "Frequency of Quote " + Id.GetStr() + " Over Time: " + ContentStr);
  GP.SetXLabel(TStr("Hour Offset"));
  GP.SetYLabel(TStr("Frequency of Quote"));
  GP.AddPlot(FreqV, gpwLinesPoints, "Frequency");
  if (PeakV.Len() > 0) {
    GP.AddPlot(PeakV, gpwPoints, "Peaks");
  }
  TStr SetXTic = TStr("set xtics 24\nset terminal png small size 1000,800");
  GP.SavePng("./plots/" + Filename + ".png", 1000, 800, TStr(), SetXTic);
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

/// Returns true if one of the quotes is a "substring" of the other, and false otherwise.
//  Compares the parsed content of the quotes, rather than the original content
//  Based off the method TQuoteBs::IsLinkPhrases in memes.cpp of memecluster.
bool TQuoteBase::IsSubstring(TInt QuoteId1, TInt QuoteId2) {
  TQuote Quote1, Quote2;
  GetQuote(QuoteId1, Quote1);
  GetQuote(QuoteId2, Quote2);
  TInt ShortLen = TMath::Mn(Quote1.GetContentNumWords(), Quote2.GetContentNumWords()); // changed from GetParsedContentNumWords
  TStrV ParsedContent1, ParsedContent2;
  Quote1.GetContent(ParsedContent1); // Changed from GetParsedContent
  Quote2.GetContent(ParsedContent2);

  //TStr Str1, Str2;
  //Quote1.GetContentString(Str1);
  //Quote2.GetContentString(Str2);
  //fprintf(stderr, "Short Len: %d - %s - %s\n", ShortLen.Val, Str1.CStr(), Str2.CStr());

  TInt Overlap = LongestSubSequenceOfWords(ParsedContent1, ParsedContent2);
  if (ShortLen <= 5 && Overlap == ShortLen) { fprintf(stderr, "reason A\n"); return true; } // full overlap, no skip
  else if ((ShortLen == 6 && Overlap >= 5 )) { fprintf(stderr, "reason B\n"); return true; }
  else if (Overlap/double(ShortLen+3) > 0.5 || Overlap > 10) { fprintf(stderr, "reason C\n"); return true; }
  return false;
}

/// Finds the length of the approximate longest common subsequence of words
//  Based off the method TQuoteBs::LongestCmnSubSq(const TIntV& WIdV1, const TIntV& WIdV2,
//  int& WIdV1Start, int& WIdV2Start, int& SkipId) in memes.cpp of memecluster
TInt TQuoteBase::LongestSubSequenceOfWords(const TStrV& Content1, const TStrV& Content2) {
  const TStrV& V1 = Content1.Len() > Content2.Len() ? Content1:Content2; // long
  const TStrV& V2 = Content1.Len() > Content2.Len() ? Content2:Content1; // short
  TInt V1Len = V1.Len();
  TInt V2Len = V2.Len();

  THash<TStr, TIntV> SharedWordsH;
  THashSet<TStr> V2WordsSet;

  TInt WordV1Start, WordV2Start, SkipId;
  WordV1Start = WordV2Start = SkipId = 0;
  for (int i = 0; i < V2Len; i++) { // word position index
    V2WordsSet.AddKey(V2[i]);
  }
  for (int i = 0; i < V1Len; i++) { // word position index
    if (V2WordsSet.IsKey(V1[i])) {
      SharedWordsH.AddDat(V1[i]).Add(i);
    }
  }

  // Counts the sequence length
  int MaxLen = 0;
  for (int w = 0; w < V2Len; w++) {
    TStr Word = V2[w];
    if (! SharedWordsH.IsKey(Word)) { continue; }
    TIntV& OccV = SharedWordsH.GetDat(Word);
    for (int o = 0; o < OccV.Len(); o++) {
      //fprintf(stderr, "SERIES A FUNDING\n");
      int beg = OccV[o];
      //fprintf(stderr, "SERIES B FUNDING\n");
      int cnt = 0, tmp = 0;
      while (w+cnt < V2Len && beg+cnt < V1Len && V2[w+cnt]==V1[beg+cnt]) { cnt++; tmp=0; }           // no skip
      //fprintf(stderr, "TECHCRUNCH SPIKE\n");
      while (beg+1+cnt < V1Len && w+cnt < V2Len && V2[w+cnt]==V1[beg+cnt+1]) { cnt++; tmp=-1; }      // skip word in long
      //fprintf(stderr, "REDDIT SUBREDDIT\n");
      while (beg+cnt+1 < V1Len && w+cnt+1 < V2Len && V2[w+cnt+1]==V1[beg+cnt+1]) {  cnt++; tmp=-2;}  // skip word in both
      //fprintf(stderr, "PIT OF DOOM\n");
      while (beg+cnt < V1Len && w+cnt+1 < V2Len && V2[w+cnt+1]==V1[beg+cnt]) { cnt++; tmp=-3;}       // skip word in short
      //fprintf(stderr, "ACQUIRED!\n");
      if (MaxLen < cnt) {
        MaxLen = cnt;
        SkipId=tmp;
        WordV1Start = beg;
        WordV2Start = w;
      }
      //fprintf(stderr, "IPO!\n");
      IAssert(cnt >= 1);
    }
  }
  if (! (Content1.Len()>Content2.Len())) {
    int tmp = WordV1Start;
    WordV1Start = WordV2Start;
    WordV2Start = tmp;
  }
  return MaxLen;
  //return 0;
}
