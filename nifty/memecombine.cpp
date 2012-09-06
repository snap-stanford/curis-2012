#include "stdafx.h"
#include <stdio.h>

const TStr GraphDirectory = "../../../public_html/curis/output/clustering/visualization/graphdata/";
const TStr TableDirectory = "../../../public_html/curis/output/clustering/visualization/tabledata/";
const TStr ClusterDirectoryPrefix = "../../../public_html/curis/output/clustering/visualization/clusters/data";

int main(int argc, char *argv[]) {
  // Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  TStr StartString = ArgumentParser::GetArgument(Arguments, "start", "2009-02-01");
  TStr EndString = ArgumentParser::GetArgument(Arguments, "end", "2009-02-06");
  bool PrintTopClustersJson = Arguments.IsKey("printjson");
  TStr QBDBCDirectory = ArgumentParser::GetArgument(Arguments, "qbdbc", "/lfs/1/tmp/curis/QBDBC/");

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);
  TSecTm CurrentDate = StartDate;

  // XXCumulative contains info starting from StartDate; XXAll contains all info
  TQuoteBase QBCumulative, QBAll;
  TDocBase DBCumulative, DBAll;
  TClusterBase CBCumulative, CBAll;

  for(TSecTm CurrentDate = StartDate; CurrentDate < EndDate; CurrentDate.AddDays(1)) {
    TQuoteBase QB;
    TDocBase DB;
    TClusterBase CB;
    PNGraph QGraph;
    fprintf(stderr, "Loading cumulative QBDBCB from %s from file...\n", CurrentDate.GetDtYmdStr().CStr());
    TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);
    fprintf(stderr, "Done loading cumulative QBDBCB!\n");

    fprintf(stderr, "Merging QBDBCB!\n");

    // TODO: REMOVE THIS - just for testing
    TCluster C1;
    TStr RepQuoteStr;
    TIntV CQuoteIds;
    bool ClusterGotten = CBAll.GetCluster(5323, C1);
    if (ClusterGotten) {
      C1.GetRepresentativeQuoteString(RepQuoteStr, &QBAll);
      fprintf(stderr, "\t5323 Rep Quote: %s\n", RepQuoteStr.CStr());
      C1.GetQuoteIds(CQuoteIds);
      for (int j = 0; j < CQuoteIds.Len(); ++j) {
        TQuote Q1;
        QBAll.GetQuote(CQuoteIds[j], Q1);
        TStr QContent1;
        Q1.GetContentString(QContent1);
        fprintf(stderr, "\t\t%s\n", QContent1.CStr());
      }
    }

    TStr RepQuoteStr2;
    TDataLoader::MergeQBDBCB(QBAll, DBAll, CBAll, QB, DB, CB, CurrentDate);
    ClusterGotten = CB.GetCluster(577800, C1);
    if (ClusterGotten) {
      C1.GetRepresentativeQuoteString(RepQuoteStr2, &QB);
      fprintf(stderr, "\t577800 Rep Quote: %s\n", RepQuoteStr.CStr());
      C1.GetQuoteIds(CQuoteIds);
      for (int j = 0; j < CQuoteIds.Len(); ++j) {
        TQuote Q1;
        QB.GetQuote(CQuoteIds[j], Q1);
        TStr QContent1;
        Q1.GetContentString(QContent1);
        fprintf(stderr, "\t\t%s\n", QContent1.CStr());
      }
    }

    //TDataLoader::FilterOldData(QB, DB, CB, StartDate);
    //TDataLoader::MergeQBDBCB(QBCumulative, DBCumulative, CBCumulative, QB, DB, CB, CurrentDate);
  }

  Log.SetupNewOutputDirectory();
  TIntV TopFilteredClusters;
  CBCumulative.GetTopClusterIdsByFreq(TopFilteredClusters);
  PostCluster::FilterAndCacheClusterSize(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, EndDate);
  PostCluster::FilterAndCacheClusterPeaks(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, EndDate);

  if (PrintTopClustersJson) {
    TIntV ClustersToGraph;
    for (int i = 0; i < 50 && i < TopFilteredClusters.Len(); i++) {
      ClustersToGraph.Add(TopFilteredClusters[i]);
    }
    TIntV ClustersToTable = TopFilteredClusters;

    TPrintJson::PrintClustersJson(&QBCumulative, &DBCumulative, &CBCumulative, ClustersToGraph, ClustersToTable, GraphDirectory, TableDirectory, StartDate, EndDate);
    TPrintJson::PrintClustersDataJson(&QBAll, &DBAll, &CBAll, ClustersToTable, ClusterDirectoryPrefix, EndDate);
  }

  // Commented out the next two lines because they were causing warnings to pop up :(
  //Log.OutputClusterInformation(&DBCumulative, &QBCumulative, &CBCumulative, TopFilteredClusters, EndDate);
  //Log.WriteClusteringOutputToFile(EndDate);

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
