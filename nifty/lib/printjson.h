#ifndef printjson_h
#define printjson_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TPrintJson {
  private:
    static void PrintJSON(TStr& FileName, THash<TStr, TStrV> JSON, TStr& Extra);
    static void PrintClustersGraphJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                       TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate, bool IncludeDate);
    static void PrintClustersTableJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                       TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate, bool IncludeDate);
  public:
    static void PrintClusterTableJSON(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                               TStr& FileName, TIntV& Clusters, TStrV& RankStr);
    static void PrintClusterJSON(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, PNGraph& QGraph, 
                                      TStr& FolderName, TInt& ClusterId, TSecTm PresentTime);
    static void PrintClustersJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TIntV& ClustersToGraph,
                                  TIntV& ClustersToTable,
                                  const TStr& GraphDir, const TStr& TableDir, TSecTm& StartDate, TSecTm& EndDate, bool IncludeDate = false);
    static void PrintClustersDataJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TIntV& ClustersToPrint, const TStr& ClusterDataDir, TSecTm& EndDate);
    static void GetTopPeakClustersPerDay(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                         TIntV& ClustersToPrint, TInt NumPerDay, TSecTm& StartDate, TSecTm& EndDate);
    static TStr JSONEscape(TStr& String);
};

#endif
