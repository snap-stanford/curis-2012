#include "stdafx.h"
#include "dataloader.h"

bool TDataLoader::LoadFile(const TStr &Prefix, const TStr &FileName) {
  FILE *fin = fopen((Prefix + FileName).CStr(), "r");
  Err("FileName: %s\n", (Prefix + FileName).CStr());
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
TIntV TDataLoader::MergeQBDB(TQuoteBase &QB1, TDocBase &DB1, const TQuoteBase &QB2, const TDocBase &DB2, bool MaintainDuplicateQuotes) {
  THashSet<TUInt> SeenDocSet;

  TVec<TUInt> DocIds2;
  DB2.GetAllDocIds(DocIds2);
  for (int i = 0; i < DocIds2.Len(); i++) {
    TDoc D;
    DB2.GetDoc(DocIds2[i], D);
    TStr DUrl;
    D.GetUrl(DUrl);
    if (DB1.GetDocId(DUrl) != 0) {
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
    TVec<TUInt> Sources;
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
    if (!MaintainDuplicateQuotes && ContainDoc && QB1.GetQuoteId(QContentVectorString) < 0) {
      TInt QId = QB1.AddQuote(QContentString);
      NewQuoteIds.Add(QId);
    }

    for (int j = 0; j < Sources.Len(); j++) {
      // If doc is in original docbase, ignore it because that means
      // the document is a duplicate
      if (!SeenDocSet.IsKey(Sources[j])) {
        TDoc D;
        DB2.GetDoc(Sources[j], D);
        TUInt NewSourceId = DB1.AddDoc(D);
        if (!MaintainDuplicateQuotes) {
          QB1.AddQuote(QContentString, NewSourceId);
        } else {
          QB1.AddQuoteMerging(Q.GetId(), QContentString, NewSourceId);
        }
      }
    }
  }
  IAssert(!QB1.IsContainNullQuote());
  return NewQuoteIds;
}


void TDataLoader::MergeTopQBDBCB(TQuoteBase &QB1, TDocBase &DB1, TClusterBase &CB1, PNGraph &Q1,
                              const TQuoteBase &QB2, const TDocBase &DB2, const TClusterBase &CB2, const PNGraph &Q2, TSecTm& PresentTime,
                              bool KeepQuotesWithNoSources) {
  if (QB1.Len() == 0) {
    QB1 = QB2;
    DB1 = DB2;
    CB1 = CB2;
    Q1 = Q2;
  }

  TVec<TUInt> DocIds;
  DB2.GetAllDocIds(DocIds);
  for (int i = 0; i < DocIds.Len(); i++) {
    TDoc D;
    DB2.GetDoc(DocIds[i], D);
    DB1.AddStaticDoc(DocIds[i], D);
  }

  TIntV QuoteIds;
  QB2.GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote D;
    QB2.GetQuote(QuoteIds[i], D);
    QB1.AddStaticQuote(QuoteIds[i], D);
  }

  TIntV ClusterIds;
  CB2.GetAllClusterIds(ClusterIds);
  for (int i = 0; i < ClusterIds.Len(); i++) {
    TCluster D;
    CB2.GetCluster(ClusterIds[i], D);
    CB1.AddStaticCluster(ClusterIds[i], D);
  }

  TNGraph::TNodeI EndNode = Q2->EndNI();
  for (TNGraph::TNodeI Node = Q2->BegNI(); Node < EndNode; Node++) {
    Q1->AddNode(Node.GetId());
  }
  TNGraph::TEdgeI EndEdge = Q2->EndEI();
  for (TNGraph::TEdgeI Edge = Q2->BegEI(); Edge < EndEdge; Edge++) {
    Q1->AddEdge(Edge.GetSrcNId(), Edge.GetDstNId());
  }
}

/// Merge QBDBCB2 into QBDBCB1
void TDataLoader::MergeQBDBCB(TQuoteBase &QB1, TDocBase &DB1, TClusterBase &CB1,
                              const TQuoteBase &QB2, const TDocBase &DB2, const TClusterBase &CB2, TSecTm& PresentTime,
                              bool KeepQuotesWithNoSources) {
  // First step: Merge QB2 and DB2 into QB1 and DB1, respectively
  TIntV NewQuoteIds;
  NewQuoteIds = MergeQBDB(QB1, DB1, QB2, DB2, true); // set MaintainDuplicateQuotes flag!

  IAssert(!QB1.IsContainNullQuote());

  TIntV ClusterIds2;
  CB2.GetAllClusterIds(ClusterIds2);
  for (int i = 0; i < ClusterIds2.Len(); i++) {
    TCluster C;
    CB2.GetCluster(ClusterIds2[i], C);
    TIntV CQuoteIds, NewCQuoteIds;
    C.GetQuoteIds(CQuoteIds);

    TIntV CRepQuoteIds, NewCRepQuoteIds;
    C.GetRepresentativeQuoteIds(CRepQuoteIds);

    IAssert(CQuoteIds.Len() > 0 && CRepQuoteIds.Len() > 0);

    C.DeathDate = PresentTime;
    CB1.AddCluster(C);
    //}
  }
  
  TIntV ClusterIds;
  CB1.GetAllClusterIds(ClusterIds);
  for (int i = 0; i < ClusterIds.Len(); i++) {
    TCluster C;
    CB1.GetCluster(ClusterIds[i], C);
    C.DeathDate = PresentTime;
  }
}

void TDataLoader::FilterOldData(TQuoteBase &QB, TDocBase &DB, TClusterBase &CB, TSecTm CutOffDate) {
  TQuoteBase NewQB;
  TDocBase NewDB;

  THashSet<TUInt> OldDocSet;
  TVec<TUInt> DocIds;
  DB.GetAllDocIds(DocIds);
  for (int i = 0; i < DocIds.Len(); i++) {
    TDoc D;
    DB.GetDoc(DocIds[i], D);
    if (D.GetDate() < CutOffDate) {
      OldDocSet.AddKey(DocIds[i]);
    }
  }

  TIntV QuoteIds;
  QB.GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    QB.GetQuote(QuoteIds[i], Q);
    TVec<TUInt> Sources;
    Q.GetSources(Sources);
    TStr QContentString;
    Q.GetContentString(QContentString);

    for (int j = 0; j < Sources.Len(); j++) {
      if (!OldDocSet.IsKey(Sources[j])) {
        TDoc D;
        DB.GetDoc(Sources[j], D);
        TUInt NewSourceId = NewDB.AddDoc(D);
        NewQB.AddQuote(QContentString, NewSourceId);
      }
    }
  }

  TIntIntH OldToNewQuoteId;
  QuoteIds.Clr();
  NewQB.GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    NewQB.GetQuote(QuoteIds[i], Q);

    TStrV QContent;
    Q.GetContent(QContent);
    OldToNewQuoteId.AddDat(QB.GetQuoteId(QContent), QuoteIds[i]);
  }
  QB = NewQB;
  DB = NewDB;

  TIntV ClusterIds;
  CB.GetAllClusterIds(ClusterIds);
  for (int i = 0; i < ClusterIds.Len(); i++) {
    TCluster C;
    CB.GetCluster(ClusterIds[i], C);
    TIntV CQuoteIds, NewCQuoteIds;
    C.GetQuoteIds(CQuoteIds);
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      TInt NewId = CQuoteIds[j];
      if (OldToNewQuoteId.IsKeyGetDat(CQuoteIds[j], NewId)) {
        NewCQuoteIds.Add(NewId);
      }
    }

    TIntV CRepQuoteIds, NewCRepQuoteIds;
    C.GetRepresentativeQuoteIds(CRepQuoteIds);
    for (int j = 0; j < CRepQuoteIds.Len(); j++) {
      TInt NewId = CRepQuoteIds[j];
      if (OldToNewQuoteId.IsKeyGetDat(CRepQuoteIds[j], NewId)) {
        NewCRepQuoteIds.Add(NewId);
      }
    }

    if (NewCQuoteIds.Len() != 0 && NewCRepQuoteIds.Len() != 0) {
      C.SetQuoteIds(&QB, &DB, NewCQuoteIds);
      C.SetRepresentativeQuoteIds(NewCRepQuoteIds);
	  //C.DeathDate = PresentTime;
      CB.AddCluster(C);
    } else {
      CB.RemoveCluster(ClusterIds[i]);
    }
  }
}

