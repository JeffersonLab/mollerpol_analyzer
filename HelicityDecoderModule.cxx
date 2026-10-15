#include "HelicityDecoderModule.h"
#include "THaSlotData.h"
#include <iostream>

namespace Decoder {

   Module::TypeIter_t HelicityDecoderModule::fgThisType =
      DoRegister(ModuleType("Decoder::HelicityDecoderModule", 3564));

   HelicityDecoderModule::HelicityDecoderModule(UInt_t crate, UInt_t slot, UInt_t secondary_id)
      : VmeModule(crate, slot), 
	fBlockNum(0), fEventsInBlock(0), fWordsInBlock(0), fEventIndex(0),
	fSavedTcBits(0), fSavedTdBits(0), fSavedTeBits(0), fSavedTfBits(0) {}

   void HelicityDecoderModule::Clear(Option_t* opt) {
      VmeModule::Clear(opt);
   }

   UInt_t HelicityDecoderModule::LoadSlot(THaSlotData* sldat, const UInt_t* evbuffer, const UInt_t* pstop) {
      if (evbuffer == nullptr || pstop == nullptr || pstop < evbuffer) return 0;
      UInt_t nwords = static_cast<UInt_t>(pstop - evbuffer + 1);
      DecodeBlock(evbuffer, nwords);
      return nwords;
   }

   UInt_t HelicityDecoderModule::LoadSlot(THaSlotData* sldat, const UInt_t* evbuffer, UInt_t pos, UInt_t len) {
      if (evbuffer == nullptr || len == 0) return 0;
      DecodeBlock(evbuffer + pos, len);
      return len;
   }

   Int_t HelicityDecoderModule::Decode(const UInt_t* debuffer) {
      return 0;
   }

