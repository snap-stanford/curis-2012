#include "logoutput.h"
#include "quote.h"
#include "doc.h"

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

void LogOutput::OutputClusterInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime) {
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
  fprintf(H, "<div class=\"page-header\"><center><h1>Top Clusters<br /><small>MESSAGE HERE</small></h1></center></div>\n");
  fprintf(H, "<table border=\"1\" class=\"table table-condensed table-striped\">\n");
  fprintf(H, "<b><tr><td>Rank</td><td>Previous Rank</td><td>Size</td><td>Quote</td></tr></b>\n");
  /*<tr>
  <td>Row 1, cell 1</td>
  <td>Row 1, cell 2</td>
  </tr>*/

  int Rank = 0;

  for (int i = 0; i < ClusterIds.Len(); i++) {
    TCluster Cluster;
    CB->GetCluster(ClusterIds[i], Cluster);
    TStr RepQuoteStr;
    Cluster.GetRepresentativeQuoteString(RepQuoteStr, QB);
    TInt FreqOfAllClusterQuotes = Cluster.GetNumQuotes();
    TIntV QuotesInCluster;
    Cluster.GetQuoteIds(QuotesInCluster);
    fprintf(F, "%d\t%d\t%s\n", FreqOfAllClusterQuotes.Val, QuotesInCluster.Len(), RepQuoteStr.CStr());

    // Write HTML
    if (FreqOfAllClusterQuotes >= FrequencyCutoff) {
      if (i % 100 == 99) {
        fprintf(stderr, "saved %d files so far!\n", i);
      }
      ++Rank;
      TStr URLLink = "<a href=\"cluster/" + TInt(Rank).GetStr() + ".html\">" + RepQuoteStr + "</a>";
      fprintf(H, "<tr><td>%d</td><td>N/A</td><td>%d</td><td>%s</td></tr>\n", Rank, Cluster.GetNumQuotes().Val, URLLink.CStr());
      TStr ClusterFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(Rank).GetStr() + ".html";
      TStr ImageFileName = WebDirectory + TimeStamp + "/cluster/" + TInt(Rank).GetStr();
      Cluster.GraphFreqOverTime(DB, QB, ImageFileName, 2, 1, PresentTime);
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
      fprintf(C, "<table border=\"1\" width=\"75%%\" class=\"table table-condensed table-striped\">\n");
      fprintf(C, "<b><tr><td>Frequency</td><td>Quote</td></tr></b>\n");
      for (int j = 0; j < QuotesInCluster.Len(); j++) {
        TQuote Quote;
        if (QB->GetQuote(QuotesInCluster[j], Quote)) {
          TStr QuoteStr, QuoteRepUrl;
          Quote.GetContentString(QuoteStr);
          Quote.GetRepresentativeUrl(DB, QuoteRepUrl);
          fprintf(C, "<tr><td>%d</td><td><a href=\"%s\">%s</a><br />\n", Quote.GetNumSources().Val, QuoteRepUrl.CStr(), QuoteStr.CStr());
          fprintf(C, "<a data-toggle=\"collapse\" href=\"#quoteUrls%d\">Urls</a><br />\n", Quote.GetId().Val);
          TIntV QuoteSources;
          Quote.GetSources(QuoteSources);
          fprintf(C, "<div style=\"padding-left: 10px;\" id=\"quoteUrls%d\" class=\"collapse\">", Quote.GetId().Val);
          for (int k = 0; k < QuoteSources.Len(); k++) {
            TDoc Doc;
            DB->GetDoc(QuoteSources[k], Doc);
            TStr DocUrl;
            Doc.GetUrl(DocUrl);
            fprintf(C, "%s<br />\n", DocUrl.CStr());
          }
          fprintf(C, "</div></td></tr>\n");
        }
      }
      fprintf(C, "</table>\n");
      fprintf(C, "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js\"></script>");
      fprintf(C, "<script src=\"%s\"></script>\n", TWITTER_BOOTSTRAP_JS);
      fprintf(C, "<script type=\"text/javascript\" charset=\"utf-8\"> $(\".collapse\").collapse({toggle: false}); </script>\n");
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

  // HTML ending
  fprintf(H, "</table>\n");
  fprintf(H, "</body>\n");
  fprintf(H, "</html>\n");

  //Close files
  fclose(F);
  fclose(H);
}

void LogOutput::OutputDiscardedClusters(TQuoteBase *QB, TVec<TCluster>& DiscardedClusters) {
  TStr DiscardedFileName = WebDirectory + TimeStamp + "/discarded_clusters.txt";
  FILE *D = fopen(DiscardedFileName.CStr(), "w");

  for (int i = 0; i < DiscardedClusters.Len(); ++i) {
    TStr RepQuoteStr;
    DiscardedClusters[i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    fprintf(D, "%d\t%s\n", 0, RepQuoteStr.CStr()); // TODO: Add num clusters somehow
  }

  fclose(D);
}
