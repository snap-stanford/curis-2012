#include "logoutput.h"
#include "quote.h"
#include "doc.h"
#include "stdafx.h"

const TStr LogOutput::OutputDirectory = "/lfs/1/tmp/curis/output/clustering/";
const TStr LogOutput::WebDirectory = "../../public_html/curis/output/clustering/";
const TStr LogOutput::PercentEdgesDeleted = "PercentEdgesDeleted";
const TStr LogOutput::NumOriginalEdges = "NumOriginalEdges";
const TStr LogOutput::NumRemainingEdges = "NumRemainingEdges";
const TStr LogOutput::NumQuotes = "NumQuotes";
const TStr LogOutput::NumClusters = "NumClusters";

LogOutput::LogOutput() {
  ShouldLog = true;
}

void LogOutput::DisableLogging() {
  ShouldLog = false;
}

void LogOutput::SetupFiles() {
  TSecTm Tm = TSecTm::GetCurTm();
  TimeStamp= TStr::Fmt("%04d-%02d-%02d",  Tm.GetYearN(), Tm.GetMonthN(), Tm.GetDayN());
  TimeStamp += "_" + Tm.GetTmStr();
  //TimeStamp = Tm.GetYmdTmStr() + "_" + Tm.GetTmStr();
  printf("%s %s\n", TimeStamp.CStr(), Tm.GetTmStr().CStr());
  //TStr Command = "mkdir -p " + OutputDirectory + TimeStamp;
  //system(Command.CStr());
  TStr Command = "mkdir -p " + WebDirectory + TimeStamp;
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
  if (!ShouldLog) return;
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
  if (!ShouldLog) return;
  TStr FileName = WebDirectory + TimeStamp + "/top_clusters.txt";
  fprintf(stderr, "Filename: %s", FileName.CStr());
  FILE *F = fopen(FileName.CStr(), "w");
  fprintf(stderr, "F: %p", F);

  fprintf(stderr, "File created\n");
  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    //fprintf(stderr, "Representative quote: %d\n", RepQuotesAndFreq[i].Val1.Val);
    TQuote RepQuote;
    if (QB->GetQuote(RepQuotesAndFreq[i].Val1, RepQuote)) {
      //fprintf(stderr, "Quote obtained!\n");
      TStr RepQuoteStr;
      RepQuote.GetContentString(RepQuoteStr);
      //fprintf(stderr, "Content string obtained!\n");
      TInt FreqOfAllClusterQuotes = RepQuotesAndFreq[i].Val2;
      //fprintf(stderr, "Frequency obtained: %d!\n", FreqOfAllClusterQuotes.Val);
      TIntV QuotesInCluster = RepQuotesAndFreq[i].Val3;
      //fprintf(stderr, "Number of quotes in cluster: %d\n", QuotesInCluster.Len());
      fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());
      for (int j = 0; j < QuotesInCluster.Len(); j++) {
        //fprintf(stderr, "Actual quote: %d\n", QuotesInCluster[j].Val);
        TQuote Quote;
        if (QB->GetQuote(QuotesInCluster[j], Quote)) {
          TStr QuoteStr;
          Quote.GetContentString(QuoteStr);
          fprintf(F, "\t%d\t%s\n", Quote.GetNumSources().Val, QuoteStr.CStr());
        }
      }
    }
  }
  //Save(QuotesFile);
  fclose(F);
}
