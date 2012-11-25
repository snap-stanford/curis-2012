#include "stdafx.h"

int main(int argc, char *argv[]) {
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log);

  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-01-14");
  TStr EndString = ArgumentParser::GetArgument(Arguments, "end", "2012-09-30");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", "/lfs/1/tmp/curis/QBDBC-final/");
  TStr QBDBDirectory = ArgumentParser::GetArgument(Arguments, "qbdb", "/lfs/1/tmp/curis/QBDB/");

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);

  TQuoteBase QB;
  TDocBase DB;
  TClusterBase CB;
  PNGraph QGraph;
  TQuoteBase NewQB;
  TDocBase NewDB;

  TSecTm CurrentDate = StartDate;
  TInt NumUnprocessedQuotes = 0;
  TInt NumUnprocessedDocs = 0;
  TInt NumQuotes = 0;
  TInt NumClusters = 0;
  TInt NumTopClusters = 0;
  TInt NumDiscardedClustersByPeak = 0;
  TInt NumDiscardedClustersByVariant = 0;
  TInt NumRemainingClusters = 0;
  TInt NumVariants = 0;
  TInt NumDocs = 0;
  TInt Count = 0;
  while(CurrentDate < EndDate) {
    if (Count % 100 == 0) Err("%d days evaluated!\n", Count.Val);
    TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);
    TDataLoader::LoadQBDB(QBDBDirectory, CurrentDate.GetDtYmdStr(), NewQB, NewDB);

    if (NewQB.Len() > 0) {
      NumUnprocessedQuotes += NewQB.Len();
      NumUnprocessedDocs += NewDB.Len();
    }

    if (QB.Len() > 0) {
      Count++;
      Err("Loaded base for %s! Calculating stats...\n", CurrentDate.GetDtYmdStr().CStr());
      NumQuotes += QB.Len();
      NumClusters += CB.Len();
      NumDocs += DB.Len();

      TIntV TopClusters;
      CB.GetTopClusterIdsByFreq(TopClusters);

      int NmTopClusters = TopClusters.Len();
      NumTopClusters += NmTopClusters;
      for (int i = 0; i < NmTopClusters; i++) {
        TCluster C;
        CB.GetCluster(TopClusters[i], C);
        if (C.GetDiscardState() == 1) {
          NumDiscardedClustersByPeak++;
        } else if (C.GetDiscardState() == 2) {
          NumDiscardedClustersByVariant++;
        } else {
          NumRemainingClusters++;
          NumVariants += C.GetNumUniqueQuotes();
        }
      }
    }
    CurrentDate.AddDays(1);
  }

  Err("Number of quotes processed through in total: %d\n", NumUnprocessedQuotes.Val);
  Err("Number of docs processed through in total: %d\n", NumUnprocessedDocs.Val);
  Err("Number of quotes in total: %d\n", NumQuotes.Val);
  Err("Number of clusters in total: %d\n", NumClusters.Val);
  Err("Number of docs in total: %d\n", NumDocs.Val);
  Err("Number of top clusters in total: %d\n", NumTopClusters.Val);
  Err("Number of discarded clusters by peak in total: %d\n", NumDiscardedClustersByPeak.Val);
  Err("Number of discarded clusters by variant total: %d\n", NumDiscardedClustersByVariant.Val);
  Err("Number of top clusters remaining in total: %d\n", NumRemainingClusters.Val);
  Err("Number of top variants found: %d\n", NumVariants.Val);
  Err("=============\n===========\n");
  Err("Number of days: %d\n", Count.Val);
  double AvgQuotes = (NumQuotes.Val * 1.0 / Count.Val);
  double AvgClusters = (NumClusters.Val * 1.0 / Count.Val);
  double AvgDocs = (NumDocs.Val * 1.0 / Count.Val);
  double AvgTopClusters = (NumTopClusters.Val * 1.0 / Count.Val);
  double AvgDiscardedPeaks = (NumDiscardedClustersByPeak.Val * 1.0 / Count.Val);
  double AvgDiscardedVariants = (NumDiscardedClustersByVariant.Val * 1.0 / Count.Val);
  double AvgRemaining = (NumRemainingClusters.Val * 1.0 / Count.Val);
  double AvgNumVariants = (NumVariants.Val * 1.0 / Count.Val);
  Err("Average number of quotes in total: %f\n", AvgQuotes);
  Err("Average number of clusters in total: %f\n", AvgClusters);
  Err("Average number of docs in total: %f\n", AvgDocs);
  Err("Average number of top clusters in total: %f\n", AvgTopClusters);
  Err("Average number of discarded clusters by peak in total: %f\n", AvgDiscardedPeaks);
  Err("Average number of discarded clusters by variant total: %f\n", AvgDiscardedVariants);
  Err("Average number of top clusters remaining in total: %f\n", AvgRemaining);
  Err("Average number of top variants found: %f\n", AvgNumVariants);
  return 0;
}
