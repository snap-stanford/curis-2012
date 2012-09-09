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

void LogOutput::SetupNewOutputDirectory() {
  if (!ShouldLog) return;
  TSecTm Tm = TSecTm::GetCurTm();

  TimeStamp = Tm.GetDtYmdStr() + "_" + Tm.GetTmStr();
  printf("Setting up new output directory at %s\n", (WebDirectory + TimeStamp).CStr());

  TStr Command = "mkdir -p " + WebDirectory + TimeStamp + "/output";
  system(Command.CStr());
  Command = "mkdir -p " + WebDirectory + TimeStamp + "/web";
  system(Command.CStr());
}

void LogOutput::SetDirectory(const TStr &Directory) {
  TimeStamp = Directory;
}

void LogOutput::GetDirectory(TStr& Directory) {
  Directory = this->TimeStamp;
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

void LogOutput::WriteClusteringOutputToFile(TSecTm& Date) {
  if (!ShouldLog) return;
  TStr FileName = WebDirectory + TimeStamp + "/output/clustering_info_" + Date.GetDtYmdStr() + ".txt";
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
  TIntV OldTopClusters;
  OutputClusterInformation(DB, QB, CB, ClusterIds, PresentTime, OldTopClusters);
}

void LogOutput::OutputClusterInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime, TIntV &OldTopClusters) {
  if (!ShouldLog) return;
  TStr CurDateString = PresentTime.GetDtYmdStr();
  fprintf(stderr, "Writing cluster information ...\n");
  TStr FileName = WebDirectory + TimeStamp + "/output/top_clusters_" + CurDateString + ".txt";
  TStr HTMLFileName = WebDirectory + TimeStamp + "/web/clusters_" + CurDateString + ".html";
  TStr Command = "mkdir -p " + WebDirectory + TimeStamp + "/web/cluster_" + CurDateString;
  system(Command.CStr());
  FILE *F = fopen(FileName.CStr(), "w");
  FILE *H = fopen(HTMLFileName.CStr(), "w");

  // PREVIOUS RANKING SETUP
  THash<TInt, TInt> OldRankings;
  if (OldTopClusters.Len() > 0) {
    for (int i = 0; i < OldTopClusters.Len(); i++) {
      OldRankings.AddDat(OldTopClusters[i], i + 1);
    }
  }

  // HTML setup
  fprintf(H, "<html>\n");
  fprintf(H, "<head>\n");
  fprintf(H, "<title>%s</title>\n", CurDateString.CStr());
  fprintf(H, "<link href=\"%s\" rel=\"stylesheet\">\n", TWITTER_BOOTSTRAP);
  fprintf(H, "</head>\n");
  fprintf(H, "<body>\n");
  TSecTm PrevDay = PresentTime;
  PrevDay.AddDays(-1);
  TStr PrevDayLink = "clusters_" + PrevDay.GetDtYmdStr() + ".html";
  TSecTm NextDay = PresentTime;
  NextDay.AddDays(1);
  TStr NextDayLink = "clusters_" + NextDay.GetDtYmdStr() + ".html";
  TStr LogMessage;
  if (!OutputValues.IsKeyGetDat("notes", LogMessage)) LogMessage = "";
  fprintf(H, "<div class=\"page-header\"><center><h1>\n");
  fprintf(H, "<a href=\"%s\">&laquo;</a> &middot; Top Clusters for %s &middot; <a href=\"%s\">&raquo;</a><h1><br />\n", PrevDayLink.CStr(), CurDateString.CStr(), NextDayLink.CStr());
  fprintf(H, "<small>%s</small>\n", LogMessage.CStr());
  fprintf(H, "</h1></center></div>\n");
  fprintf(H, "<table border=\"1\" class=\"table table-condensed table-striped\">\n");
  fprintf(H, "<b><tr><td>Rank</td><td>Old</td><td>Size</td><td>Unique</td><td>Quote</td></tr></b>\n");
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
      TStr URLLink = "<a href=\"cluster_" + CurDateString + "/" + TInt(Rank).GetStr() + ".html\">" + RepQuoteStr + "</a>";
      TStr OldRankStr;
      ComputeOldRankString(OldRankings, ClusterIds[i], Rank, OldRankStr);
      fprintf(H, "<tr><td>%d</td><td>%s</td><td>%d</td><td>%d</td><td>%s</td></tr>\n", Rank, OldRankStr.CStr(), Cluster.GetNumQuotes().Val, QuotesInCluster.Len(), URLLink.CStr());
      TStr ClusterFileName = WebDirectory + TimeStamp + "/web/cluster_" + CurDateString + "/" + TInt(Rank).GetStr() + ".html";
      TStr ImageFileName = WebDirectory + TimeStamp + "/web/cluster_" + CurDateString + "/" + TInt(Rank).GetStr();
      //fprintf(stderr, "Cluster: %s\n", RepQuoteStr.CStr());
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
          QB->GetRepresentativeUrl(DB, Quote.GetId(), QuoteRepUrl);
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

void LogOutput::ComputeOldRankString(THash<TInt, TInt>& OldRankings, TInt& ClusterId, TInt CurRank, TStr& OldRankStr) {
  TInt OldRanking;
  if (OldRankings.IsKeyGetDat(ClusterId, OldRanking)) {
    TInt Difference = OldRanking - CurRank;
    if (Difference < 0) {
      OldRankStr = "<b><center><font color=\"red\">" + Difference.GetStr() + "</font></center></b>";
    } else if (Difference > 0) {
      OldRankStr = "<b><center><font color=\"green\">+" + Difference.GetStr() + "</font></center></b>";
    } else {
      OldRankStr = "<b><center>0</center></b>";
    }
  } else {
    OldRankStr = "<center>new!</center>";
  }
}

void LogOutput::OutputDiscardedClusters(TQuoteBase *QB, TVec<TPair<TCluster, TInt> >& DiscardedClusters, TSecTm& Date) {
  TStr DiscardedFileName = WebDirectory + TimeStamp + "/output/discarded_clusters_" + Date.GetDtYmdStr() + ".txt";
  FILE *D = fopen(DiscardedFileName.CStr(), "w");

  for (int i = 0; i < DiscardedClusters.Len(); ++i) {
    TStr RepQuoteStr;
    DiscardedClusters[i].Val1.GetRepresentativeQuoteString(RepQuoteStr, QB);
    fprintf(D, "%d\t%s\n", DiscardedClusters[i].Val2.Val, RepQuoteStr.CStr()); // TODO: Add num clusters somehow
  }

  fclose(D);
}

void LogOutput::OutputDiscardedClustersBySize(TQuoteBase *QB, TVec<TCluster>& DiscardedClusters, TSecTm& Date) {
  TStr DiscardedFileName = WebDirectory + TimeStamp + "/output/discarded_clusters_by_size_" + Date.GetDtYmdStr() + ".txt";
  FILE *D = fopen(DiscardedFileName.CStr(), "w");

  for (int i = 0; i < DiscardedClusters.Len(); ++i) {
    TStr RepQuoteStr;
    DiscardedClusters[i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    fprintf(D, "%d\t%s\n", DiscardedClusters[i].GetNumQuotes().Val, RepQuoteStr.CStr()); // TODO: Add num clusters somehow
  }

  fclose(D);
}

void LogOutput::SetupQBDBCBSizeFile() {
  TStr QBDBCB = WebDirectory + TimeStamp + "/QBDBCB_info.txt";
  QBDBCBSizeFile = fopen(QBDBCB.CStr(), "w");
}

void LogOutput::LogQBDBCBSize(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB) {
  fprintf(QBDBCBSizeFile, "DB\t%d\tQB\t%d\tCB\t%d\n", DB->Len(), QB->Len(), CB->Len());
  fprintf(stderr, "DB\t%d\tQB\t%d\tCB\t%d\n", DB->Len(), QB->Len(), CB->Len());
}

void LogOutput::ShutDown() {
  fclose(QBDBCBSizeFile);
}
