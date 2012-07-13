#include "logoutput.h"
#include "quote.h"
#include "doc.h"
#include "stdafx.h"

const TStr LogOutput::OutputDirectory = "/lfs/1/tmp/curis/output/clustering/";
const TStr LogOutput::WebDirectory = "/u/cysuen/public_html/curis/output/clustering/";
const TStr LogOutput::PercentEdgesDeleted = "PercentEdgesDeleted";
const TStr LogOutput::NumOriginalEdges = "NumOriginalEdges";
const TStr LogOutput::NumRemainingEdges = "NumRemainingEdges";
const TStr LogOutput::NumQuotes = "NumQuotes";
const TStr LogOutput::NumClusters = "NumClusters";

void LogOutput::SetupFiles() {
  TSecTm Tm = TSecTm::GetCurTm();
  TimeStamp= TStr::Fmt("%04d-%02d-%02d",  Tm.GetYearN(), Tm.GetMonthN(), Tm.GetDayN());
  TimeStamp += "_" + Tm.GetTmStr();
  //TimeStamp = Tm.GetYmdTmStr() + "_" + Tm.GetTmStr();
  printf("%s %s\n", TimeStamp.CStr(), Tm.GetTmStr().CStr());
  TStr Command = "mkdir " + OutputDirectory + TimeStamp;
  //system(Command.CStr());
  //TStr Command = "mkdir " + WebDirectory + TimeStamp;
  system(Command.CStr());
}

void LogOutput::LogValue(const TStr Key, TStr Value) {
  OutputValues.AddDat(Key, Value);
}

void LogOutput::LogValue(const TStr Key, TInt Value) {
  OutputValues.AddDat(Key, Value.GetStr());
}

void LogOutput::LogValue(const TStr Key, TFlt Value) {
  OutputValues.AddDat(Key, Value.GetStr());
}

void LogOutput::WriteClusteringOutputToFile() {
  TStr FileName = WebDirectory + TimeStamp + "/clustering_info.txt";
  FILE *F = fopen(FileName.CStr(), "w");

  TStrV Keys;
  OutputValues.GetKeyV(Keys);

  for (int i = 0; i < Keys.Len(); ++i) {
    fprintf(F, "%s\t%s\n", Keys[i].CStr(), OutputValues.GetDat(Keys[i]).CStr());
    printf("%s\t%s\n", Keys[i].CStr(), OutputValues.GetDat(Keys[i]).CStr());
  }

  fclose(F);
}

void LogOutput::OutputClusterInformation(TQuoteBase* QB, TVec<TTriple<TInt, TInt, TIntV> >& RepQuotesAndFreq) {
  TStr FileName = WebDirectory + TimeStamp + "/top_clusters.txt";
  FILE *F = fopen(FileName.CStr(), "w");

  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    TQuote RepQuote;
    QB->GetQuote(RepQuotesAndFreq[i].Val1, RepQuote);
    TStr RepQuoteStr;
    RepQuote.GetContentString(RepQuoteStr);
    TInt FreqOfAllClusterQuotes = RepQuotesAndFreq[i].Val2;
    TIntV QuotesInCluster = RepQuotesAndFreq[i].Val3;
    fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());
    for (int j = 0; j < QuotesInCluster.Len(); j++) {
      TQuote Quote;
      QB->GetQuote(QuotesInCluster[j], Quote);
      TStr QuoteStr;
      Quote.GetContentString(QuoteStr);
      fprintf(F, "\t%d\t%s\n", Quote.GetNumSources().Val, QuoteStr.CStr());
    }
  }
  //Save(QuotesFile);
  fclose(F);
}
