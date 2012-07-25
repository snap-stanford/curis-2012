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
  static TIntV MergeQBDB(TQuoteBase&, TDocBase&, const TQuoteBase&, const TDocBase&);
  static void LoadCumulative(const TStr&, const TStr&, TQuoteBase&, TDocBase&, TVec<TCluster>&);
  static void LoadQBDB(const TStr&, const TStr&, TQuoteBase&, TDocBase&);
  static TSecTm LoadBulkQBDB(const TStr&, const TStr&, TQuoteBase&, TDocBase&);
};

#endif
