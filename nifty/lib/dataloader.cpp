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
	TStringUtil::RemoveNonEnglish(PostUrlStr);
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
	if (!CurLn.Empty() && CurLn[0] == 'T') { SIn.GetNextLn(CurLn);  LineCnt++; }
	if (!CurLn.Empty() && CurLn[0] == 'C') {
	  ContentStr = CurLn.CStr()+2;
	  TStringUtil::RemoveNonEnglish(ContentStr);
	  SIn.GetNextLn(CurLn);
	  LineCnt++;
	}

	// Links
	do {
	  if (CurLn.Empty() || CurLn[0]!='L') { break; }
	  int linkb=2;
	  while (CurLn[linkb]!='\t') { linkb++; }
	  CurLn[linkb]=0;
	  LinkV.Add(CurLn.CStr()+linkb+1);
	  LinkPosV.Add(atoi(CurLn.CStr()+2));
	  LineCnt++;
	} while (SIn.GetNextLn(CurLn));
	// Quotes
	do {
		if (CurLn.Empty() || CurLn[0]!='Q') { break; }
		int qb1=2;      while (CurLn[qb1]!='\t') { qb1++; }
		int qb2=qb1+1;  while (CurLn[qb2]!='\t') { qb2++; }
		CurLn[qb1]=0;  CurLn[qb2]=0;
		TChA Quote = CurLn.CStr()+qb2+1;
		TStringUtil::RemoveNonEnglish(Quote);
		MemeV.Add(Quote);
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
    TStringUtil::ParseStringIntoWords(QContentString, QContentVectorString);

    bool ContainDoc = false;
    for (int j = 0; j < Sources.Len(); j++) {
      if (!SeenDocSet.IsKey(Sources[j])) {
        ContainDoc = true;
      }
    }
    // Make note if the quote does not exist in QB1
    if (ContainDoc && QB1.GetQuoteId(QContentVectorString) < 0) {
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
        QB1.AddQuote(QContentString, NewSourceId);
      }
    }
  }
  IAssert(!QB1.IsContainNullQuote());
  return NewQuoteIds;
}

