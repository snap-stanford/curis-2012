#include "logoutput.h"
#include "quote.h"
#include "doc.h"

const TStr LogOutput::WebDirectory = WEB_DIR_DEFAULT;
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
  ShouldLog.Save(SOut);
}

void LogOutput::Load(TSIn& SIn) {
  OutputValues.Load(SIn);
  ShouldLog.Load(SIn);
}

void LogOutput::DisableLogging() {
  ShouldLog = false;
}

void LogOutput::EnableLogging() {
  ShouldLog = true;
}

// Directory is timestamped in the web directory by default.
void LogOutput::SetupNewOutputDirectory(TStr Directory) {
  if (!ShouldLog) return;
  // CASE 1: We need to set up a new directory
  if (Directory == TStr("")) {
    TSecTm Tm = TSecTm::GetCurTm();
    TStr TimeStamp = Tm.GetDtYmdStr() + "_" + Tm.GetTmStr();
    this->Directory = WebDirectory + TimeStamp;
    Err("New directory set up: %s\n", this->Directory.CStr());
  } else {
    // CASE 2: The user has specified a directory - we just have to store it
    this->Directory = Directory;
    Err("Using existing directory: %s\n", this->Directory.CStr());
  }

  // Copy directories over.
  TStrV Commands;
  Commands.Add("mkdir -p " + this->Directory);
  Commands.Add("cp -r resources/output/text " + this->Directory);
  Commands.Add("cp -r resources/output/web " + this->Directory);

  for (int i = 0; i < Commands.Len(); i++) {
    system(Commands[i].CStr());
  }
  Err("Necessary files copied over to %s\n", this->Directory.CStr());
}

void LogOutput::GetDirectory(TStr& Directory) {
  Directory = this->Directory;
}

void LogOutput::SetDirectory(TStr& Directory) {
  this->Directory = Directory;
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

void LogOutput::LogAllInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime, TIntV& OldTopClusters, TStr& QBDBCDirectory) {
  if (!ShouldLog) return;
  WriteClusteringStatisticsToFile(PresentTime);
  PrintClusterInformationToText(DB, QB, CB, ClusterIds, PresentTime);
  PrintClusterInformation(DB, QB, CB, ClusterIds, PresentTime, OldTopClusters);
  LogQBDBCBSize(DB, QB, CB, PresentTime);
  TStr JSONDirectory = Directory + "/web/json/";
  TStr CurDateString = PresentTime.GetDtYmdStr();
  TSecTm NextDay = PresentTime;
  NextDay.AddDays(1);
  TStr NextDayString = NextDay.GetDtYmdStr();
  TPrintClusterJson JSONJob(JSONDirectory);
  LogOutput TmpLog;
  TmpLog.DisableLogging();
  //JSONJob.PrintClusterJSONForPeriod(CurDateString, "week", QBDBCDirectory);
  //JSONJob.PrintClusterJSONForPeriod(CurDateString, "month", QBDBCDirectory);
  //JSONJob.PrintClusterJSONForPeriod(CurDateString, "3month", QBDBCDirectory);

  // Update current date file
  TStr CurrentDateFileName = Directory + "/web/currentdate.txt";
  FILE *C = fopen(CurrentDateFileName.CStr(), "w");
  fprintf(C, "%s", CurDateString.CStr());
  fclose(C);
  Err("Done logging everything! Current date file updated to %s at %s.\n", CurDateString.CStr(), CurrentDateFileName.CStr());
}

void LogOutput::WriteClusteringStatisticsToFile(TSecTm& Date) {
  if (!ShouldLog) return;
  TStr FileName = Directory + "/text/statistics/statistics_" + Date.GetDtYmdStr() + ".txt";
  FILE *F = fopen(FileName.CStr(), "w");

  TStrV Keys;
  OutputValues.GetKeyV(Keys);

  for (int i = 0; i < Keys.Len(); ++i) {
    fprintf(F, "%s\t%s\n", Keys[i].CStr(), OutputValues.GetDat(Keys[i]).CStr());
  }

  fclose(F);
}

void LogOutput::PrintClusterInformationToText(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime) {
  if (!ShouldLog) return;

  TStr CurDateString = PresentTime.GetDtYmdStr();
  TStr TopFileName = Directory + "/text/top/topclusters_" + CurDateString + ".txt";
  FILE *T = fopen(TopFileName.CStr(), "w");

  for (int i = 0; i < ClusterIds.Len(); i++) {
    TCluster C;
    CB->GetCluster(ClusterIds[i], C);
    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);

    TIntV CQuoteIds, CUniqueSources;
    C.GetQuoteIds(CQuoteIds);
    TCluster::GetUniqueSources(CUniqueSources, CQuoteIds, QB);

    fprintf(T, "%d\t%d\t%s\n", CUniqueSources.Len(), CQuoteIds.Len(), CRepQuote.CStr());

    for (int j = 0; j < CQuoteIds.Len(); j++) {
      TQuote Q;
      if (QB->GetQuote(CQuoteIds[j], Q)) {
        TStr QuoteStr;
        Q.GetContentString(QuoteStr);
        fprintf(T, "\t%d\t%s\n", Q.GetNumSources().Val, QuoteStr.CStr());
      }
    }
  }
  fclose(T);
}

