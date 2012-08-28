#include "stdafx.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  // Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  TStr StartString, EndString;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2009-02-01";
  }
  if (!Arguments.IsKeyGetDat("end", EndString)) {
    EndString = "2009-02-06";
  }

  TStr QBDBCDirectory;
  if (!Arguments.IsKeyGetDat("qbdbc", QBDBCDirectory)) {
      QBDBCDirectory = "/lfs/1/tmp/curis/QBDBC/";
  }

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);
  TSecTm CurrentDate = StartDate;

  TQuoteBase QBCumulative;
  TDocBase DBCumulative;
  TClusterBase CBCumulative;
  for(TSecTm CurrentDate = StartDate; CurrentDate < EndDate; CurrentDate.AddDays(1)) {
    TQuoteBase QB;
    TDocBase DB;
    TClusterBase CB;
    PNGraph QGraph;
    fprintf(stderr, "Loading cumulative QBDBCB from %s from file...\n", CurrentDate.GetDtYmdStr().CStr());
    TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);
    fprintf(stderr, "Done loading cumulative QBDBCB!\n");

    fprintf(stderr, "Merging QBDBCB!\n");
    TDataLoader::FilterOldData(QB, DB, CB, StartDate);
    TDataLoader::MergeQBDBCB(QBCumulative, DBCumulative, CBCumulative, QB, DB, CB, CurrentDate);
  }

  Log.SetupNewOutputDirectory();
  TIntV TopFilteredClusters;
  CBCumulative.GetTopClusterIdsByFreq(TopFilteredClusters);
  PostCluster::FilterAndCacheClusterSize(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, EndDate);
  PostCluster::FilterAndCacheClusterPeaks(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, EndDate);

  Log.OutputClusterInformation(&DBCumulative, &QBCumulative, &CBCumulative, TopFilteredClusters, EndDate);
  Log.WriteClusteringOutputToFile(EndDate);

  // Construct new top QB, DB and CB
  TQuoteBase TopQB;
  TDocBase TopDB;
  TClusterBase TopCB;
  for (int i = 0; i < TopFilteredClusters.Len(); i++) {
    TCluster C;
    CBCumulative.GetCluster(TopFilteredClusters[i], C);
    TIntV QuoteIds, NewQuoteIds;
    C.GetQuoteIds(QuoteIds);
    for (int j = 0; j < QuoteIds.Len(); j++) {
      TQuote Q;
      QBCumulative.GetQuote(QuoteIds[j], Q);
      TStr QContentString;
      Q.GetContentString(QContentString);
      TIntV DocIds;
      Q.GetSources(DocIds);
      for (int k = 0; k < DocIds.Len(); k++) {
        TDoc D;
        DBCumulative.GetDoc(DocIds[k], D);
        TInt NewSourceId = TopDB.AddDoc(D);
        TopQB.AddQuote(QContentString, NewSourceId);
      }

      TStrV QContent;
      Q.GetContent(QContent);
      NewQuoteIds.Add(TopQB.GetQuoteId(QContent));
    }
    C.SetQuoteIds(&TopQB, NewQuoteIds);

    TIntV RepQuoteIds, NewRepQuoteIds;
    C.GetRepresentativeQuoteIds(RepQuoteIds);
    for (int j = 0; j < RepQuoteIds.Len(); j++) {
      TQuote Q;
      QBCumulative.GetQuote(RepQuoteIds[j], Q);

      TStrV QContent;
      Q.GetContent(QContent);
      NewRepQuoteIds.Add(TopQB.GetQuoteId(QContent));
    }
    C.SetRepresentativeQuoteIds(NewRepQuoteIds);

    TopCB.AddCluster(C);
  }

  TFOut FOut("TOPQBDBCB.bin");
  TopQB.Save(FOut);
  TopDB.Save(FOut);
  TopCB.Save(FOut);
  return 0;
}