/// Merge QBDBCB2 into QBDBCB1
void TDataLoader::MergeQBDBCB(TQuoteBase &QB1, TDocBase &DB1, TClusterBase &CB1,
                              const TQuoteBase &QB2, const TDocBase &DB2, const TClusterBase &CB2, TSecTm& PresentTime) {
  // First step: Merge QB2 and DB2 into QB1 and DB1, respectively
  // TODO: decompose this code - right now it is basically just a copy of the method above
  THashSet<TInt> SeenDocSet;  // Set of doc ids in DB2 that are also in DB1

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

  TIntIntH OldToNewQuoteId;  // Only contains quotes with new source document
  TIntSet ToRemoveQuotes;
  TIntV QuoteIds2;
  QB2.GetAllQuoteIds(QuoteIds2);
  for (int i = 0; i < QuoteIds2.Len(); i++) {
    TQuote Q;
    QB2.GetQuote(QuoteIds2[i], Q);
    TIntV QSources;
    Q.GetSources(QSources);
    TStr QContentString;
    Q.GetContentString(QContentString);
    TStrV QContentVectorString;
    TStringUtil::ParseStringIntoWords(QContentString, QContentVectorString);

    bool ContainDoc = false;
    for (int j = 0; j < QSources.Len(); j++) {
      if (!SeenDocSet.IsKey(QSources[j])) {
        ContainDoc = true;
        break;
      }
    }

    if (ContainDoc) {
      if (QB1.GetQuoteId(QContentVectorString) < 0) {
        TInt QId = QB1.AddQuote(QContentString);
        OldToNewQuoteId.AddDat(QuoteIds2[i], QId);
      } else {
        OldToNewQuoteId.AddDat(QuoteIds2[i], QB1.GetQuoteId(QContentVectorString));
      }
    } else {
      if (QB1.GetQuoteId(QContentVectorString) >= 0) {
        OldToNewQuoteId.AddDat(QuoteIds2[i], QB1.GetQuoteId(QContentVectorString));
      } else {
        ToRemoveQuotes.AddKey(QuoteIds2[i]);
      }
    }

    for (int j = 0; j < QSources.Len(); j++) {
      if (!SeenDocSet.IsKey(QSources[j])) {
        TDoc D;
        DB2.GetDoc(QSources[j], D);
        TInt NewSourceId = DB1.AddDoc(D);
        QB1.AddQuote(QContentString, NewSourceId);
      }
    }
  }

  /*for (TIntIntH::TIter I = OldToNewQuoteId.BegI(); I < OldToNewQuoteId.EndI(); I++) {
    TQuote Q1, Q2;
    QB1.GetQuote(I.GetDat(), Q1);
    QB2.GetQuote(I.GetKey(), Q2);
    TStr Q1Str, Q2Str;
    Q1.GetContentString(Q1Str);
    Q2.GetContentString(Q2Str);
    if (Q1Str != Q2Str) {
      fprintf(stderr, "ERROR: Q1 not equal to Q2: %d, %s vs. %d, %s\n", I.GetDat(), Q1Str.CStr(), I.GetKey(), Q2Str.CStr());
    }
  }*/

  IAssert(!QB1.IsContainNullQuote());

  TIntV ClusterIds2;
  CB2.GetAllClusterIds(ClusterIds2);
  for (int i = 0; i < ClusterIds2.Len(); i++) {
    TCluster C;
    CB2.GetCluster(ClusterIds2[i], C);
    TIntV CQuoteIds, NewCQuoteIds;
    C.GetQuoteIds(CQuoteIds);
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      if (ToRemoveQuotes.IsKey(CQuoteIds[j])) { continue; }
      TInt NewId = CQuoteIds[j];
      OldToNewQuoteId.IsKeyGetDat(CQuoteIds[j], NewId);
      NewCQuoteIds.Add(NewId);

      TQuote Q1, Q2;
      QB1.GetQuote(NewId, Q1);
      QB2.GetQuote(CQuoteIds[j], Q2);
      TStr Q1Str, Q2Str;
      Q1.GetContentString(Q1Str);
      Q2.GetContentString(Q2Str);
      if (Q1Str != Q2Str) {
        fprintf(stderr, "ERROR: Q1 not equal to Q2: %d, %s vs. %d, %s\n", NewId.Val, Q1Str.CStr(), CQuoteIds[j].Val, Q2Str.CStr());
      }
      
    }

    TIntV CRepQuoteIds, NewCRepQuoteIds;
    C.GetRepresentativeQuoteIds(CRepQuoteIds);
    for (int j = 0; j < CRepQuoteIds.Len(); j++) {
      if (ToRemoveQuotes.IsKey(CRepQuoteIds[j])) { continue; }
      TInt NewId = CRepQuoteIds[j];
      OldToNewQuoteId.IsKeyGetDat(CRepQuoteIds[j], NewId);
      NewCRepQuoteIds.Add(NewId);
    }

    C.SetQuoteIds(&QB1, NewCQuoteIds);
    C.SetRepresentativeQuoteIds(NewCRepQuoteIds);
    CB1.AddCluster(C);

  }


}

void TDataLoader::LoadCumulative(const TStr &Prefix, const TStr &Date, TQuoteBase &QB, TDocBase &DB, TClusterBase &CB, PNGraph& P) {
  TStr CurFileName = "QBDBC" + Date + ".bin";
  TFIn CurFile(Prefix + CurFileName);
  QB.Load(CurFile);
  DB.Load(CurFile);
  CB.Load(CurFile);
  P = TNGraph::Load(CurFile);
  IAssert(!QB.IsContainNullQuote());
}

void TDataLoader::LoadQBDB(const TStr &Prefix, const TStr &Date, TQuoteBase &QB, TDocBase &DB) {
  TSecTm CurrentDate = TSecTm::GetDtTmFromYmdHmsStr(Date);
  TStr SubDir = TStr::Fmt("%d/", CurrentDate.GetYearN());
  TStr CurFileName = "QBDB" + Date + ".bin";
  TFIn CurFile(Prefix + SubDir + CurFileName);
  QB.Load(CurFile);
  DB.Load(CurFile);
  IAssert(!QB.IsContainNullQuote());
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
  return TSecTm::GetDtTmFromYmdHmsStr(CurrentDate);
}

TSecTm TDataLoader::LoadQBDBByWindow(const TStr& Prefix, const TStr& StartDate, const TInt WindowSize, TQuoteBase& QB, TDocBase& DB) {
  TSecTm CurrentDate = TSecTm::GetDtTmFromYmdHmsStr(StartDate);
  for (int i = 0; i < WindowSize; i++) {
    TQuoteBase TmpQB;
    TDocBase TmpDB;
    fprintf(stderr, "Seeking file %s.bin\n", CurrentDate.GetDtYmdStr().CStr());
    LoadQBDB(Prefix, CurrentDate.GetDtYmdStr(), TmpQB, TmpDB);
    MergeQBDB(QB, DB, TmpQB, TmpDB);
    if (i + 1 < WindowSize) {
      CurrentDate.AddDays(1);
    }
  }
  return CurrentDate;
}
