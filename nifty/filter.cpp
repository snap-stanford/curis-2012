#include "stdafx.h"
#include "data_loader.h"
#include "quote.h"
#include "doc.h"

const int MinMemeFreq = 5;
const int MinQtWrdLen = 3;
const int MaxQtWrdLen = 30;

THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);

bool IsEnglish(TChA &quote) {
	return quote.CountCh('?') <= quote.Len()/2;
}

bool IsUrlInBlackList(TChA &Url) {
	if (strstr(Url.CStr(), "facebook.com") != NULL) { return true; }
	if (strstr(Url.CStr(), "twitter.com") != NULL) { return true; }
	return false;
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
	Memes.LoadFileList(InFileName, "/lfs/hulk/0/datasets/spinn3r/spinn3r-full5/");
	while (Memes.LoadNextFile()) {
		while (Memes.LoadNextEntry()) {
			if (IsUrlInBlackList(Memes.PostUrlStr)) continue;
			if (IsDuplicateUrl(Memes.PostUrlStr)) { NSkip++;continue; }
			for (int m = 0; m < Memes.MemeV.Len(); m++) {
				if (IsEnglish(Memes.MemeV[m])) {
					TStr QtStr = Memes.MemeV[m];
					TQuoteBase::QuoteFilter(QtStr);
					MemeCntH.AddDat(TMd5Sig(QtStr)) += 1;
				}
			}
		}
		printf("1: Complete %d file out of %d files HashSize = %d UrlSize = %d\n",
				++fileCnt, Memes.GetNumFiles(), MemeCntH.Len(), SeenUrlSet.Len());

		printf("Done \nAll quotes: %d\n", MemeCntH.Len());
		printf("  skip %d urls, keep %d\n", NSkip, SeenUrlSet.Len());

		printf("Meme2QtBs DONE!\n");
	}

	// Find frequent quotes
	THashSet<TMd5Sig> FreqMemeSet;
	for (int i = 0; i < MemeCntH.Len(); i++) {
		if (MemeCntH[i] >= MinMemeFq) {
			FreqMemeSet.AddKey(MemeCntH.GetKey(i)); }
	}
	SeenUrlSet.Clr(true);
	MemeCntH.Clr(true);
	printf("Number of frequent quotes: %d\n", FqMemeSet.Len());

	NSkip = 0; 	fileCnt = 0;
	TQuoteBase *QB = new TQuoteBase;
	TDocBase *DB = new TDocBase;
	// Add frequent quote containing memes into quote base
	Memes.LoadFileList(InFileName, "/lfs/hulk/0/datasets/spinn3r/spinn3r-full5/");
	while (Memes.LoadNextFile()) {
		while (Memes.LoadNextEntry()) {
			if (IsUrlInBlackList(Memes.PostUrlStr)) continue;
			if (IsDuplicateUrl(Memes.PostUrlStr)) { NSkip++;continue; }
			for (int m = 0; m < Memes.MemeV.Len(); m++) { // delete non-frequent memes
				TStr qtStr = Memes.MemeV[m];
				TQuoteBase::QuoteFilter(qtStr);
				Memes.MemeV[m] = qtStr;
				if (! FqMemeSet.IsKey(TMd5Sig(Memes.MemeV[m])) ||
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
	printf("\n2: Complete %d out of %d files\n", ++fileCnt, filelist.Len());
	printf("SAVE: %d quotes\n", Len());
	printf("LOADING DATA TO QUOTE BASE DONE!\n");
	return 0;
}
