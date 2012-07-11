#include "stdafx.h"
#include "dataloader.h"

TDataLoader::TDataLoader() {
  CurrentFileId = -1;
}


bool TDataLoader::LoadNextFile() {
	if (CurrentFileId == FileList.Len() || CurrentFileId == -1) {
		return false;
	} else {
	  SInPt = TZipIn::New(Prefix + FileList[CurrentFileId]);
	  //SInPt = TFIn::New(Prefix + FileList[CurrentFileId]);
	  CurrentFileId++;
		return true;
	}
}

void TDataLoader::LoadFileList(const TStr& InFileName, const TStr& Directory) {
	// Get file list from file
  FileList.Clr();

	Prefix = Directory;
	PSIn InFileNameF = TFIn::New(InFileName);
	TStr FileName;
	while (!InFileNameF->Eof() && InFileNameF->GetNextLn(FileName))
		FileList.Add(FileName);

	TChAV DeleteList;
	// Test whether each file is good
	for (int i = 0; i < FileList.Len(); i++) {
		FILE *fin = fopen((Prefix + FileList[i]).CStr(), "r");
		if (fin == NULL) {
			printf("Error reading file %s, ignore...\n", FileList[i].CStr());
			DeleteList.Add(FileList[i]);
		} else {
			fclose(fin);
		}
	}
	for (int i = 0; i < DeleteList.Len(); i++)
		FileList.DelIfIn(DeleteList[i]);
	CurrentFileId = 0;
}

TSecTm TDataLoader::GetCurrentFileTime() {
  int i;
  for (i = 0; i < FileList[CurrentFileId].Len(); i++) {
    if (FileList[CurrentFileId][i] == '-') { i++; break; }
  }

  TChA tmp_date;
  for (; i < FileList[CurrentFileId].Len(); i++) {
    if (FileList[CurrentFileId][i] == 'T') { i++; break; }
    tmp_date += FileList[CurrentFileId][i];
  }

  tmp_date += ' ';

  for (; i < FileList[CurrentFileId].Len(); i++) {
    if (FileList[CurrentFileId][i] == 'Z') { break; }
    tmp_date += FileList[CurrentFileId][i];
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
