#include "stdafx.h"
#include "quote.h"
#include "doc.h"

const uint TQuote::NumSecondsInHour = 3600;
const uint TQuote::NumSecondsInWeek = 604800;

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
  StemAndStopWordsContentString(Content, ParsedContent);
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

void TQuote::StemAndStopWordsContentString(const TStrV &ContentV, TStrV &NewContent) {
  for (int i = 0; i < ContentV.Len(); ++i) {
    if (!StopWordSet->IsIn(ContentV[i], false)) {
      NewContent.Add(TPorterStemmer::StemX(ContentV[i]).GetLc());
    }
  }
}

bool TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV) {
  return GetPeaks(DocBase, PeakTimesV, TInt(1), TInt(1));
}

bool TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize) {
  if (Sources.Len() == 0) {
    return false;
  }

  TIntFltPrV FreqV;
  TVec<TSecTm> HourOffsets;
  GetFreqVector(DocBase, FreqV, HourOffsets, BucketSize, SlidingWindowSize);

  TFltV FreqFltV;
  for (int i = 0; i < FreqV.Len(); ++i) {
    FreqFltV.Add(TFlt(FreqV[i].Val2));
  }

  TMom M(FreqFltV);
  TFlt FreqMean = TFlt(M.GetMean());
  TFlt FreqStdDev = TFlt(M.GetSDev());

  for (int i = 0; i < FreqV.Len(); ++i) {
    TFlt Freq = TFlt(FreqV[i].Val2);
    if (Freq > FreqMean + FreqStdDev) {
      PeakTimesV.Add(HourOffsets[i]);
    }
  }

  // If no peak satisfies the definition, then the "peak" time is
  // the hour with the highest quote frequency
  if (PeakTimesV.Len() == 0) {
    TFlt FreqMax = FreqFltV[0];
    TInt FreqMaxIndex = 0;
    for (int i = 0; i < FreqFltV.Len(); ++i) {
      if (FreqFltV[i] >= FreqMax) {
        FreqMax = FreqFltV[i];
        FreqMaxIndex = i;
      }
    }
    PeakTimesV.Add(HourOffsets[FreqMaxIndex]);
  }
  return true;
}

bool TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename) {
  return GraphFreqOverTime(DocBase, Filename, TInt(1), TInt(1));
}

/// If BucketSize is > 1, a sliding window average will not be calculated
//  Otherwise, if BucketSize = 1, a sliding window average of size SlidingWindowSize will be calculated
bool TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize) {
  if (Sources.Len() == 0) {
    return false;
  }

  TIntFltPrV FreqV;
  TVec<TSecTm> HourOffsets;
  GetFreqVector(DocBase, FreqV, HourOffsets, BucketSize, SlidingWindowSize);

  // Find peaks and add them to the plot
  // Define a peak as anything that is more than one standard deviation above the mean
  TIntFltPrV PeakV;
  TFltV FreqFltV;
  for (int i = 0; i < FreqV.Len(); ++i) {
    FreqFltV.Add(TFlt(FreqV[i].Val2));
  }

  TMom M(FreqFltV);
  TFlt FreqMean = TFlt(M.GetMean());
  TFlt FreqStdDev = TFlt(M.GetSDev());

  for (int i = 0; i < FreqV.Len(); ++i) {
    TFlt Freq = TFlt(FreqV[i].Val2);
    if (Freq > FreqMean + FreqStdDev) {
      PeakV.Add(FreqV[i]);
    }
  }

  printf("Creating the plot...\n");
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
  return true;
}

void TQuote::GetFreqVector(TDocBase *DocBase, TIntFltPrV& FreqV, TVec<TSecTm>& HourOffsets, TInt BucketSize, TInt SlidingWindowSize) {
  //printf("%s\n", "Getting frequency vector");
  TIntV SourcesSorted(Sources);
  SourcesSorted.SortCmp(TCmpDocByDate(true, DocBase));

  // Only consider documents up to a week before the date of the last document
  TDoc LastDoc;
  DocBase->GetDoc(SourcesSorted[SourcesSorted.Len()-1], LastDoc);
  TUInt EndTime = TUInt(LastDoc.GetDate().GetAbsSecs());
  TUInt FirstTimeToConsider = EndTime - NumSecondsInWeek;

  TDoc StartDoc;
  int StartDocIndex = 0;
  DocBase->GetDoc(SourcesSorted[StartDocIndex], StartDoc);
  TUInt StartTime = TUInt(StartDoc.GetDate().GetAbsSecs());

  while (StartTime < FirstTimeToConsider) {
    ++StartDocIndex;
    DocBase->GetDoc(SourcesSorted[StartDocIndex], StartDoc);
    StartTime = TUInt(StartDoc.GetDate().GetAbsSecs());
  }

  TInt Freq = TInt(1);
  TInt HourNum = 0;
  uint BucketSizeSecs = NumSecondsInHour * BucketSize.Val;
  TIntV FreqAbs;

  for (int i = StartDocIndex+1; i < SourcesSorted.Len(); ++i) {
    TDoc CurrDoc;
    DocBase->GetDoc(SourcesSorted[i], CurrDoc);
    TUInt CurrTime = TUInt(CurrDoc.GetDate().GetAbsSecs());
    if (CurrTime - StartTime < BucketSizeSecs) {
      // Increment the number of quotes seen in this hour-long period by one
      Freq += 1;
    } else {
      HourOffsets.Add(TSecTm(StartTime));
      FreqAbs.Add(Freq);
      FreqV.Add(TIntFltPr(HourNum, CalcWindowAvg(FreqAbs, SlidingWindowSize)));
      TInt NumHoursAhead = (CurrTime - StartTime) / BucketSizeSecs;
      //printf("PrevDoc Date: %s, CurrDoc Date: %s, NumHoursAhead: %d\n", PrevDoc.GetDate().GetYmdTmStr().GetCStr(), CurrDoc.GetDate().GetYmdTmStr().GetCStr(), NumHoursAhead.Val);
      // Add frequencies of 0 if there are hours in between the two occurrences
      for (int j = 1; j < NumHoursAhead; ++j) {
        TUInt NewStartTime = StartTime + j * BucketSizeSecs;
        HourOffsets.Add(TSecTm(NewStartTime));
        FreqAbs.Add(TInt(0));
        FreqV.Add(TIntFltPr(HourNum + j, CalcWindowAvg(FreqAbs, SlidingWindowSize)));
      }
      HourNum += NumHoursAhead;
      //printf("HourNum: %d\n", HourNum.Val);
      Freq.Val = 1;
      StartTime = StartTime + ((CurrTime - StartTime) / BucketSizeSecs) * BucketSizeSecs;
    }
  }
  HourOffsets.Add(TSecTm(StartTime));
  FreqAbs.Add(TInt(0));
  FreqV.Add(TIntFltPr(HourNum, CalcWindowAvg(FreqAbs, SlidingWindowSize)));
}

TFlt TQuote::CalcWindowAvg(TIntV& FreqV, TInt SlidingWindowSize) {
  TInt StartIndex = FreqV.Len() - SlidingWindowSize;
  if (StartIndex < 0) {
    StartIndex = 0;
  }
  TFlt Avg = 0;
  for (int i = StartIndex; i < FreqV.Len(); i++) {
    Avg += FreqV[i];
  }
  Avg /= (FreqV.Len() - StartIndex);
  return Avg;
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
