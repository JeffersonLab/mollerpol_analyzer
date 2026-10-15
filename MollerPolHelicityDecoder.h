#ifndef MollerPolHelicityDecoder_h_
#define MollerPolHelicityDecoder_h_

#include "THaHelicityDet.h"
#include "THaSlotData.h"
#include "HelicityDecoderModule.h"
#include "Module.h"

class MollerPolHelicityDecoder : public THaHelicityDet {
public:
   MollerPolHelicityDecoder(const char* name, const char* description, THaApparatus* a = nullptr);
   virtual ~MollerPolHelicityDecoder() = default;
   virtual THaAnalysisObject::EStatus Init( const TDatime& date );
   virtual Int_t Decode(const THaEvData& evdata);
   virtual void Clear(Option_t* opt = "") override;
   virtual const char* GetDBFileName() const {
      static TString db_name;
      db_name = fPrefix;
      db_name.Chop(); // Removes the trailing '.' 
      return db_name.Data();
   }
protected:
   virtual Int_t ReadDatabase(const TDatime& date);
   virtual Int_t DefineVariables(EMode mode);
   UInt_t GetRandbit30(UInt_t& ranseed);
   Bool_t CheckPredictor(UInt_t &testval);
   
   Int_t     fBank_ID;
   Int_t     fHelicityDelay;
   UInt_t    fBlockNum;
   UInt_t    fEventsInBlock;
   UInt_t    fWordsInBlock;
   UInt_t    fTriggerNum;
   Double_t  fTriggerTime;
   Bool_t    fTstableActive;
   Bool_t    fHelReported;
   Bool_t    fHelTrue;
   Bool_t    fEventPolarity;
   Bool_t    fPatternPolarityReported;
   Bool_t    fPatternPolarityTrue;
   Int_t     fPatternPhase;
   UInt_t    fHelicitySeed;
   UInt_t    fFallingEdgeTStable;
   UInt_t    fRisingEdgeTStable;
   UInt_t    fPatternSyncCount;
   UInt_t    fPairSyncCount;
   Double_t  fTimeTrigStartTStable;
   Double_t  fTimeTrigEndTStable;
   Double_t  fDurationLastTStable;
   Double_t  fDurationLastSettle;
   UInt_t    fStatusAtTrigger;
   UInt_t    fLast32PatternSync;
   UInt_t    fLast32PairSync;
   UInt_t    fLast32HelicityState;
   UInt_t    fLast32HelicityPatternStart;
   
   ClassDef(MollerPolHelicityDecoder, 1)
};
#endif
