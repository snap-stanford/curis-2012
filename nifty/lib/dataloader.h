#ifndef dataloader_h
#define dataloader_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TDataLoader {
private:
  TStr CurrentFileName;
  PSIn SInPt;
  TChA CurLn;
public:
  uint64 LineCnt;
  TChA PostUrlStr;
  TChA ContentStr;
  TSecTm PubTm;
  TVec<TChA> MemeV;       // quote
  TVec<TIntPr> MemePosV;  // (index, length), quote begins at ContentStr[MemePos[i]]
  TVec<TChA> LinkV;       // link url
  TVec<TInt> LinkPosV;    // url begins at ContentStr[LinkPosV[i]]
public:
  void Clr();
  bool LoadFile(const TStr&, const TStr&);
  bool LoadNextEntry();

  static TSecTm GetFileTime(const TStr&);
  static TIntV MergeQBDB(TQuoteBase&, TDocBase&, const TQuoteBase&, const TDocBase&, bool MaintainDuplicateQuotes = false);
  static void MergeQBDBCB(TQuoteBase &QB1, TDocBase &DB1, TClusterBase &CB1,
                              const TQuoteBase &QB2, const TDocBase &DB2, const TClusterBase &CB2, TSecTm& PresentTime,
                          bool KeepQuotesWithNoSources = false);
  static void MergeTopQBDBCB(TQuoteBase &QB1, TDocBase &DB1, TClusterBase &CB1, PNGraph &Q1,
                              const TQuoteBase &QB2, const TDocBase &DB2, const TClusterBase &CB2, const PNGraph &Q2, TSecTm& PresentTime,
                          bool KeepQuotesWithNoSources = false);
  static void LoadCumulative(const TStr&, const TStr&, TQuoteBase&, TDocBase&, TClusterBase&, PNGraph&, bool LoadTop = false);
  static void FilterOldData(TQuoteBase &QB, TDocBase &DB, TClusterBase &CB, TSecTm CutOffDate);
  static void LoadQBDB(const TStr&, const TStr&, TQuoteBase&, TDocBase&);
  static TSecTm LoadBulkQBDB(const TStr&, const TStr&, TQuoteBase&, TDocBase&);
  static TSecTm LoadQBDBByWindow(const TStr& Prefix, const TStr& StartDate, const TInt WindowSize, TQuoteBase& QB, TDocBase& DB);

  static void SaveQBDBC(TStr FileName, TQuoteBase *QB, TDocBase *DB, TClusterBase *CB);
  static void SaveQBDBCQ(TStr FileName, TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, PNGraph QGraph);
};

#endif
