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

void LogOutput::OutputClusterInformation(TDocBase *DB, TQuoteBase* QB, TVec<TCluster>& ClusterSummaries) {
  fprintf(stderr, "printing stuff\n");
  if (!ShouldLog) return;
  TStr FileName = WebDirectory + TimeStamp + "/top_clusters.txt";
  TStr HTMLFileName = WebDirectory + TimeStamp + "/clusters.html";
  TStr Command = "mkdir -p " + WebDirectory + TimeStamp + "/cluster";
  system(Command.CStr());
  FILE *F = fopen(FileName.CStr(), "w");
  FILE *H = fopen(HTMLFileName.CStr(), "w");

  // HTML setup
  fprintf(H, "<html>\n");
  fprintf(H, "<head>\n");
  fprintf(H, "<title>Top Clusters</title>\n");
  fprintf(H, "<link href=\"%s\" rel=\"stylesheet\">\n", TWITTER_BOOTSTRAP);
  fprintf(H, "</head>\n");
  fprintf(H, "<body>\n");
  fprintf(H, "<div class=\"page-header\"><h1>Top Clusters</h1></div>\n");
  fprintf(H, "<table border=\"1\" class=\"table table-condensed table-striped\">\n");
  fprintf(H, "<b><tr><td>Rank</td><td>Previous Rank</td><td>Quote</td></tr></b>\n");
  /*<tr>
  <td>Row 1, cell 1</td>
  <td>Row 1, cell 2</td>
  </tr>*/


  for (int i = 0; i < ClusterSummaries.Len(); i++) {
    TQuote RepQuote;
    if (QB->GetQuote(ClusterSummaries[i].GetRepresentativeQuoteId(), RepQuote)) {
      TStr RepQuoteStr;
      RepQuote.GetContentString(RepQuoteStr);
      TInt FreqOfAllClusterQuotes = ClusterSummaries[i].GetNumQuotes();
      TIntV QuotesInCluster;
      ClusterSummaries[i].GetQuoteIds(QuotesInCluster);
      fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());

      // Write HTML
      if (FreqOfAllClusterQuotes >= FrequencyCutoff) {
        TStr URLLink = "<a href=\"cluster/" + TInt(i).GetStr() + ".html\">" + RepQuoteStr + "</a>";
        fprintf(H, "<tr><td>%d</td><td>N/A</td><td>%s</td></tr>\n", i, URLLink.CStr());
        TStr ClusterFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(i).GetStr() + ".html";
        TStr ImageFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(i).GetStr();
        ClusterSummaries[i].GraphFreqOverTime(DB, QB, ImageFileName, 2, 1);
        FILE *C = fopen(ClusterFileName.CStr(), "w");
        fprintf(C, "<html>\n");
        fprintf(C, "<head>\n");
        fprintf(C, "<title>%s</title>\n", RepQuoteStr.CStr());
        fprintf(C, "<link href=\"%s\" rel=\"stylesheet\">\n", TWITTER_BOOTSTRAP);
        fprintf(C, "</head>\n");
        fprintf(C, "<body>\n");
        fprintf(C, "<h2>%s</h2><br />\n", RepQuoteStr.CStr());
        fprintf(C, "<h2>%d</h2><br /><br />\n", FreqOfAllClusterQuotes.Val);
        fprintf(C, "<img src=\"%d.png\" /><br />\n", i);
        for (int j = 0; j < QuotesInCluster.Len(); j++) {
          TQuote Quote;
          if (QB->GetQuote(QuotesInCluster[j], Quote)) {
            TStr QuoteStr;
            Quote.GetContentString(QuoteStr);
            fprintf(C, "\t%d\t%s<br />\n", Quote.GetNumSources().Val, QuoteStr.CStr());
          }
        }
        fprintf(C, "</body>\n");
        fprintf(C, "</html>\n");
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
  fprintf(H, "</table>\n");
  fprintf(H, "</body>\n");
  fprintf(H, "</html>\n");

  //Close files
  fclose(F);
  fclose(H);
}
