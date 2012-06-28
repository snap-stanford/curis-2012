#include "stdafx.h"
#include "data_loader.h"
#include "quote.h"
#include "doc.h"

const int MinMemeFreq = 5;
const int MinQtWrdLen = 3;
const int MaxQtWrdLen = 30;

THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);
THashSet<TStr> URLBlackList;

bool IsEnglish(TChA &quote) {
	return quote.CountCh('?') <= quote.Len()/2;
}

void LoadURLBlackList() {
  PSIn BlackListFile = TFIn::New("URLBlacklist");
  TStr BadURL;
  while (!BlackListFile->Eof() && BlackListFile->GetNextLn(BadURL)) {
    URLBlackList.AddKey(BadURL);
  }
}

bool IsUrlInBlackList(TChA &Url) {
  TStr UrlStr(Url);
  TStrV PeriodVector;
  UrlStr.SplitOnAllAnyCh(".", PeriodVector);

  if (PeriodVector.Len() >= 2) {
    TStrV SlashVector;
    PeriodVector[PeriodVector.Len() - 1].SplitOnAllAnyCh("/", SlashVector);
    if (SlashVector.Len() >= 1) {
      TStr DomainName = PeriodVector[PeriodVector.Len() - 2] + "." + SlashVector[0];
      if (URLBlackList.IsKey(DomainName)) {
        return true;
      }
    }
  }

	return false;
}


// Removes all punctuation in the quotes and replace with spaces.
// Also converts upper case to lower case.
// Adapted (but modified) from memes.h because I want a white list, not a blacklist.
// TODO: move to filter.cpp
void FilterSpacesAndSetLowercase(TStr &QtStr) {
  // Three passes...hopefully this isn't too slow.
  TChA QtChA(QtStr);
  for (int i = 0; i < QtChA.Len(); ++i) {
    if (isalpha(QtChA[i]) || QtChA[i] == '\'') {
      QtChA[i] = ' ';
    }
    QtChA[i] = tolower(QtStr[i]);
  }
  QtStr = TStr(QtChA);
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}

bool IsDuplicateUrl(TChA &Url) {
	TMd5Sig UrlSig = TMd5Sig(Url);
	if (SeenUrlSet.IsKey(UrlSig)) { return true; }
		SeenUrlSet.AddKey(UrlSig);
	return false;
}

// usage filelist directory
int main(int argc, char *argv[]) {
	TStr InFileName = "filename list";

	printf("Loading data from Spinn3r dataset to QuoteBase...\n");
	int NSkip = 0, fileCnt = 0;
	THash<TMd5Sig, TInt> MemeCntH(Mega(100), true);

	// Read files and count the quotes
	TDataLoader Memes;
	Memes.LoadFileList(InFileName, "/lfs/1/tmp/spinn3r-full3/");
	while (Memes.LoadNextFile()) {
		while (Memes.LoadNextEntry()) {
			if (IsUrlInBlackList(Memes.PostUrlStr)) continue;
      if (IsDuplicateUrl(Memes.PostUrlStr)) { NSkip++;continue; }
			for (int m = 0; m < Memes.MemeV.Len(); m++) {
				if (IsEnglish(Memes.MemeV[m])) {
					TStr QtStr = Memes.MemeV[m];
					FilterSpacesAndSetLowercase(QtStr);
					MemeCntH.AddDat(TMd5Sig(QtStr)) += 1;
				}
			}
		}
		printf("1: Complete %d file out of %d files HashSize = %d UrlSize = %d\n",
				++fileCnt, Memes.GetNumFiles(), MemeCntH.Len(), SeenUrlSet.Len());

		printf("Done \nAll quotes: %d\n", MemeCntH.Len());
		printf("  skip %d urls, keep %d\n", NSkip, SeenUrlSet.Len());
	}

	// Find frequent quotes
	THashSet<TMd5Sig> FreqMemeSet;
	for (int i = 0; i < MemeCntH.Len(); i++) {
		if (MemeCntH[i] >= MinMemeFreq) {
			FreqMemeSet.AddKey(MemeCntH.GetKey(i)); }
	}
	SeenUrlSet.Clr(true);
	MemeCntH.Clr(true);
	printf("Number of frequent quotes: %d\n", FreqMemeSet.Len());

	NSkip = 0; 	fileCnt = 0;
	TQuoteBase *QB = new TQuoteBase;
	TDocBase *DB = new TDocBase;
	// Add frequent quote containing memes into quote base
	Memes.LoadFileList(InFileName, "/lfs/1/tmp/spinn3r-full3/");
	while (Memes.LoadNextFile()) {
		while (Memes.LoadNextEntry()) {
			if (IsUrlInBlackList(Memes.PostUrlStr)) continue;
			if (IsDuplicateUrl(Memes.PostUrlStr)) { NSkip++;continue; }
			for (int m = 0; m < Memes.MemeV.Len(); m++) { // delete non-frequent memes
				TStr qtStr = Memes.MemeV[m];
				FilterSpacesAndSetLowercase(qtStr);
				Memes.MemeV[m] = qtStr;
				if (! FreqMemeSet.IsKey(TMd5Sig(Memes.MemeV[m])) ||
						TStrUtil::CountWords(Memes.MemeV[m].CStr()) < MinQtWrdLen ||
						TStrUtil::CountWords(Memes.MemeV[m].CStr()) > MaxQtWrdLen)
					Memes.MemeV[m].Clr();
			}
			if (Memes.MemeV.Len() >= 1) {
				TInt CurrentDocId = DB->AddDoc(Memes.PostUrlStr, Memes.PubTm, Memes.ContentStr, Memes.LinkV);
				for (int m = 0; m < Memes.MemeV.Len(); m++) {
					QB->AddQuote(Memes.MemeV[m], CurrentDocId);
				}
			}
		}
	}
	printf("\n2: Complete %d out of %d files\n", ++fileCnt, Memes.GetNumFiles());
	printf("SAVE: %d quotes\n", QB->Len());
	printf("LOADING DATA TO QUOTE BASE DONE!\n");
	return 0;
}
