#include "analyzer.h"
#include "quote.h"

TStr DCluster::GetClusterString(TQuoteBase *QB, TDocBase *DB, TCluster& C, TFreqTripleV &FreqV, TInt NumPeaks, TStr &End) {
  TStr Response;
  TSecTm Start;
  TStr RepStr, RepURL;
  C.GetBirthDate(Start);
  C.GetRepresentativeQuoteString(RepStr, QB);
  C.GetRepresentativeQuoteURL(QB, DB, RepURL);
  TStrV QuoteV;
  RepStr.SplitOnStr(" ", QuoteV);

  TSecTm First, Last, Peak;
  DCluster::GetFMP(FreqV, First, Last, Peak);

  Response = Start.GetDtYmdStr() + "\t" + End + "\t";
  Response += C.GetNumUniqueQuotes().GetStr() + "\t" + C.GetNumQuotes().GetStr() + "\t";
  Response += NumPeaks.GetStr() + "\t" + TInt(QuoteV.Len()).GetStr() + "\t";
  Response += RepStr + "\t" + RepURL + "\t";
  Response += First.GetYmdTmStr() + "\t" + Last.GetYmdTmStr() + "\t" + Peak.GetYmdTmStr();
  Response += TBool::GetStr(TBool(C.IsArchived()));

  return Response;
}

DCluster::DCluster(TStr LineInput) {
  // Parse start/end date
  TStrV Params;
  LineInput.SplitOnStr("\t", Params);
  Start = TSecTm::GetDtTmFromYmdHmsStr(Params[0]);
  End = TSecTm::GetDtTmFromYmdHmsStr(Params[1]);
  uint StartDay = Start.GetInUnits(tmuDay);
  uint EndDay = End.GetInUnits(tmuDay);
  DiffDay = EndDay - StartDay + 1;

  // Stats/size
  Unique = TInt(Params[2].GetInt());
  Size = TInt(Params[3].GetInt());
  NumPeaks = TInt(Params[4].GetInt());
  PopStrLen = TInt(Params[5].GetInt());

  // Strings
  RepStr = Params[6];
  RepURL = Params[7];

  // Peak times
  First = TSecTm::GetDtTmFromYmdHmsStr(Params[8]);
  Last = TSecTm::GetDtTmFromYmdHmsStr(Params[9]);
  Peak = TSecTm::GetDtTmFromYmdHmsStr(Params[10]);
  bool ArchiveBool = true;
  Params[11].IsBool(ArchiveBool);
  Archived = TBool(ArchiveBool);
}

void DCluster::GetFMP(TFreqTripleV& FreqV, TSecTm& First, TSecTm& Last, TSecTm& Peak) {
  TInt FreqLength = FreqV.Len();
  TInt MaxVal = -1, MinIndex = -1, MaxIndex = 0;
  for (int i = 0; i < FreqLength; ++i) {
    // calculate first and last.
    if (FreqV[i].Val2 > 0) {
      MaxIndex = i;
      if (MinIndex < 0) {
        MinIndex = i;
      }
    }
    // calculate peak
    if (FreqV[i].Val2 > MaxVal) {
      MaxVal = FreqV[i].Val2;
      Peak = FreqV[i].Val3;
    }
  }
  if (MinIndex >= 0) {
    First = FreqV[MinIndex].Val3;
  } else {
    Err("Uh oh!\n");
    First = FreqV[0].Val3;
  }
  Last = FreqV[MaxIndex].Val3;
}

TStr DQuote::GetQuoteString(TDocBase *DB, TQuote& Quote, TSecTm &PresentTime) {
  TFreqTripleV FreqV, PeakV;
  TIntV Sources;
  Quote.GetSources(Sources);
  Peaks::GetPeaks(DB, Sources, PeakV, FreqV, PEAK_BUCKET, PEAK_WINDOW, PresentTime);
  TSecTm First, Last, Peak;
  DCluster::GetFMP(FreqV, First, Last, Peak);

  TStr Str, RepURL;
  Quote.GetContentString(Str);
  Quote.GetRepresentativeUrl(DB, RepURL);

  TStr Response = Quote.GetNumSources().GetStr() + "\t" + TInt(PeakV.Len()).GetStr() + "\t" + Quote.GetContentNumWords().GetStr() + "\t";
  Response += Str + "\t" + RepURL + "\t";
  Response += First.GetYmdTmStr() + "\t" + Last.GetYmdTmStr() + "\t" + Peak.GetYmdTmStr();

  return Response;
}
