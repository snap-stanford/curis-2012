#ifndef data_loader_h
#define data_loader_h

#include "../../snap/snap-core/Snap.h"

class TDataLoader {
private:
	TStr Prefix;
	TChAV FileList;
	TInt CurrentFileId;
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
  void LoadFileList(const TStr&, const TStr&);
  bool LoadNextFile();
  bool LoadNextEntry();
  int GetNumFiles() { return FileList.Len(); }
};

#endif
