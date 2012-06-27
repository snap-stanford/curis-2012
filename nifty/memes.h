#ifndef memes_h
#define memes_h

#include "../../snap/snap-core/Snap.h"


class TQuote {
private:
	TInt QuoteId;
	TStrV QuoteContent;
	TIntV SourceDocs;
public:

};

class TQuoteBase {
private:
	THash<TInt, TQuote> IdToQuote;
	THash<TQuote, TInt> QuoteToId;
public:

//	TStr GetQuoteString();
};
