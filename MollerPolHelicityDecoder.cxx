#include "MollerPolHelicityDecoder.h"
#include "HelicityDecoderModule.h"
#include "THaEvData.h"
#include "THaDetMap.h"
#include "THaSlotData.h"
#include "THaVarList.h"
#include "THaVar.h"
#include "VarDef.h"


MollerPolHelicityDecoder::MollerPolHelicityDecoder(const char* name, const char* description, THaApparatus* a)
   : THaHelicityDet(name, description, a),
     fBlockNum(0), fEventsInBlock(0), fWordsInBlock(0), fTriggerNum(0), fTriggerTime(0),
     fHelReported(0), fHelTrue(0), fEventPolarity(0), fPatternPolarityReported(0),
     fPatternPolarityTrue(0), fPatternPhase(0),fHelicitySeed(0), fFallingEdgeTStable(0),
     fRisingEdgeTStable(0), fPatternSyncCount(0), fPairSyncCount(0), fTimeTrigStartTStable(0),
     fTimeTrigEndTStable(0), fDurationLastTStable(0), fDurationLastSettle(0), fStatusAtTrigger(0),
     fLast32PatternSync(0), fLast32PairSync(0), fLast32HelicityState(0), fLast32HelicityPatternStart(0) {}

THaAnalysisObject::EStatus MollerPolHelicityDecoder::Init( const TDatime& date ) {
   // 1. Bypass THaHelicityDet::Init() to avoid looking for db_hel.dat.
   //    Jump directly to the standard base detector initialization hook.
   THaAnalysisObject::EStatus status = THaDetectorBase::Init(date);
   if( status != kOK ) return status;

   // 2. Explicitly call your custom ReadDatabase method to parse db_<heldecoder_name>.dat
   Int_t dbstatus = ReadDatabase(date);
   if( dbstatus != kOK ) return THaAnalysisObject::kInitError;

   // 3. Set the global Podd initialization flag true
   fIsInit = true;
   return THaAnalysisObject::kOK;
}

Int_t MollerPolHelicityDecoder::ReadDatabase(const TDatime& date) {
   static const char* const here = "ReadDatabase";

   // 1. Let the base class execute first. 
   Int_t base_status = THaHelicityDet::ReadDatabase(date);
   if (base_status != kOK) return base_status;

   // 2. Open our module database 
   FILE* file = OpenFile(date);
   if (!file) return kFileError;

   Int_t map_info[] = { 0, 0, 0, 0, 0 };

   DBRequest request[] = {
      { "map",           map_info,         kInt, 5, false, -1},
      { "verbose",       &fDebug,          kInt, 0, true,   1},
      { "heldelay",      &fHelicityDelay,  kInt, 0, true,   1},
      { "bankid",        &fBank_ID,        kInt, 0, false,  0},
      { nullptr }
   };

   // 3. Load variables using the prefix (searches for "<heldecoder_name>.map")
   Int_t err = LoadDB(file, date, request, fPrefix);
   fclose(file);
   if (err != 0) return kInitError;

   // 4. Register parameters into the detector map natively
   fDetMap->Clear();
   if (fDetMap->AddModule(map_info[0], map_info[1], map_info[2], map_info[3], map_info[4]) < 0) {
      Error(Here(here), "Error registering layout map (crate %d, slot %d)", map_info[0], map_info[1]);
      return kInitError;
   }

   return kOK;
}

void MollerPolHelicityDecoder::Clear(Option_t* opt) {
   // 1. Invoke the base framework clean loop
   THaHelicityDet::Clear(opt);

   // 2. Clear out your event-level scalar tracking variables completely
   fHelReported                = 0;
   fHelTrue                    = 0;
   fPatternPolarityReported    = 0;
   fPatternPolarityTrue        = 0;
   fEventPolarity              = 0;
   fPatternPhase               = 0;
   fBlockNum                   = 0;
   fEventsInBlock              = 0;
   fWordsInBlock               = 0;
   fTriggerNum                 = 0;
   fTriggerTime                = 0.0;
   fHelicitySeed               = 0;
   fFallingEdgeTStable         = 0;
   fRisingEdgeTStable          = 0;
   fPatternSyncCount           = 0;
   fPairSyncCount              = 0;
   fTimeTrigStartTStable       = 0;
   fTimeTrigEndTStable         = 0;
   fDurationLastTStable        = 0;
   fDurationLastSettle         = 0;
   fStatusAtTrigger            = 0;
   fLast32PatternSync          = 0;
   fLast32PairSync             = 0;
   fLast32HelicityState        = 0;
   fLast32HelicityPatternStart = 0;
}