void LogOutput::PrintClusterInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime, TIntV &OldTopClusters) {
  if (!ShouldLog) return;
  TStr CurDateString = PresentTime.GetDtYmdStr();
  Err("Writing cluster information...\n");

  // PREVIOUS RANKING SETUP
  THash<TInt, TInt> OldRankings;
  if (OldTopClusters.Len() > 0) {
    for (int i = 0; i < OldTopClusters.Len(); i++) {
      OldRankings.AddDat(OldTopClusters[i], i + 1);
    }
  }

  TStrV RankStr;
  TStr ClusterJSONDirectory = Directory + "/web/json/clusters/";
  for (int i = 0; i < ClusterIds.Len(); i++) {
    TStr OldRankStr;
    ComputeOldRankString(OldRankings, ClusterIds[i], i+1, OldRankStr);
    RankStr.Add(OldRankStr);

    // JSON file for each cluster!
    TPrintJson::PrintClusterJSON(QB, DB, CB, ClusterJSONDirectory, ClusterIds[i], PresentTime);
  }

  Err("JSON Files for individual written!\n");
  TStr JSONTableFileName = Directory + "/web/json/daily/" + CurDateString + ".json";
  TPrintJson::PrintClusterTableJSON(QB, DB, CB, JSONTableFileName, ClusterIds, RankStr);
  Err("JSON Files for the cluster table written!\n");
}

void LogOutput::ComputeOldRankString(THash<TInt, TInt>& OldRankings, TInt& ClusterId, TInt CurRank, TStr& OldRankStr) {
  TInt OldRanking;
  if (OldRankings.IsKeyGetDat(ClusterId, OldRanking)) {
    TInt Difference = OldRanking - CurRank;
    if (Difference < 0) {
      OldRankStr = "<b><center><font color=\\\"red\\\">" + Difference.GetStr() + "</font></center></b>";
    } else if (Difference > 0) {
      OldRankStr = "<b><center><font color=\\\"green\\\">+" + Difference.GetStr() + "</font></center></b>";
    } else {
      OldRankStr = "<b><center>0</center></b>";
    }
  } else {
    OldRankStr = "<center>new!</center>";
  }
}

void LogOutput::OutputDiscardedClusters(TQuoteBase *QB, TVec<TPair<TCluster, TInt> >& DiscardedClusters, TSecTm& Date) {
  if (!ShouldLog) return;
  TStr DiscardedFileName = Directory + "/text/discarded/peaks/discarded_by_peaks_" + Date.GetDtYmdStr() + ".txt";
  FILE *D = fopen(DiscardedFileName.CStr(), "w");

  for (int i = 0; i < DiscardedClusters.Len(); ++i) {
    TStr RepQuoteStr;
    DiscardedClusters[i].Val1.GetRepresentativeQuoteString(RepQuoteStr, QB);
    fprintf(D, "%d\t%s\n", DiscardedClusters[i].Val2.Val, RepQuoteStr.CStr()); // TODO: Add num clusters somehow
  }

  fclose(D);
}

void LogOutput::OutputDiscardedClustersBySize(TQuoteBase *QB, TVec<TCluster>& DiscardedClusters, TSecTm& Date) {
  if (!ShouldLog) return;
  TStr DiscardedFileName = Directory + "/text/discarded/variants/discarded_by_size_" + Date.GetDtYmdStr() + ".txt";
  FILE *D = fopen(DiscardedFileName.CStr(), "w");

  for (int i = 0; i < DiscardedClusters.Len(); ++i) {
    TStr RepQuoteStr;
    DiscardedClusters[i].GetRepresentativeQuoteString(RepQuoteStr, QB);
    fprintf(D, "%d\t%s\n", DiscardedClusters[i].GetNumQuotes().Val, RepQuoteStr.CStr()); // TODO: Add num clusters somehow
  }

  fclose(D);
}

void LogOutput::LogQBDBCBSize(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TSecTm PresentTime) {
  if (!ShouldLog) return;
  TStr CurDateString = PresentTime.GetDtYmdStr();
  TStr QBDBCB = Directory + "/text/QBDBCB_info.txt";
  QBDBCBSizeFile = fopen(QBDBCB.CStr(), "a");
  fprintf(QBDBCBSizeFile, "%s\tDB\t%d\tQB\t%d\tCB\t%d\n", CurDateString.CStr(), DB->Len(), QB->Len(), CB->Len());
  fprintf(stderr, "DB\t%d\tQB\t%d\tCB\t%d\n", DB->Len(), QB->Len(), CB->Len());
  fclose(QBDBCBSizeFile);
}
