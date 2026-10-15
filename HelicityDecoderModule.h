#ifndef HelicityDecoderModule_h_
#define HelicityDecoderModule_h_

#include "VmeModule.h"
#include <vector>

namespace Decoder {

   class HelicityDecoderModule : public VmeModule {
   public:
      // Default constructor required by ROOT's dynamic TClass reflection engine
      HelicityDecoderModule() : HelicityDecoderModule(0, 0) {}

      // Standard hardware constructor
      HelicityDecoderModule(UInt_t crate, UInt_t slot, UInt_t secondary_id = 0);
      virtual ~HelicityDecoderModule() = default;

      // Un-hide the overloaded LoadSlot methods from base classes
      using VmeModule::LoadSlot;

      // Concrete interface overrides for VmeModule
      virtual UInt_t LoadSlot(THaSlotData* sldat, const UInt_t* evbuffer, const UInt_t* pstop) override;
      virtual UInt_t LoadSlot(THaSlotData* sldat, const UInt_t* evbuffer, UInt_t pos, UInt_t len) override;
      virtual void Clear(Option_t* opt = "") override;
      virtual Int_t Decode(const UInt_t* debuffer) override;

      // Advance our event retrieval cursor forward for the next physics tree row
      void AdvanceSubEvent() const { fEventIndex++; }

      // Getters that safely look up block-level macro scalars
      UInt_t    GetBlockNum()                   const { return fBlockNum; }
      UInt_t    GetEventsInBlock()              const { return fEventsInBlock; }
      UInt_t    GetWordsInBlock()               const { return fWordsInBlock; }
  
      // Getters that dynamically read out event-level variables based on the current index row
      UInt_t GetHelReported() const {
	 return fEventIndex < fHelReportedVec.size() ? fHelReportedVec[fEventIndex] : 0;
      }
      UInt_t GetHelTrue() const {
	 return fEventIndex < fHelTrueVec.size() ? fHelTrueVec[fEventIndex] : 0;
      }
      UInt_t GetPatternPolarityReported() const {
	 return fEventIndex < fPatternPolarityReportedVec.size() ? fPatternPolarityReportedVec[fEventIndex] : 0;
      }
      UInt_t GetPatternPolarityTrue() const {
	 return fEventIndex < fPatternPolarityTrueVec.size() ? fPatternPolarityTrueVec[fEventIndex] : 0;
      }
      UInt_t GetPatternPhase() const {
	 return fEventIndex < fPatternPhaseVec.size() ? fPatternPhaseVec[fEventIndex] : 0;
      }
      UInt_t GetTriggerNum() const { 
	 return (fEventIndex < fTriggerNumVec.size()) ? fTriggerNumVec[fEventIndex] : 0; 
      }
      ULong64_t GetTriggerTime() const { 
	 return (fEventIndex < fTriggerTimeVec.size()) ? fTriggerTimeVec[fEventIndex] : 0; 
      }
      UInt_t GetHelicitySeed() const { 
	 return (fEventIndex < fHelicitySeedVec.size()) ? fHelicitySeedVec[fEventIndex] : 0; 
      }
      Bool_t GetEventPolarity() const { 
	 return (fEventIndex < fEventPolarityVec.size()) ? fEventPolarityVec[fEventIndex] : 0; 
      }
      Bool_t GetTstableActive() const { 
	 return (fEventIndex < fTstableActiveVec.size()) ? fTstableActiveVec[fEventIndex] : 0; 
      }
      UInt_t GetFallingEdgeTStable() const { 
	 return (fEventIndex < fFallingEdgeTStableVec.size()) ? fFallingEdgeTStableVec[fEventIndex] : 0; 
      }
      UInt_t GetRisingEdgeTStable() const { 
	 return (fEventIndex < fRisingEdgeTStableVec.size()) ? fRisingEdgeTStableVec[fEventIndex] : 0; 
      }
      UInt_t GetPatternSyncCount() const { 
	 return (fEventIndex < fPatternSyncCountVec.size()) ? fPatternSyncCountVec[fEventIndex] : 0; 
      }
      UInt_t GetPairSyncCount() const { 
	 return (fEventIndex < fPairSyncCountVec.size()) ? fPairSyncCountVec[fEventIndex] : 0; 
      }
      UInt_t GetTimeTrigStartTStable() const { 
	 return (fEventIndex < fTimeTrigStartTStableVec.size()) ? fTimeTrigStartTStableVec[fEventIndex] : 0; 
      }
      UInt_t GetTimeTrigEndTStable() const { 
	 return (fEventIndex < fTimeTrigEndTStableVec.size()) ? fTimeTrigEndTStableVec[fEventIndex] : 0; 
      }
      UInt_t GetDurationLastTStable() const { 
	 return (fEventIndex < fDurationLastTStableVec.size()) ? fDurationLastTStableVec[fEventIndex] : 0; 
      }
      UInt_t GetDurationLastSettle() const { 
	 return (fEventIndex < fDurationLastSettleVec.size()) ? fDurationLastSettleVec[fEventIndex] : 0; 
      }
      UInt_t GetStatusAtTrigger() const { 
	 return (fEventIndex < fStatusAtTriggerVec.size()) ? fStatusAtTriggerVec[fEventIndex] : 0; 
      }
      UInt_t GetLast32PatternSync() const { 
	 return (fEventIndex < fLast32PatternSyncVec.size()) ? fLast32PatternSyncVec[fEventIndex] : 0; 
      }
      UInt_t GetLast32PairSync() const { 
	 return (fEventIndex < fLast32PairSyncVec.size()) ? fLast32PairSyncVec[fEventIndex] : 0; 
      }
      UInt_t GetLast32HelicityState() const { 
	 return (fEventIndex < fLast32HelicityStateVec.size()) ? fLast32HelicityStateVec[fEventIndex] : 0; 
      }
      UInt_t GetLast32HelicityPatternStart() const { 
	 return (fEventIndex < fLast32HelicityPatternStartVec.size()) ? fLast32HelicityPatternStartVec[fEventIndex] : 0; 
      }
   protected:
      static TypeIter_t fgThisType;