Int_t MollerPolHelicityDecoder::DefineVariables(EMode mode) {
   if (mode == kDefine && fIsSetup) return kOK;
   fIsSetup = true;

   // Register variables that you want added to the TTree
   RVarDef vars[] = {
      //{ "blk_num",            "Block Sequence Identification Number",           "fBlockNum" },
      //{ "evts_in_blk",        "Total Event Load Counts in Active Block",        "fEventsInBlock" },
      //{ "words_in_blk",       "Block Buffer Size Word Length Metrics",          "fWordsInBlock" },
      //{ "helicity_seed",      "Recovered Pseudo-Random Helicity Seed",          "fHelicitySeed" },
      //{ "hist_pat_sync",      "History Register Matrix: Last 32 Pat Sync",      "fLast32PatternSync" },
      //{ "hist_pair_sync",     "History Register Matrix: Last 32 Pair Sync",     "fLast32PairSync" },
      //{ "hist_hel_window",    "Register of last 32 helicity window states",     "fLast32HelicityState" },
      //{ "hist_hel_pattern",   "Register of last 32 helicity pattern states",    "fLast32HelicityPatternStart" },
      { "tstable_active",     "Tstable currently active",                       "fTstableActive" },
      { "hel_reported",       "Current reported (delayed) helicity",            "fHelReported" },
      { "hel_true",           "Current predicted true helicity",                "fHelTrue" },
      { "patpol_reported",    "Current reported pattern polarity",              "fPatternPolarityReported" },
      { "patpol_true",        "Current predicted true pattern polarity",        "fPatternPolarityTrue" },
      { "pat_phase",          "Current phase in helicity pattern",              "fPatternPhase" },
      { "trig_num",           "Trigger Processing Pipeline Sequence ID",        "fTriggerNum" },
      { "trig_time",          "44-Bit Core System Epoch Clock Timestamp",       "fTriggerTime" },
      { "n_tstable_fall",     "Falling Edge T_STABLE Accumulator Value",        "fFallingEdgeTStable" },
      { "n_tstable_rise",     "Rising Edge T_STABLE Accumulator Value",         "fRisingEdgeTStable" },
      { "n_patsync",          "Total Pattern Sync Sequence Window Counts",      "fPatternSyncCount" },
      { "n_pairsync",         "Total Pair Sync Sequence Window Counts",         "fPairSyncCount" },
      { "time_tstable_start", "Time from T_STABLE Start (ns)",                  "fTimeTrigStartTStable" },
      { "time_tstable_end",   "Time from T_STABLE End (ns)",                    "fTimeTrigEndTStable" },
      { "tstable_dur",        "Time Duration of Previous T_STABLE (ns)",        "fDurationLastTStable" },
      { "tsettle_dur",        "Time Duration of Previous T_SETTLE (ns)",        "fDurationLastSettle" },
      { "status",             "Internal Helicity Module State Register",        "fStatusAtTrigger" },
      { nullptr }
   };

   return DefineVarsFromList(vars, mode);
}

