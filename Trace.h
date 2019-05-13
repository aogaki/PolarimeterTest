//JMM, Jan 2015
//This class handles the processing of individual traces from a digitizer

#ifndef _Trace_incl
#define _Trace_incl

#include <iostream>
#include <cstdlib>
#include <string>
#include <signal.h>
#include <vector>
#include <map>
#include <math.h>
#include <algorithm>
#include <stdint.h>
#include <numeric>
#include "Math/Interpolator.h"

using namespace std;

class Trace {

 public:
  //default constructor
  Trace(vector<short> trace, map<string,int> param_map2, uint32_t trig_time_offset2 = 0, int event_number2 = 0);
  Trace(map<string,int> param_map2);
  ~Trace();

  //get/set methods
  inline void Set_Raw_Trace(vector<short> trace) {raw_trace = trace;}
  inline vector<short> Get_Raw_Trace() {return raw_trace;}
  inline vector<short> Get_Deriv() {return deriv;}
  inline int Get_Num_Pulses() {return num_pulses;}
  inline vector<double> Get_Trig_Times() {return trig_times;}
  inline uint32_t Get_Trig_Time_Offset() {return trig_time_offset;}
  inline uint32_t Get_Det_No() {return det_no;}
  inline void Set_Det_No(uint32_t det_no2) {det_no = det_no2;}
  inline uint32_t Get_Blk_Transfer_ID() {return blk_transfer_id;}
  inline vector<double> Get_Bsub_Trace() {return bsub_trace;}
  inline int Get_Start_Bsub_Time() {return start_bsub_time;}
  inline int Get_End_Bsub_Time() {return end_bsub_time;}
  inline double Get_Baseline() {return baseline;}
  inline vector<double> Get_QS() {return qs;}
  inline vector<double> Get_QL() {return ql;}
  inline vector<double> Get_Phmax() {return phmax;}
  inline vector<int> Get_Overlap() {return overlap;}
  inline vector<int> Get_In_Window() {return in_window;}
  inline vector<int> Get_Saturated() {return saturated;}
  inline map<string,int> Get_Param_Map() {return param_map;}
  inline bool Get_Good_Trace() {return good_trace;}
  inline void Set_Good_Trace(bool good_trace2) {good_trace = good_trace2;}
  inline int Get_Threshold_Type() {return threshold_type;}
  inline void Set_Threshold_Type(int threshold_type2) {threshold_type = threshold_type2;}
  inline void Set_Cur_Trace(int cur_trace2) {event_number = cur_trace2;}
  inline int Get_Cur_Trace() { return event_number; }
  inline void Set_Trig_Time_Offset(uint32_t trig_time_offset2) {trig_time_offset = trig_time_offset2;}
  inline void Set_Blk_Transfer_ID(uint32_t blk_transfer_id2) {blk_transfer_id = blk_transfer_id2;}
  inline bool Get_Used_Thorough_Bsub() { return used_thorough_bsub;}
  inline vector<ROOT::Math::Interpolator*> Get_Inter_Keeper() { return inter_keeper; }
  inline vector<float> Get_Inter_Bounds() { return inter_bounds; }

  //actual methods
  void Initialize(map<string,int> param_map2); //initialize is called by both constructors
  void Determine_Threshold_Type();
  void Make_Deriv(); //make the pulse derivative
  void Make_Num_Pulses_Deriv_Threshold(); //find the number of pulses using a threshold on the derivative
  void Make_Num_Pulses_LED();
  void Make_Trig_Times_ZC();
  void Make_Trig_Times_LED();
  void Do_CS_Fit(bool write_trig_times);
  void Make_Trig_Times_CS();
  void Make_Trig_Times_CFD(float delay, float atten);
  void Check_For_Saturated();
  void Make_Saturated_Times();
  void Make_Trig_Times(); //call the appropriate trigger time method
  bool Make_Baseline_Sub_Deriv_Threshold(); //make the baseline-subtracted trace using the deriv threshold method
  void Make_Baseline_Sub_LED_Thorough(); //make the baseline-subtracted trace using a thorough method
  bool Make_Baseline_Sub_LED_Rough(); //make the baseline-subtracted trace using the beginning of the pulse
  void Make_Pulse_Amplitude(); //find the maximum amplitude of the pulse
  void Improve_Num_Pulses(); //improve the number of pulses calculation by killing pulses due to ringing effects
  void Make_Charges(); //make the integration windows and do the pulse integration
  void Reset_Values();
  void Process(); //execute the above methdos in the correct order

