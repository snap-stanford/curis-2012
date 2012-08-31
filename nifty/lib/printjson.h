#ifndef printjson_h
#define printjson_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TPrintJson {
  private:
    static void PrintClustersGraphJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                       TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate);
    static void PrintClustersTableJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                       TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate);
  public:
    static void PrintClustersJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TIntV& ClustersToGraph,
                                  TIntV& ClustersToTable,
                                  const TStr& GraphDir, const TStr& TableDir, TSecTm& StartDate, TSecTm& EndDate);
    static void PrintClustersDataJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TIntV& ClustersToPrint, const TStr& ClusterDataDir, TSecTm& EndDate);
    static void GetTopPeakClustersPerDay(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                         TIntV& ClustersToPrint, TInt NumPerDay, TSecTm& StartDate, TSecTm& EndDate);
};

#endif