Int_t MollerPolHelicityDecoder::Decode(const THaEvData& evdata) {
   if (!fIsInit) return -1;

   auto* slot_data = fDetMap->GetModule(0); 
   if (!slot_data) return -1;
  
   Int_t crate = slot_data->crate;
   Int_t slot  = slot_data->slot;

   // Direct dynamic cast to query your vector-based getters
   auto* module = dynamic_cast<const Decoder::HelicityDecoderModule*>(evdata.GetModule(crate, slot));
  
   if (module) {
      fBlockNum                   = module->GetBlockNum();
      fEventsInBlock              = module->GetEventsInBlock();
      fWordsInBlock               = module->GetWordsInBlock();
    
      // Dynamically extracts the distinct value matching the current fEventIndex row!
      fTriggerNum                 = module->GetTriggerNum();
      fTriggerTime                = static_cast<Double_t>(module->GetTriggerTime());
      fHelicitySeed               = module->GetHelicitySeed();
      fTstableActive              = module->GetTstableActive();
      fFallingEdgeTStable         = module->GetFallingEdgeTStable();
      fRisingEdgeTStable          = module->GetRisingEdgeTStable();
      fEventPolarity              = module->GetEventPolarity();
      fPatternSyncCount           = module->GetPatternSyncCount();
      fPairSyncCount              = module->GetPairSyncCount();
      fTimeTrigStartTStable       = module->GetTimeTrigStartTStable();
      fTimeTrigEndTStable         = module->GetTimeTrigEndTStable();
      fDurationLastTStable        = module->GetDurationLastTStable();
      fDurationLastSettle         = module->GetDurationLastSettle();
      fStatusAtTrigger            = module->GetStatusAtTrigger();
      fLast32PatternSync          = module->GetLast32PatternSync();
      fLast32PairSync             = module->GetLast32PairSync();
      fLast32HelicityState        = module->GetLast32HelicityState();
      fLast32HelicityPatternStart = module->GetLast32HelicityPatternStart();
      fHelReported                = module->GetHelReported();
      fHelTrue                    = module->GetHelTrue();
      fPatternPolarityReported    = module->GetPatternPolarityReported();
      fPatternPolarityTrue        = module->GetPatternPolarityTrue();
      fPatternPhase               = module->GetPatternPhase();
      UInt_t seed = fHelicitySeed;
      for(int i=0; i<fHelicityDelay; ++i)GetRandbit30(seed);
      fPatternPolarityTrue = ((seed & 0x1)==1);
      //True helicity of a given window given by polarity of pattern
      //and whether or not its current helicity is the same or different
      //from the pattern polarity
      fHelTrue = (fEventPolarity  != fPatternPolarityTrue);

	 
      // std::cout << "Event: " << fTriggerNum 
      //           << " | Helicity seed: " << fHelicitySeed 
      //           << " | Time Tstable fall: " << fTimeTrigStartTStable
      //           << " | Time Tstable rise: " << fTimeTrigEndTStable
      //           << " | Trigger status: " << fStatusAtTrigger
      //           << " | Tstable duration: " << fDurationLastTStable
      // 	      << std::endl;

      module->AdvanceSubEvent();
   } else {
      std::cout << "Module pointer is NULL for crate " << crate << ", slot " << slot << std::endl;
   }
  
   return 0;
}

Bool_t MollerPolHelicityDecoder::CheckPredictor(UInt_t& testval)
{
   //Check if the lowest two bits (latest two reported helicities pattern polarities)
   //are predicted given by the 30 highest bite (the signs of the previous 30 patterns)
   Bool_t retval = 0;
   UInt_t val = (testval>>2)&0x3fffffff;
   GetRandbit30(val);
   GetRandbit30(val);
   if (fDebug>1) std::cerr << std::hex << "Original seed: " << testval
			   << "; regenerated seed: " << val
			   << std::endl;
   if (testval == val) {
      if ( fDebug>1 ) std::cout << "Helicity seed is good" << std::endl;
      retval = true;
   } else {
      if (fDebug>0||1) std::cerr << "PROBLEM reconstructing helicity seed!!" << std::endl;
      retval = false;
      std::cerr << std::hex << "Original seed: " << testval
		<< "; regenerated seed: " << val
		<< std::endl;
   }
   return retval;
}

   
UInt_t MollerPolHelicityDecoder::GetRandbit30(UInt_t& ranseed)
{
   /** This is a 30 bit random bit generator according to the "new" algorithm
       described in "G0 Helicity Digital Controls" by E. Stangland, R. Flood, H. Dong.


       The helicity board uses a maximum-length linear feedback shift registers
       for the generation of a pseudo-random sequence of bits.  The length of the
       register (24 bits or 30 bits) defines the length before a sequence is
       repeated: 2^n - 1.

       For a mathematical introduction to the generation of pseudo-random numbers
       with maximum-length linear feedback shift registers (LFSR), see the
       following web references:
       http://en.wikipedia.org/wiki/Linear_feedback_shift_register
       http://www.newwaveinstruments.com/resources/articles/m_sequence_linear_feedback_shift_register_lfsr.htm

       In particular, the used solutions are to place XNOR taps at the bits
       24 stages, 4 taps:  (47 sets)
       [24, 23, 21, 20]
       30 stages, 4 taps:  (104 sets)
       [30, 29, 28, 7]
   **/

   UInt_t bit7    = (ranseed & 0x00000040) != 0;
   UInt_t bit28   = (ranseed & 0x08000000) != 0;
   UInt_t bit29   = (ranseed & 0x10000000) != 0;
   UInt_t bit30   = (ranseed & 0x20000000) != 0;

   UInt_t result = (bit30 ^ bit29 ^ bit28 ^ bit7) & 0x1;

   ranseed =  ( (ranseed << 1) | result ) & 0x3FFFFFFF;

   return(result);
}


ClassImp(MollerPolHelicityDecoder)



