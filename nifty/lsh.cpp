#include "stdafx.h"
#include "lsh.h"

const int LSH::BandSize = 1;
const int LSH::NumBands = 20;
const int LSH::ShingleLen = 4;

// For every quote, add it to corresponding bucket for each hashed x-character shingle of the quote
void LSH::HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig, TIntSet>& ShingleToQuoteIds) {
  printf("Hashing shingles...\n");
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);
  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    if (qt % 1000 == 0) {
      printf("%d out of %d completed\n", qt, QuoteIds.Len());
    }
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);

    // Put x-character shingles into hash table; x is specified by ShingleLen parameter
    TStr QContentStr;
    Q.GetParsedContentString(QContentStr);
    TChA QContentChA = TChA(QContentStr);
    
    for (int i = 0; i < QContentChA.Len()-ShingleLen+1; i++) {
      TChA ShingleChA = TChA();
      for (int j = 0; j < ShingleLen; j++) {
        ShingleChA.AddCh(QContentChA.GetCh(i + j));
      }
      TStr Shingle = TStr(ShingleChA);
      const TMd5Sig ShingleMd5(Shingle);
      TIntSet ShingleQuoteIds;
      if (ShingleToQuoteIds.IsKey(ShingleMd5)) {
        ShingleQuoteIds = ShingleToQuoteIds.GetDat(ShingleMd5);
      }
      ShingleQuoteIds.AddKey(QuoteIds[qt]);
      ShingleToQuoteIds.AddDat(ShingleMd5, ShingleQuoteIds);
    }
  }
}

void  LSH::MinHash(THash<TMd5Sig, TIntSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets) {
  TRnd RandomGenerator; // TODO: make this "more random" by incorporating time
  for(int i = 0; i < NumBands; ++i) {
    THash<TInt, TIntV> Inverted; // (QuoteID, QuoteSignatureForBand)
    THash<TIntV, TIntSet> BandBuckets; // (BandSignature, QuoteIDs)
    for (int j = 0; j < BandSize; ++j) {
      // Create new signature
      TVec<TMd5Sig> Signature;
      ShingleToQuoteIds.GetKeyV(Signature);
      Signature.Shuffle(RandomGenerator);

      // Place in bucket - not very efficient
      int SigLen = Signature.Len();
      for (int k = 0; k < SigLen; ++k) {
        TIntSet CurSet = ShingleToQuoteIds.GetDat(Signature[k]);
        for (TIntSet::TIter l = CurSet.BegI(); l < CurSet.EndI(); l++) {
          TInt Key = l.GetKey();
          if (Inverted.IsKey(Key)) {
            TIntV CurSignature = Inverted.GetDat(Key);
            if (CurSignature.Len() <= j) {
              CurSignature.Add(k);
              Inverted.AddDat(Key, CurSignature);
            }
          } else {
            TIntV NewSignature;
            NewSignature.Add(k);
            Inverted.AddDat(Key, NewSignature);
          }
        }
      }
    }

    TIntV InvertedKeys;
    Inverted.GetKeyV(InvertedKeys);
    TInt InvertedLen = InvertedKeys.Len();
    for (int k = 0; k < InvertedLen; ++k) {
      TIntSet Bucket;
      TIntV Signature = Inverted.GetDat(InvertedKeys[k]);
      if (BandBuckets.IsKey(Signature)) {
        Bucket = BandBuckets.GetDat(Signature);
      }
      Bucket.AddKey(InvertedKeys[k]);
      BandBuckets.AddDat(Signature, Bucket);
    }

    SignatureBandBuckets.Add(BandBuckets);
    printf("%d out of %d band signatures computed\n", i+1, NumBands);
  }
  printf("Minhash step complete!\n");
}
