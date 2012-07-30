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

/// Merge QBDB2 into QBDB1; returns the indices (in the new QB1) of the quotes in QB2
//  that are not in QB1
TIntV TDataLoader::MergeQBDB(TQuoteBase &QB1, TDocBase &DB1, const TQuoteBase &QB2, const TDocBase &DB2) {
  THashSet<TInt> SeenDocSet;

  TIntV DocIds2;
  DB2.GetAllDocIds(DocIds2);
  for (int i = 0; i < DocIds2.Len(); i++) {
    TDoc D;
    DB2.GetDoc(DocIds2[i], D);
    TStr DUrl;
    D.GetUrl(DUrl);
    if (DB1.GetDocId(DUrl) != -1) {
      SeenDocSet.AddKey(DocIds2[i]);
    }
  }

  TIntV NewQuoteIds;
  TIntV QuoteIds2;
  QB2.GetAllQuoteIds(QuoteIds2);
  for (int i = 0; i < QuoteIds2.Len(); i++) {
    TQuote Q;
    QB2.GetQuote(QuoteIds2[i], Q);
    TStr QContentString;
    Q.GetContentString(QContentString);
    TIntV Sources;
    Q.GetSources(Sources);

    TStrV QContentVectorString;
    TQuote::ParseContentString(QContentString, QContentVectorString);
    // Make note if the quote does not exist in QB1
    if (QB1.GetQuoteId(QContentVectorString) < 0) {
      TInt QId = QB1.AddQuote(QContentString);
      NewQuoteIds.Add(QId);
    }
    for (int j = 0; j < Sources.Len(); j++) {
      // If doc is in original docbase, ignore it because that means
      // the document is a duplicate
      if (!SeenDocSet.IsKey(Sources[j])) {
        TDoc D;
        DB2.GetDoc(Sources[j], D);
        TInt NewSourceId = DB1.AddDoc(D);
        D.SetId(NewSourceId);
        QB1.AddQuote(QContentString, D);
      }
    }
  }

  return NewQuoteIds;
}

void TDataLoader::LoadCumulative(const TStr &Prefix, const TStr &Date, TQuoteBase &QB, TDocBase &DB, TClusterBase &CB, PNGraph& P) {
  TStr CurFileName = "QBDBC" + Date + ".bin";
  TFIn CurFile(Prefix + CurFileName);
  QB.Load(CurFile);
  DB.Load(CurFile);
  CB.Load(CurFile);
  P = TNGraph::Load(CurFile);
}

void TDataLoader::LoadQBDB(const TStr &Prefix, const TStr &Date, TQuoteBase &QB, TDocBase &DB) {
  TStr CurFileName = "QBDB" + Date + ".bin";
  TFIn CurFile(Prefix + CurFileName);
  QB.Load(CurFile);
  DB.Load(CurFile);
}

TSecTm TDataLoader::LoadBulkQBDB(const TStr &Prefix, const TStr &InFileName, TQuoteBase &QB, TDocBase &DB) {
  PSIn InFileNameF = TFIn::New(InFileName);
  TStr Date;
  TStr CurrentDate;
  while (!InFileNameF->Eof() && InFileNameF->GetNextLn(Date)) {
    TQuoteBase TmpQB;
    TDocBase TmpDB;
    LoadQBDB(Prefix, Date, TmpQB, TmpDB);
    MergeQBDB(QB, DB, TmpQB, TmpDB);
    CurrentDate = Date;
  }
  return TSecTm::GetDtTmFromYmdHmsStr(CurrentDate + " 23:00:00");
}
