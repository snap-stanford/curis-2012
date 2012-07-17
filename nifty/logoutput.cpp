#include "logoutput.h"
#include "quote.h"
#include "doc.h"
#include "stdafx.h"

const TStr LogOutput::OutputDirectory = "/lfs/1/tmp/curis/output/clustering/";
const TStr LogOutput::WebDirectory = "../../public_html/curis/output/clustering/";
const TStr LogOutput::PercentEdgesDeleted = "PercentEdgesDeleted";
const TStr LogOutput::PercentEdgesDeletedNotFromSubgraphs = "PercentEdgesDeletedNotFromSubgraphs";
const TStr LogOutput::PercentEdgesDeletedNFSBaseline = "PercentEdgesDeletedNFSBaseline";
const TStr LogOutput::NumOriginalEdges = "NumOriginalEdges";
const TStr LogOutput::NumRemainingEdges = "NumRemainingEdges";
const TStr LogOutput::NumQuotes = "NumQuotes";
const TStr LogOutput::NumClusters = "NumClusters";
const TInt LogOutput::FrequencyCutoff = 400;

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
  TStr HTMLFileName = WebDirectory + TimeStamp + "/clusters.html";
  TStr Command = "mkdir -p " + WebDirectory + TimeStamp + "/cluster";
  system(Command.CStr());
  FILE *F = fopen(FileName.CStr(), "w");
  FILE *H = fopen(HTMLFileName.CStr(), "w");

  // HTML setup
  fprintf(H, "<html>");
  fprintf(H, "<head>");
  fprintf(H, "<title>Top Clusters</title>");
  fprintf(H, "</head>");
  fprintf(H, "<body>");
  fprintf(H, "<table border=\"1\">");
  fprintf(H, "<b><tr><td>Rank</td><td>Previous Rank</td><td>Quote</td></tr></b>");
  /*<tr>
  <td>Row 1, cell 1</td>
  <td>Row 1, cell 2</td>
  </tr>*/


  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    TQuote RepQuote;
    if (QB->GetQuote(RepQuotesAndFreq[i].Val1, RepQuote)) {
      TStr RepQuoteStr;
      RepQuote.GetContentString(RepQuoteStr);
      TInt FreqOfAllClusterQuotes = RepQuotesAndFreq[i].Val2;
      TIntV QuotesInCluster = RepQuotesAndFreq[i].Val3;
      fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());

      // Write HTML
      if (FreqOfAllClusterQuotes >= FrequencyCutoff) {
        TStr URLLink = "<a href=\"cluster/" + TInt(i).GetStr() + ".html\">" + RepQuoteStr + "</a>";
        fprintf(H, "<tr><td>%d</td><td>N/A</td><td>%s</td></tr>", i, URLLink.CStr());
        TStr ClusterFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(i).GetStr() + ".html";
        FILE *C = fopen(ClusterFileName.CStr(), "w");
        fprintf(C, "<html>");
        fprintf(C, "<head>");
        fprintf(C, "<title>%s</title>", RepQuoteStr.CStr());
        fprintf(C, "</head>");
        fprintf(C, "<body>");
        fprintf(C, "<h2>%s</h2><br />", RepQuoteStr.CStr());
        fprintf(C, "<h2>%d</h2><br /><br />", FreqOfAllClusterQuotes.Val);
        for (int j = 0; j < QuotesInCluster.Len(); j++) {
          TQuote Quote;
          if (QB->GetQuote(QuotesInCluster[j], Quote)) {
            TStr QuoteStr;
            Quote.GetContentString(QuoteStr);
            fprintf(C, "\t%d\t%s<br />", Quote.GetNumSources().Val, QuoteStr.CStr());
          }
        }
        fprintf(C, "</body>");
        fprintf(C, "</html>");
        fclose(C);
      }

      // Write quote information
      for (int j = 0; j < QuotesInCluster.Len(); j++) {
        TQuote Quote;
        if (QB->GetQuote(QuotesInCluster[j], Quote)) {
          TStr QuoteStr;
          Quote.GetContentString(QuoteStr);
          fprintf(F, "\t%d\t%s\n", Quote.GetNumSources().Val, QuoteStr.CStr());
        }
      }
    }
  }

  // HTML ending
  fprintf(H, "</table>");
  fprintf(H, "</body>");
  fprintf(H, "</html>");

  //Close files
  fclose(F);
  fclose(H);
}