   private:
      Int_t DecodeBlock(const UInt_t* rwhit, UInt_t nwords);

      UInt_t    fBlockNum;
      UInt_t    fEventsInBlock;
      UInt_t    fWordsInBlock;
      Int_t     fDebug;

      // Event-level values stored as vector arrays to hold all N blocklevel events simultaneously
      std::vector<Bool_t>    fHelReportedVec;
      std::vector<Bool_t>    fTstableActiveVec;
      std::vector<Bool_t>    fHelTrueVec;
      std::vector<Bool_t>    fEventPolarityVec;
      std::vector<Bool_t>    fPatternPolarityReportedVec;
      std::vector<Bool_t>    fPatternPolarityTrueVec;
      std::vector<UInt_t>    fPatternPhaseVec;
      std::vector<UInt_t>    fTriggerNumVec;
      std::vector<Double_t>  fTriggerTimeVec;
      std::vector<UInt_t>    fHelicitySeedVec;
      std::vector<UInt_t>    fFallingEdgeTStableVec;
      std::vector<UInt_t>    fRisingEdgeTStableVec;
      std::vector<UInt_t>    fPatternSyncCountVec;
      std::vector<UInt_t>    fPairSyncCountVec;
      std::vector<Double_t>  fTimeTrigStartTStableVec;
      std::vector<Double_t>  fTimeTrigEndTStableVec;
      std::vector<Double_t>  fDurationLastTStableVec;
      std::vector<Double_t>  fDurationLastSettleVec;
      std::vector<UInt_t>    fStatusAtTriggerVec;
      std::vector<UInt_t>    fLast32PatternSyncVec;
      std::vector<UInt_t>    fLast32PairSyncVec;
      std::vector<UInt_t>    fLast32HelicityStateVec;
      std::vector<UInt_t>    fLast32HelicityPatternStartVec;

      // Stateful sequential row extraction pointer
      mutable size_t fEventIndex; 

      // Bitwise assembly components for multi-word trigger calculations
      UInt_t    fSavedTcBits;
      UInt_t    fSavedTdBits;
      UInt_t    fSavedTeBits;
      UInt_t    fSavedTfBits;

      ClassDef(Decoder::HelicityDecoderModule, 0)
   };

} // namespace Decoder

#endif