void TDataLoader::LoadCumulative(const TStr &Prefix, const TStr &Date, TQuoteBase &QB, TDocBase &DB, TClusterBase &CB, PNGraph& P, bool LoadTop) {
  TStr CurFileName = Prefix + "QBDBC" + Date + ".bin";
  if (LoadTop) {
    CurFileName = Prefix + "topQBDBC" + Date + ".bin";
  }
  if (TFile::Exists(CurFileName)) {
    TFIn CurFile(CurFileName);
    QB.Load(CurFile);
    DB.Load(CurFile);
    CB.Load(CurFile);
    P = TNGraph::Load(CurFile);
    IAssert(!QB.IsContainNullQuote());
  } else {
    P = TNGraph::New();
    Err("File not found: %s", CurFileName.CStr());
  }
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

void TDataLoader::SaveQBDBC(TStr FileName, TQuoteBase *QB, TDocBase *DB, TClusterBase *CB) {
  TFOut FOut(FileName);
  QB->Save(FOut);
  DB->Save(FOut);
  CB->Save(FOut);
}


void TDataLoader::SaveQBDBCQ(TStr FileName, TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, PNGraph QGraph) {
  Err("Saving Cluster information to file: %s\n", FileName.CStr());
  {
    TFOut FOut(FileName);
    Err("Opened file");
    QB->Save(FOut);
    Err("Saved QB");
    DB->Save(FOut);
    Err("Saved DB");
    CB->Save(FOut);
    Err("Saved CB");
    QGraph->Save(FOut);
    Err("Saved QGraph");
  }
  Err("Done!\n");
}