   Int_t HelicityDecoderModule::DecodeBlock(const UInt_t* rwhit, UInt_t nwords) {
      // Clear vectors that hold the n events in a block 
      fHelReportedVec.clear();
      fTstableActiveVec.clear();
      fHelTrueVec.clear();
      fPatternPolarityReportedVec.clear();
      fPatternPolarityTrueVec.clear();
      fPatternPhaseVec.clear();
      fTriggerNumVec.clear();
      fTriggerTimeVec.clear();
      fEventPolarityVec.clear();
      fHelicitySeedVec.clear();
      fFallingEdgeTStableVec.clear();
      fRisingEdgeTStableVec.clear();
      fPatternSyncCountVec.clear();
      fPairSyncCountVec.clear();
      fTimeTrigStartTStableVec.clear();
      fTimeTrigEndTStableVec.clear();
      fDurationLastTStableVec.clear();
      fDurationLastSettleVec.clear();
      fStatusAtTriggerVec.clear();
      fLast32PatternSyncVec.clear();
      fLast32PairSyncVec.clear();
      fLast32HelicityStateVec.clear();
      fLast32HelicityPatternStartVec.clear();

  
      fEventIndex = 0; 
      Double_t nspercnt = 8.0;
      Int_t lastDefinedType = -1;
      UInt_t dataWordsExpected = 0;
      UInt_t dataWordsCount = 0;
      Double_t currentTriggerTime = 0;
	 
      for (UInt_t i = 0; i < nwords; ++i) {
	 UInt_t word = rwhit[i];

	 if (lastDefinedType == 8 && dataWordsCount < dataWordsExpected) {
	    switch (dataWordsCount) {
	    case 0:  fHelicitySeedVec.push_back(word); break;
	    case 1:  fFallingEdgeTStableVec.push_back(double(word)*nspercnt); break;
	    case 2:  fRisingEdgeTStableVec.push_back(double(word)*nspercnt); break;
	    case 3:  fPatternSyncCountVec.push_back(word); break;
	    case 4:  fPairSyncCountVec.push_back(word); break;
	    case 5:  fTimeTrigStartTStableVec.push_back(double(word)*nspercnt); break;
	    case 6:  fTimeTrigEndTStableVec.push_back(double(word)*nspercnt); break;
	    case 7:  fDurationLastTStableVec.push_back(double(word)*nspercnt); break;
	    case 8:  fDurationLastSettleVec.push_back(double(word)*nspercnt); break;
	    case 9:
	       fTstableActiveVec.push_back((0x1 & word)==1);                  //0th bit Tstable On?
	       fStatusAtTriggerVec.push_back(word);
	       fHelReportedVec.push_back(((word >> 3) & 0x1)==1);             //4th bit helwin delayed
	       fPatternPolarityReportedVec.push_back(((word >> 4) & 0x1)==1); //5th bit helpat delayed
	       //event polarity = XOR of current helicity and helicity at pattern start
	       fEventPolarityVec.push_back((word >> 5) & 0x1);                //6th bit hel!=helpat
	       fPatternPhaseVec.push_back((word >> 8) & 0xFF);                //Bits 9-16 pat phase
	       break;
	    case 10: fLast32PatternSyncVec.push_back(word); break;
	    case 11: fLast32PairSyncVec.push_back(word); break;
	    case 12: fLast32HelicityStateVec.push_back(word); break;
	    case 13:
	       fLast32HelicityPatternStartVec.push_back(word);
	       if((fHelicitySeedVec.back()&0x3ffffff) != (fLast32HelicityPatternStartVec.back()&0x3ffffff))
		  std::cout<<"Disagreement between helicity seed and sequence."<<std::endl;
	       break;
    	    }
	    dataWordsCount++;
	    if (dataWordsCount == dataWordsExpected) {
	       lastDefinedType = -1;
	       // Every time an event's telemetry finishes parsing, pair it with the active timestamp
	       fTriggerTimeVec.push_back(currentTriggerTime*nspercnt);
	    }
	 } 
	 else {
	    Bool_t isTypeDefining = ((word >> 31) & 0x1) == 1;

	    if (isTypeDefining) {
	       UInt_t typeTag = (word >> 27) & 0xF;
	       lastDefinedType = typeTag;

	       switch (typeTag) {
	       case 0:
		  fBlockNum      = (word >> 8) & 0x3FF;
		  fEventsInBlock = word & 0xFF;
		  break;
	       case 1:
		  fWordsInBlock  = word & 0x3FFFFF;
		  break;
	       case 2: // Event Header
		  // FIX: Append the event-level sequence number directly into the tracking vector
		  fTriggerNumVec.push_back(word & 0xFFF); 
		  break;
	       case 3:
		  fSavedTcBits = (word >> 24) & 0x7;
		  fSavedTdBits = (word >> 16) & 0xFF;
		  fSavedTeBits = (word >> 8)  & 0xFF;
		  fSavedTfBits = word         & 0xFF;
		  break;
	       case 8:
		  dataWordsExpected = word & 0x3F;
		  dataWordsCount = 0;
		  break;
	       case 14:
		  return nwords;
	       }
	    } 
	    else if (lastDefinedType == 3) {
	       UInt_t taBits = (word >> 16) & 0xF;
	       UInt_t tbBits = (word >> 8)  & 0xFF;
	       UInt_t tcBits = word         & 0xFF;

	       ULong64_t fullTime = 0;
	       fullTime |= ((ULong64_t)taBits)       << 40;
	       fullTime |= ((ULong64_t)tbBits)       << 32;
	       fullTime |= ((ULong64_t)tcBits)       << 24;
	       fullTime |= ((ULong64_t)fSavedTdBits) << 16;
	       fullTime |= ((ULong64_t)fSavedTeBits) << 8;
	       fullTime |= ((ULong64_t)fSavedTfBits);
        
	       currentTriggerTime = fullTime; // Stash time until the data payload finishes
	       lastDefinedType = -1;
	    }
	 }
      }

      //std::cout << "[DEBUG PARSER] Loaded Multi-Block. Total sub-events cached: " << fHelicitySeedVec.size() << std::endl;
      return 0;
   }

} // namespace Decoder