  static int ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG; //count the number of times this error message appears
  static int ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG_MAX; //when the message has appeared this many times, suppress the message
  const static int ZC_TIME_METHOD = 1;
  const static int LED_TIME_METHOD = 2;
  const static int CS_TIME_METHOD = 3;
  const static int CFD_TIME_METHOD = 4;

 private:
  int DERIV_THRESHOLD; //threshold in the derivative to claim we found a pulse
  int TIME_GAP_BETWEEN_PULSES; //time gap req'd b/w pulses before we say there's another trigger
  int TIME_TO_WALK_BACK_FROM_LED; //time to walk backwards from LED trigger for baseline sub (deriv_threshold trigger can act as LED)
  int TIME_TO_AVG_FOR_BSUB; //# of samples to average over for baseline sub
  int RINGING_THRESH; //threshold at which to neglect ringing (expressed as % of orig pulse height)
  int RINGING_TIME_WINDOW; //time during which to apply the RINGING_THRESH threshold instead of DERIV_THRESHOLD
  int TIME_TO_WALK_BACK_Q; //time to walk back from pulse for charge integration (rel. to trig)
  int TIME_TO_WALK_FORWARD_QS; //time to walk forward from pulse for charge integration (short gate) (rel. to trig)
  int TIME_TO_WALK_FORWARD_QL; //time to walk forward from pulse for charge integration (long gate) (rel. to trig)
  int BASELINE_VARIATION; //# of channels variation for determining which samples to include in baseline estimation
  int BASELINE_SAMPLES; //# of samples at the beginning of the pulse to use for the baseline estimation
  int LED_THRESHOLD; //LED threshold for triggers
  int TIME_TRIGGER_METHOD; //which method to use for determining the pulse arrival time
  int CFD_DELAY; //delay parameter for CFD time trigger method
  int CFD_ATTEN; //attenuation parameter for CFD time trigger method (in %)

  
  map<string,int> param_map; //map of the above parameters, passed to Trace in the constructor

  vector<short> raw_trace; //the raw trace from the digitizer
  vector<short> deriv; //its derivative
  int num_pulses; //the number of pulses in the trace
  vector<int> led_times; //a vector of the LED times of the pulses in the trace (ascending order in time)
  vector<double> trig_times; //the trig times of the pulses in the trace (ascending order in time)
  double baseline; //the baseline of the raw_pulse
  vector<double> bsub_trace; //baseline subtracted pulse
  int start_bsub_time; //the integration window to determine the baseline (start to end bsub time)
  int end_bsub_time;
  vector<double> qs; //short gate charge, indexed according to trig_times
  vector<double> ql; //long gate charge, indexed according to trig_times
  vector<int> overlap; //if gates for two different pulses overlap, set to 1, otherwise 0
  vector<int> in_window; //if gates are within the window, set to 1, otherwise 0
  vector<int> saturated; //if pulse is saturated, set to 1, otherwise 0
  vector<double> phmax; //maximum pulse height, indexed according to trig_times
  uint32_t trig_time_offset; //the time offset according to the header of the event
  uint32_t blk_transfer_id; //the block transfer id according to the header of the event
  int event_number; //the event number from the constructor
  bool good_trace; //if it's a good trace (if the baseline subtraction can be performed) set to true, else false
  bool used_thorough_bsub;
  int threshold_type; // = 0 for LED threshold, = 1 for deriv threshold
  bool print_uncommon_errors; //usually errors with finding the phmax, can ignore
  bool CS_fit_done; //whether or not the CS fit has been done for determining the phmax and CS trigger time
  
  vector<ROOT::Math::Interpolator*> inter_keeper; //keep an interpolator for plotting (if necessary)
  vector<float> inter_bounds; //boundaries of inter_keeper for plotting.  NOTE: xlow_0, xhigh_0, xlow_1, xhigh_1, etc.. where 0 or 1 indexes the pulse number within the trace

  uint32_t det_no;

};
#endif
