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
const TInt LogOutput::FrequencyCutoff = 300;
const TInt LogOutput::PeakThreshold = 5;

LogOutput::LogOutput() {
  ShouldLog = true;
}

void LogOutput::Save(TSOut& SOut) const {
  OutputValues.Save(SOut);
  TimeStamp.Save(SOut);
  ShouldLog.Save(SOut);
}

void LogOutput::Load(TSIn& SIn) {
  OutputValues.Load(SIn);
  TimeStamp.Load(SIn);
  ShouldLog.Load(SIn);
}

void LogOutput::DisableLogging() {
  ShouldLog = false;
}

void LogOutput::SetupFiles() {
  TSecTm Tm = TSecTm::GetCurTm();

  TimeStamp= TStr::Fmt("%04d-%02d-%02d",  Tm.GetYearN(), Tm.GetMonthN(), Tm.GetDayN());
  TimeStamp += "_" + Tm.GetTmStr();

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
  TStr DiscardedFileName = WebDirectory + TimeStamp + "/discarded_clusters.txt";
  system(Command.CStr());
  FILE *F = fopen(FileName.CStr(), "w");
  FILE *H = fopen(HTMLFileName.CStr(), "w");
  FILE *D = fopen(DiscardedFileName.CStr(), "w");

  // HTML setup
  fprintf(H, "<html>\n");
  fprintf(H, "<head>\n");
  fprintf(H, "<title>Top Clusters</title>\n");
  fprintf(H, "<link href=\"%s\" rel=\"stylesheet\">\n", TWITTER_BOOTSTRAP);
  fprintf(H, "</head>\n");
  fprintf(H, "<body>\n");
  fprintf(H, "<div class=\"page-header\"><center><h1>Top Clusters<br /><small>MESSAGE HERE</small></h1></center></div>\n");
  fprintf(H, "<table border=\"1\" class=\"table table-condensed table-striped\">\n");
  fprintf(H, "<b><tr><td>Rank</td><td>Previous Rank</td><td>Size</td><td>Quote</td></tr></b>\n");
  /*<tr>
  <td>Row 1, cell 1</td>
  <td>Row 1, cell 2</td>
  </tr>*/

  int Rank = 0;

  for (int i = 0; i < ClusterSummaries.Len(); i++) {
    TStr RepQuoteStr;
    ClusterSummaries[i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    TInt FreqOfAllClusterQuotes = ClusterSummaries[i].GetNumQuotes();
    TIntV QuotesInCluster;
    ClusterSummaries[i].GetQuoteIds(QuotesInCluster);
    fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());

    // Write HTML
    if (FreqOfAllClusterQuotes >= FrequencyCutoff) {
      if (i % 100 == 99) {
        fprintf(stderr, "saved %d files so far!\n", i);
      }
      TFreqTripleV PeakTimesV;
      TFreqTripleV FreqV;
      ClusterSummaries[i].GetPeaks(DB, QB, PeakTimesV, FreqV, 2, 1);
      if (PeakTimesV.Len() <= LogOutput::PeakThreshold) {
        ++Rank;
        TStr URLLink = "<a href=\"cluster/" + TInt(i).GetStr() + ".html\">" + RepQuoteStr + "</a>";
        fprintf(H, "<tr><td>%d</td><td>N/A</td><td>%d</td><td>%s</td></tr>\n", Rank, ClusterSummaries[i].GetNumQuotes().Val, URLLink.CStr());
        TStr ClusterFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(Rank).GetStr() + ".html";
        TStr ImageFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(Rank).GetStr();
        ClusterSummaries[i].GraphFreqOverTime(DB, QB, ImageFileName, 2, 1);
        FILE *C = fopen(ClusterFileName.CStr(), "w");
        fprintf(C, "<html>\n");
        fprintf(C, "<head>\n");
        fprintf(C, "<title>%s</title>\n", RepQuoteStr.CStr());
        fprintf(C, "<link href=\"%s\" rel=\"stylesheet\">\n", TWITTER_BOOTSTRAP2);
        fprintf(C, "</head>\n");
        fprintf(C, "<body>\n");
        fprintf(C, "<center>\n");
        fprintf(C, "<div class=\"page-header\"><h2>%s</h2></div>\n", RepQuoteStr.CStr());
        fprintf(C, "<img src=\"%d.png\" /><br />\n", Rank);
        fprintf(C, "</center>\n");
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
      } else {
        fprintf(D, "%d\t%s\n", PeakTimesV.Len(), RepQuoteStr.CStr());
      }

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

  // HTML ending
  fprintf(H, "</table>\n");
  fprintf(H, "</body>\n");
  fprintf(H, "</html>\n");

  //Close files
  fclose(F);
  fclose(H);
}
