#include "stdafx.h"
#include "dataloader.h"

bool TDataLoader::LoadFile(const TStr &Prefix, const TStr &FileName) {
  FILE *fin = fopen((Prefix + FileName).CStr(), "r");
  if (fin == NULL) {
    printf("Error reading file %s, ignore...\n", FileName.CStr());
    return false;
  }
  fclose(fin);
  SInPt = TZipIn::New(Prefix + FileName);
  CurrentFileName = FileName;
  return true;
}

TSecTm TDataLoader::GetFileTime(const TStr &FileName) {
  int i;
  for (i = 0; i < FileName.Len(); i++) {
    if (FileName[i] == '-') { i++; break; }
  }
  TChA tmp_date;
  for (; i < FileName.Len(); i++) {
    if (FileName[i] == 'T') { i++; break; }
    tmp_date += FileName[i];
  }
  tmp_date += ' ';
  for (; i < FileName.Len(); i++) {
    if (FileName[i] == 'Z') { break; }
    tmp_date += FileName[i];
  }
  return TSecTm::GetDtTmFromYmdHmsStr(TStr(tmp_date), '-', '-');
}


void TDataLoader::Clr() {
	PostUrlStr.Clr();
	ContentStr.Clr();
	PubTm = TSecTm();
	MemeV.Clr(false);
	MemePosV.Clr(false);
	LinkV.Clr(false);
	LinkPosV.Clr(false);
}

// FORMAT:
//U \t Post URL
//D \t Post time
//T \t Post title (optional!)
//C \t Post content
//L \t Index \t URL      (URL starts at Content[Index])
//Q \t Index \t Length \t Quote (Quote starts at Content[Index])
bool TDataLoader::LoadNextEntry() {
	Clr();
	if (SInPt.Empty() || SInPt->Eof()) {
		return false;
	}
	TSIn& SIn = *SInPt;
	CurLn.Clr();

	// Keep reading until line starts with P\t
	while (SIn.GetNextLn(CurLn) && (CurLn.Empty() || (CurLn[0]!='U' || CurLn[1]!='\t'))) {
		printf("SKIP: L: %s\n", CurLn.CStr()); LineCnt++; }
	LineCnt++;
	if (CurLn.Empty()) { return LoadNextEntry(); }
	if (!((! CurLn.Empty()) && CurLn[0]=='U' && CurLn[1]=='\t'))
		{printf("Error reading this file, return\n"); return false;}
	IAssertR((! CurLn.Empty()) && CurLn[0]=='U' && CurLn[1]=='\t',
			TStr::Fmt("ERROR1: %s [line %llu]: '%s'\n", SIn.GetSNm().CStr(), LineCnt, CurLn.CStr()).CStr());
	PostUrlStr = CurLn.CStr()+2;
	while (SIn.GetNextLn(CurLn) && (CurLn.Empty() || (CurLn[0]!='D' || CurLn[1]!='\t'))) { LineCnt++; }

	if (!((! CurLn.Empty()) && CurLn[0]=='D' && CurLn[1]=='\t' && CurLn[2] <'A'))
	  	  {printf("Error reading this file, return\n"); return false;}
	IAssertR((! CurLn.Empty()) && CurLn[0]=='D',
			TStr::Fmt("ERROR2: %s [line %llu]: '%s'\n", SIn.GetSNm().CStr(), LineCnt, CurLn.CStr()).CStr());  LineCnt++;
	try {
		PubTm = TSecTm::GetDtTmFromStr(CurLn);
	} catch (PExcept Except){ PubTm = 1; ErrNotify(Except->GetStr());
    	printf("ERROR3: %s [line %llu]: '%s'\n", SIn.GetSNm().CStr(), LineCnt, CurLn.CStr());
	}

	IAssertR(SIn.GetNextLn(CurLn) && (! CurLn.Empty()) && (CurLn[0]=='C' || CurLn[0]=='T'),
			TStr::Fmt("ERROR4: %s [line %llu]: '%s'\n", SIn.GetSNm().CStr(), LineCnt, CurLn.CStr()).CStr());  LineCnt++;
	if (!((! CurLn.Empty()) && (CurLn[0]=='C' || CurLn[0]=='T')))
	  	  {printf("Error reading this file, return\n"); return false;}
	if (CurLn[0] == 'T') { // skip title
		IAssertR(SIn.GetNextLn(CurLn) && (! CurLn.Empty()) && CurLn[0]=='C',
				TStr::Fmt("ERROR5: %s [line %llu]: '%s'\n", SIn.GetSNm().CStr(), LineCnt, CurLn.CStr()).CStr());  LineCnt++; }
	ContentStr = CurLn.CStr()+2;
	// Links
	while (SIn.GetNextLn(CurLn)) {  LineCnt++;
    	if (CurLn.Empty() || CurLn[0]!='L') { break; }
    	int linkb=2;
    	while (CurLn[linkb]!='\t') { linkb++; }
    	CurLn[linkb]=0;
    	LinkV.Add(CurLn.CStr()+linkb+1);
    	LinkPosV.Add(atoi(CurLn.CStr()+2));
	}
	// Quotes
	do {
		if (CurLn.Empty() || CurLn[0]!='Q') { break; }
		int qb1=2;      while (CurLn[qb1]!='\t') { qb1++; }
		int qb2=qb1+1;  while (CurLn[qb2]!='\t') { qb2++; }
		CurLn[qb1]=0;  CurLn[qb2]=0;
		MemeV.Add(CurLn.CStr()+qb2+1);
		MemePosV.Add(TIntPr(atoi(CurLn.CStr()+2), atoi(CurLn.CStr()+qb1+1)));
		LineCnt++;
	} while (SIn.GetNextLn(CurLn));
	return true;
}

void TDataLoader::LoadQBDB(const TStr &Prefix, const TStr &InFileName, TQuoteBase &QB, TDocBase &DB) {
  THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);
  PSIn InFileNameF = TFIn::New(InFileName);
  TStr Date;
  while (!InFileNameF->Eof() && InFileNameF->GetNextLn(Date)) {
    TStr CurFileName = "QBDB" + Date + ".bin";
    TFIn CurFile(Prefix + CurFileName);
    THashSet<TInt> SeenDocSet;

    TQuoteBase TmpQB;
    TDocBase TmpDB;
    TmpQB.Load(CurFile);
    TmpDB.Load(CurFile);

    TIntV DocIds;
    TmpDB.GetAllDocIds(DocIds);
    for (int i = 0; i < DocIds.Len(); i++) {
      TDoc D;
      TmpDB.GetDoc(DocIds[i], D);
      TStr DUrl;
      D.GetUrl(DUrl);
      TMd5Sig UrlSig = TMd5Sig(DUrl);
      if (SeenUrlSet.IsKey(UrlSig)) {
        SeenDocSet.AddKey(DocIds[i]);
        continue;
      }
      SeenUrlSet.AddKey(UrlSig);
    }

    TIntV QuoteIds;
    TmpQB.GetAllQuoteIds(QuoteIds);
    for (int i = 0; i < QuoteIds.Len(); i++) {
      TQuote Q;
      TmpQB.GetQuote(QuoteIds[i], Q);
      TStr QContentString;
      Q.GetContentString(QContentString);
      TIntV Sources;
      Q.GetSources(Sources);

      for (int j = 0; j < Sources.Len(); j++) {
        if (!SeenDocSet.IsKey(Sources[j])) {
          TDoc D;
          TmpDB.GetDoc(Sources[j], D);
          TInt NewSourceId = DB.AddDoc(D);
          QB.AddQuote(QContentString, NewSourceId);
        }
      }
    }
  }
}
