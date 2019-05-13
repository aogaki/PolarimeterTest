//JMM, Jan 2015

#include "Trace.h"
#include <boost/tuple/tuple.hpp>
#include "boost/tuple/tuple_comparison.hpp"
#include "Math/Interpolator.h"
#include "Math/Functor.h"
#include "Math/GSLMinimizer1D.h"

//set the static variables
int Trace::ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG = 0;
int Trace::ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG_MAX = 5;

using namespace std;

struct my_compare_boost_tuple
{
  bool operator()(const boost::tuple<double,int>& a, const boost::tuple<double,int>& b) const
  {
    return boost::get<0>(a) > boost::get<0>(b);
  }
}; 

Trace::Trace(vector<short> trace, map<string,int> param_map2, uint32_t trig_time_offset2, int event_number2) {
  Initialize(param_map2);
  raw_trace = trace;
  trig_time_offset = trig_time_offset2;
  event_number = event_number2;
}

Trace::Trace(map<string,int> param_map2) {
  Initialize(param_map2);
}

Trace::~Trace() { }

void Trace::Initialize(map<string,int> param_map2) {
  //clear all vectors
  num_pulses = 0;
  raw_trace.clear();
  deriv.clear();
  led_times.clear();
  trig_times.clear();
  baseline = 0;
  bsub_trace.clear();
  overlap.clear();
  in_window.clear();
  qs.clear();
  ql.clear();
  phmax.clear();
  inter_keeper.clear();
  inter_bounds.clear();

  threshold_type = -1;
  print_uncommon_errors = false;
  CS_fit_done = false;

  param_map = param_map2;

  //load params into adjustable parameters
  DERIV_THRESHOLD = param_map["DERIV_THRESHOLD"];
  TIME_GAP_BETWEEN_PULSES = param_map["TIME_GAP_BETWEEN_PULSES"];
  TIME_TO_WALK_BACK_FROM_LED = param_map["TIME_TO_WALK_BACK_FROM_LED"];
  TIME_TO_AVG_FOR_BSUB = param_map["TIME_TO_AVG_FOR_BSUB"];
  RINGING_THRESH = param_map["RINGING_THRESH"]; //% of original pulse amplitude
  RINGING_TIME_WINDOW = param_map["RINGING_TIME_WINDOW"];
  TIME_TO_WALK_BACK_Q = param_map["TIME_TO_WALK_BACK_Q"];
  TIME_TO_WALK_FORWARD_QS = param_map["TIME_TO_WALK_FORWARD_QS"];
  TIME_TO_WALK_FORWARD_QL = param_map["TIME_TO_WALK_FORWARD_QL"];
  BASELINE_VARIATION = param_map["BASELINE_VARIATION"];
  BASELINE_SAMPLES = param_map["BASELINE_SAMPLES"];
  LED_THRESHOLD = param_map["LED_THRESHOLD"];
  TIME_TRIGGER_METHOD = param_map["TIME_TRIGGER_METHOD"];
  CFD_DELAY = param_map["CFD_DELAY"];
  CFD_ATTEN = param_map["CFD_ATTEN"];

  Determine_Threshold_Type();
}

void Trace::Determine_Threshold_Type() {

  if(LED_THRESHOLD == 0 &&
     DERIV_THRESHOLD == 0) {
    cout << "Please set LED_THRESHOLD to a non-zero value to use a leading edge discriminator as the initial trigger, or set DERIV_THRESHOLD to a non-zero value to use a threshold on the derivative as the initial trigger" << endl;
    good_trace = false;
    return;
  }
  if(LED_THRESHOLD != 0 &&
     DERIV_THRESHOLD != 0) {
    cout << "Please do not set both LED_THRESHOLD and DERIV_THRESHOLD, because if both are set, then I don't know which threshold to use as an initial trigger" << endl;
    good_trace = false;
    return;
  }
  if(LED_THRESHOLD != 0) threshold_type = 0;
  else threshold_type = 1;
    
}

void Trace::Make_Deriv() {
  //if pulse is too small, can't make derivative
  if((int)raw_trace.size() < 3) {
    cout << "Cannot make derivative, raw trace size is " << raw_trace.size() << endl;
    return;
  }
  
  //push back the derivative of the pulse into deriv
  deriv.push_back(0);
  for(int i = 1; i < (int)raw_trace.size()-1; i++) {
    deriv.push_back((raw_trace[i+1]-raw_trace[i-1])/2);
  }
  //edge conditions
  deriv.push_back(deriv.back());
  deriv[0] = deriv[1]; 
  
}

void Trace::Make_Num_Pulses_Deriv_Threshold() {

  bool within_pulse = false; //keep track of whether or not we're within a pulse
  
  for(int i = 0; i < (int)deriv.size(); i++) {
    //if we were in a pulse, but it ended (deriv < thresh), set within_pulse to false and skip ahead TIME_GAP.. samples
    if(within_pulse) {
      if(fabs(deriv[i]) < fabs(DERIV_THRESHOLD)) {
	within_pulse = false;
	i+=TIME_GAP_BETWEEN_PULSES-1;
      }
    }
    //if we weren't in a pulse, and the deriv > thresh, we found a pulse.  save the time and increment num_pulses
    else {
      if(fabs(deriv[i]) > fabs(DERIV_THRESHOLD)) {
	led_times.push_back(i);
	within_pulse = true;
	num_pulses++;
      }
    }
  }
}

void Trace::Make_Num_Pulses_LED() {

  bool within_pulse = false; //keep track of whether or not we're within a pulse
  
  for(int i = 0; i < (int)bsub_trace.size(); i++) {
    //if we were in a pulse, but it ended (val < thresh), set within_pulse to false and skip ahead TIME_GAP.. samples
    if(within_pulse) {
      if(fabs(bsub_trace[i]) < LED_THRESHOLD) {
	within_pulse = false;
	i+=TIME_GAP_BETWEEN_PULSES-1;
      }
    }
    //if we weren't in a pulse, and the bsub_trace > thresh, we found a pulse.  save the time and increment num_pulses
    else {
      if(fabs(bsub_trace[i]) >= LED_THRESHOLD &&
	 i > 0 &&
	 fabs(bsub_trace[i-1]) < LED_THRESHOLD) {
	led_times.push_back(i-1);
	within_pulse = true;
	num_pulses++;
      }
    }
  }
}

void Trace::Make_Trig_Times_ZC() {

  if(num_pulses == 0) return; //if no pulses, quit
  

  //for all pulses, go to where the LED triggered.  then, advance forward until you hit a zero crossing, and record time
  for(int i = 0; i < (int)led_times.size(); i++) {
    if(saturated[i] == 0) {
      for(int j = led_times[i]; j < (int)deriv.size()-1; j++) {
	if(deriv[j]*deriv[j+1] <= 0) { //if true, we found a zero crossing
	  if(deriv[j+1]-deriv[j] == 0) {
	    trig_times.push_back((double)j+0.5);
	  }
	  else {
	    trig_times.push_back((double)j - ((double)deriv[j])/((double)(deriv[j+1]-deriv[j]))); //do a linear interpolation to find crossing time
	  }
	  break; //break out of this loop
      }
      }
      if((int)trig_times.size() != i+1) {
	trig_times.push_back((double)deriv.size()-1); //if the zero crossing happens after the trace ends, push back the last time possible
      }
    }
    else trig_times.push_back(0);
  }
}

void Trace::Make_Trig_Times_LED() {

  if(num_pulses == 0) return; //if no pulses, quit
  
  //for all pulses, go to where the LED triggered.  then, subtract out the LED_THRESHOLD and get zero crossing, record time
  int thetime;
  for(int i = 0; i < (int)led_times.size(); i++) {
    if(saturated[i] == 0) {
      thetime = led_times[i];
      trig_times.push_back((double)thetime + ((double)(LED_THRESHOLD-fabs(bsub_trace[thetime])))/(fabs(bsub_trace[thetime+1]-bsub_trace[thetime]))); //do a linear interpolation to find crossing time
    }
    else trig_times.push_back(0);
  }
}

void Trace::Do_CS_Fit(bool write_trig_times) {

  if((int)led_times.size()==0) return; //if no pulses, quit method
  vector<double> xx; //sample number
  vector<double> yy; //baseline subtracted amplitude
  int index = 0;
  double maxval = 0;
  //loop over pulses
  for(int i = 0; i < (int)led_times.size(); i++) {
    xx.clear();
    yy.clear();
    maxval = 0;
    index = 0;
    if(saturated[i] == 0) {
      //loop over samples within short gate of the trig time
      for(int j = (int)led_times[i]-TIME_TO_WALK_BACK_Q; j < (int)led_times[i] + TIME_TO_WALK_FORWARD_QS; j++) {
	if(j >= 0 && j < (int)bsub_trace.size()) {
	  xx.push_back((double)j); //set x to be the sample number
	  if(bsub_trace[j] > 0) yy.push_back(-1*(double)bsub_trace[j]);
	  else yy.push_back((double)bsub_trace[j]); //set y to be the bsub trace	  
	  if(fabs(bsub_trace[j]) > maxval && fabs(bsub_trace[j]) >= LED_THRESHOLD) {
	    maxval = fabs(bsub_trace[j]);
	    index = j;
	  }
	  else if(fabs(bsub_trace[j]) < maxval && 
		  (fabs(bsub_trace[j]) > LED_THRESHOLD || (j > 0 && fabs(bsub_trace[j-1]) >= LED_THRESHOLD))) {
	    maxval = 1e6;
	  }
	}
      }
      double guess_time = index;
      //end of trace condition
      if(fabs(guess_time - xx.back()) < .001 && fabs(xx.back() - (double)raw_trace.size() + 1) < .001) {
	xx.push_back((double)raw_trace.size());
	yy.push_back(0);
      }
      while(fabs(yy.back()) > fabs(bsub_trace[guess_time])) {
	xx.pop_back();
	yy.pop_back();
      }
      while(fabs(yy[0]) > fabs(bsub_trace[guess_time])) {
	xx.erase(xx.begin());
	yy.erase(yy.begin());
      }

      if(xx.size() < 3 || 
	 guess_time < xx[0]+.01 || guess_time > xx.back() - .01) {
	if(print_uncommon_errors) cout << "Error in finding the phmax - something wrong with endpoints for event " << event_number << endl;
	phmax.push_back(-1);	
	if(write_trig_times) trig_times.push_back(led_times[i]);
      }
      else if (xx[0] == guess_time) {
	phmax.push_back(yy[0]);
	if(write_trig_times) trig_times.push_back(led_times[i]);
      }
      else {
	
	ROOT::Math::Interpolator inter(xx,yy, ROOT::Math::Interpolation::kCSPLINE); //ROOT interpolator, use cubic spline

	//cout << "inter evals: " << endl;
	//cout << inter.Eval(xx[0]) << " " << inter.Eval(guess_time) << " " << inter.Eval(xx.back()) << endl;
	//cout << "min range: " << endl;
	//cout << guess_time << " " << xx[0]+.1 << " " << xx.back() - .1 << endl;
	
	inter_keeper.push_back(new ROOT::Math::Interpolator(xx,yy, ROOT::Math::Interpolation::kCSPLINE));
	inter_bounds.push_back(xx[0]+.01);
	inter_bounds.push_back(xx.back()-.01);
	
	ROOT::Math::Functor1D func_inter(&inter,&ROOT::Math::Interpolator::Eval);
	//cout << func_inter(ceil(led_times[i])) << " " << func_inter(floor(led_times[i]+interp_len-1)) << " " << func_inter(led_times[i]+3) << endl;
      // get the minimum value
	ROOT::Math::GSLMinimizer1D minBrent;
	if(fabs(func_inter(guess_time)) < fabs(func_inter(xx[0]+.01)) ||
	   fabs(func_inter(guess_time)) < fabs(func_inter(xx.back()-.01))) {
	  if(print_uncommon_errors) cout << "Error in finding the phmax - something wrong with endpoints for event " << event_number << endl;
	  phmax.push_back(-1);
	  if(write_trig_times) trig_times.push_back(led_times[i]);
	}
	else {
	  minBrent.SetFunction(func_inter,guess_time,xx[0]+.01,xx.back()-.01);
	  bool result = minBrent.Minimize(1000,0.001,0.001);
	  if(!result) {
	    cout << "Minimizer failed for pulse " << event_number << endl;
	  }
	  if(write_trig_times) trig_times.push_back(minBrent.XMinimum());
	  phmax.push_back(fabs(minBrent.FValMinimum()));
	}
      }
    }
    else {
      //cout << "saturated: " << event_number << endl;
      if(write_trig_times) trig_times.push_back(led_times[i]);
      phmax.push_back(fabs(baseline));
    }
   // std::cout << "Found minimum: x = " << minBrent.XMinimum() 
   //           << "  f(x) = " << minBrent.FValMinimum() << std::endl;
  }

  CS_fit_done = true;
}

void Trace::Make_Trig_Times_CS() {

  if((int)led_times.size()==0) return; //if no pulses, quit method
  else if(!CS_fit_done) Do_CS_Fit(true);

  return;

  /*
  vector<double> xx; //sample number
  vector<double> yy; //baseline subtracted amplitude
  int index = 0;
  double maxval = 0;
  //loop over pulses
  for(int i = 0; i < (int)led_times.size(); i++) {
    xx.clear();
    yy.clear();
    maxval = 0;
    index = 0;
    if(saturated[i] == 0) {
      
      //loop over samples within short gate of the trig time
      for(int j = (int)led_times[i]-TIME_TO_WALK_BACK_Q; j < (int)led_times[i] + TIME_TO_WALK_FORWARD_QS; j++) {
	if(j >= 0 && j < (int)bsub_trace.size()) {
	  xx.push_back((double)j); //set x to be the sample number
	  yy.push_back((double)bsub_trace[j]); //set y to be the bsub trace
	  if(fabs(bsub_trace[j]) > maxval && fabs(bsub_trace[j]) > LED_THRESHOLD) {
	    maxval = fabs(bsub_trace[j]);
	    index = j;
	  }
	  else if(fabs(bsub_trace[j]) < maxval && fabs(bsub_trace[j]) > LED_THRESHOLD) {
	    maxval = 1e6;
	  }
	}
      }
      double guess_time = index;
      //end of trace condition
      if(fabs(guess_time - xx.back()) < .001 && fabs(xx.back() - (double)raw_trace.size() + 1) < .001) {
	xx.push_back((double)raw_trace.size());
	yy.push_back(0);
      }
      while(fabs(yy.back()) > fabs(bsub_trace[guess_time])) {
	xx.pop_back();
	yy.pop_back();
	if(xx.size() < 2) cout << "Error getting a good interval in Trace::Make_Trig_Times_CS()" << endl;
      }
      
      //cout << "vectors: " << endl;
      //for(int k = 0; k < (int)xx.size(); k++) cout << xx[k] << " " << yy[k] << endl;
      
      ROOT::Math::Interpolator inter(xx,yy, ROOT::Math::Interpolation::kCSPLINE); //ROOT interpolator, use cubic spline
      inter_keeper = new ROOT::Math::Interpolator(xx,yy, ROOT::Math::Interpolation::kCSPLINE); //ROOT interpolator, use cubic spline

      //cout << "inter evals: " << endl;
      //cout << inter.Eval(xx[0]) << " " << inter.Eval(guess_time) << " " << inter.Eval(xx.back()) << endl;
      //cout << "min range: " << endl;
      //cout << guess_time << " " << xx[0]+.1 << " " << xx.back() - .1 << endl;
      
      
      ROOT::Math::Functor1D func_inter(&inter,&ROOT::Math::Interpolator::Eval);
      //cout << func_inter(ceil(led_times[i])) << " " << func_inter(floor(led_times[i]+interp_len-1)) << " " << func_inter(led_times[i]+3) << endl;
      // get the minimum value
      ROOT::Math::GSLMinimizer1D minBrent;
      //cout << "pre: " << event_number << endl;
      minBrent.SetFunction(func_inter,guess_time,xx[0]+.1,xx.back()-.1);
      //cout << "post: " << func_inter(guess_time) << " " << func_inter(xx[0]+.1) << " " << func_inter(xx.back()-.1) << endl;
      bool result = minBrent.Minimize(1000,0.001,0.001);
      if(!result) {
	cout << "Failed for pulse " << event_number << endl;
      }
      trig_times.push_back(minBrent.XMinimum());
    }
    else {
      //cout << "saturated: " << event_number << endl;
      trig_times.push_back(0);
    }
   // std::cout << "Found minimum: x = " << minBrent.XMinimum() 
   //           << "  f(x) = " << minBrent.FValMinimum() << std::endl;
  }
  */
}

void Trace::Make_Trig_Times_CFD(float delay, float atten) {

  //make a delayed version of the pulse
  vector<double> bsub_trace_delay; 
  for(int i = 0; i < delay; i++) bsub_trace_delay.push_back(bsub_trace[0]);
  for(int i = 0; i < (int)bsub_trace.size()-delay; i++) {
    bsub_trace_delay.push_back(bsub_trace[i]);
  }

  //make an attenuated version of the pulse
  vector<double> bsub_trace_atten;
  vector<double> bsub_cfd;
  for(int i = 0; i < (int)bsub_trace.size(); i++) {
    bsub_trace_atten.push_back(-1*atten*bsub_trace[i]);
    bsub_cfd.push_back(bsub_trace_delay[i]+bsub_trace_atten[i]);
  }
  
  //look for zero crossing in bsub_cfd near led_times
  bool found = false;
  float biggest_diff = 0;
  for(int i = 0; i < (int)led_times.size(); i++) {
    found = false;
    biggest_diff = 0;
    for(int j = led_times[i] - 5; j < led_times[i] + 10+delay; j++) {
      //cout << j << " " <<  bsub_cfd[j] << endl;
      if(j > 0 && j < (int)bsub_cfd.size() &&
	 bsub_cfd[j] * bsub_cfd[j+1] <= 0 && fabs(bsub_trace[j]) > LED_THRESHOLD/10.0) {
	if(fabs(bsub_cfd[j]) + fabs(bsub_cfd[j+1]) > biggest_diff) {
	  biggest_diff = fabs(bsub_cfd[j]) + fabs(bsub_cfd[j+1]);
	  if(!found) {
	    if(bsub_cfd[j+1]-bsub_cfd[j] == 0) {
	      trig_times.push_back((double)j+0.5);
	    }
	    else {
	      trig_times.push_back((double)j - ((double)bsub_cfd[j])/((double)(bsub_cfd[j+1]-bsub_cfd[j]))); //do a linear interpolation to find crossing time
	    }
	  }
	  else {
	    if(bsub_cfd[j+1]-bsub_cfd[j] == 0) {
	      trig_times.back() = (double)j+0.5;
	    }
	    else {
	      trig_times.back() = (double)j - ((double)bsub_cfd[j])/((double)(bsub_cfd[j+1]-bsub_cfd[j])); //do a linear interpolation to find crossing time
	    }
	  }
	  found = true;
	}
      }
    }
    if(!found) {
      /*
      for(int j = led_times[i] - 2; j < led_times[i] + 10; j++) {
	if(j > 0 && j < (int)bsub_cfd.size())
	  cout << j << " " <<  bsub_cfd[j] << endl;
      }
      */
      //cout << "Couldn't find CFD zero crossing" << endl;
      //end of trace condition
      trig_times.push_back(bsub_cfd.size()-1);
    }
  }
}

void Trace::Check_For_Saturated() {

  saturated.clear();
  for(int i = 0; i < (int)led_times.size(); i++) {
    saturated.push_back(0);
    for(int j = (int)led_times[i]-TIME_TO_WALK_BACK_Q; j < (int)led_times[i] + TIME_TO_WALK_FORWARD_QS+3; j++) {
      if(j >=0 && j < (int)raw_trace.size()) {
	if(raw_trace[j] < 2 || raw_trace[j] > 16382) saturated[i] = 1;
      }
    }
  }
  //  for(int i = 0; i < (int)saturated.size(); i++) cout << "sat: " << i << " " << saturated[i] << endl;
}

void Trace::Make_Saturated_Times() {

  if(accumulate(saturated.begin(),saturated.end(),0) > 0) {
    
    vector<int> xx_to_avg;
    for(int i = 0; i < (int)led_times.size(); i++) {
      if(saturated[i]) {
	xx_to_avg.clear();
	for(int j = led_times[i] - TIME_TO_WALK_BACK_Q; j < led_times[i] + TIME_TO_WALK_FORWARD_QS+10; j++) {
	  if(j >= 0 && j < (int)raw_trace.size() &&
	     (raw_trace[j] < 2 || raw_trace[j] > 16382)) xx_to_avg.push_back(j);
	}
	trig_times[i] = ((double)accumulate(xx_to_avg.begin(),xx_to_avg.end(),0))/((double)xx_to_avg.size());
      }
    }

  }

}

void Trace::Make_Trig_Times() {

  Check_For_Saturated();

  switch(param_map["TIME_TRIGGER_METHOD"]) {
  case ZC_TIME_METHOD:
    Make_Trig_Times_ZC();
    break;
  case LED_TIME_METHOD:
    Make_Trig_Times_LED();
    break;
  case CS_TIME_METHOD:
    Make_Trig_Times_CS();
    break;
  case CFD_TIME_METHOD:
    Make_Trig_Times_CFD(param_map["CFD_DELAY"],(float)param_map["CFD_ATTEN"]*0.01);
    break;
  default:
    Make_Trig_Times_ZC();
    break;
  }

  Make_Saturated_Times();

}

bool Trace::Make_Baseline_Sub_Deriv_Threshold() {

  if(num_pulses == 0) return true; //if no pulses, quit

  //set the baseline subtraction integration window using parameters
  end_bsub_time = led_times[0] - TIME_TO_WALK_BACK_FROM_LED;
  start_bsub_time = end_bsub_time - TIME_TO_AVG_FOR_BSUB;

  //if we're starting too early, set start to 0
  if(start_bsub_time < 0) start_bsub_time = 0; 
  if(end_bsub_time < 0) { //if we're also ending too early, give an error
    if(ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG < ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG_MAX) { //if we haven't given this error message too much already
      ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG++;
      //error message
      cout << "Adjust TIME_TO_AVG_FOR_BSUB and TIME_TO_WALK_BACK_FROM_LED" << endl;
      cout << "Currently, the baseline subtraction method window is too far back in the trace" << endl;
      cout << "Error in event number: " << event_number << endl;
      if(ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG==ADJUST_TIME_TO_AVG_FOR_BSUB_ERROR_MSG_MAX) {
	cout << "Will suppress future warnings about this error" << endl; //note that future error messages will be suppressed
      }
    }
    return false; //return false-> not a good pulse, because no baseline subtraction performed
  }
  //integrate over the window
  int total = 0;
  for(int i = start_bsub_time; i <= end_bsub_time; i++) {
    total += raw_trace[i];
  }
  //get the baseline from the integral
  baseline = ((double)total)/((double)(end_bsub_time-start_bsub_time+1));
  //make the baseline subtracted trace
  for(int i = 0; i < (int)raw_trace.size(); i++) {
    bsub_trace.push_back(raw_trace[i] - baseline);
  }
  return true; //return true -> good pulse, baseline subtraction performed
}

bool Trace::Make_Baseline_Sub_LED_Rough() {
  //this method will do the baseline subtraction based on the first BASELINE_SAMPLES
  //if it finds too much variation in those samples, it will return false
  //else return true

  if(BASELINE_SAMPLES == 0) return false;

  for(int i = 0; i < BASELINE_SAMPLES; i++) {
    baseline += raw_trace[i];
  }
  baseline = baseline/((double)BASELINE_SAMPLES);

  for(int i = 0; i < BASELINE_SAMPLES; i++) {
    if(fabs(raw_trace[i] - baseline) > BASELINE_VARIATION) return false;
  }
  
  //make the baseline subtracted trace
  for(int i = 0; i < (int)raw_trace.size(); i++) {
    bsub_trace.push_back(raw_trace[i] - baseline);
  }
  
  used_thorough_bsub = false;

  return true;
}

void Trace::Make_Baseline_Sub_LED_Thorough() {
  //this method will do the baseline subtraction based on looking for samples where the baseline does not vary by more than BASELINE_VARIATION
  //first step - identify all the samples that are within BASELINE_VARIATION*2 of both neighboring samples
  vector<int> samples_close_to_neighbors; //3-> 9.4, 4->8.72
  int neighbor_len = 4;
  bool good_pt;
  for(int i = neighbor_len; i < (int)raw_trace.size()-neighbor_len; i++) {
    good_pt = true;
    for(int j = -neighbor_len; j <= neighbor_len; j++) {
      if(abs(raw_trace[i] - raw_trace[i+j]) > 2*BASELINE_VARIATION) {
  	good_pt = false;
  	break;
      }
    }
    if(good_pt) {
      samples_close_to_neighbors.push_back(i);
    }
  }
  for(int i = 0; i < neighbor_len; i++) samples_close_to_neighbors.push_back(i);
  for(int i = (int)raw_trace.size()-neighbor_len; i < (int)raw_trace.size(); i++) samples_close_to_neighbors.push_back(i);

  //next step - calculate the average, and reject all samples that are more than BASELINE_VARIATION from the average
  //if the number of samples in the average changes, repeat.  else, done
  bool repeat;
  int num_to_kill = 5;
  if(samples_close_to_neighbors.size() < 10) num_to_kill = 1;
  else if(samples_close_to_neighbors.size() < 20) num_to_kill = 3;
  int num_repeats = 0;
  vector< boost::tuple<double,int> > residuals;
  vector<int> indicies_to_kill;
  int n_in_samples;

  do {
    repeat = false;
    baseline = 0;
    n_in_samples = (int)samples_close_to_neighbors.size();

    for(int i = 0; i < n_in_samples; i++) {
      baseline += raw_trace[samples_close_to_neighbors[i]];
    }
    baseline = baseline/((double)n_in_samples);
    
    for(int i = 0; i < n_in_samples; i++) {
      residuals.push_back(boost::make_tuple(fabs(raw_trace[samples_close_to_neighbors[i]] - baseline),i));
    }

    sort(residuals.begin(),residuals.end(),my_compare_boost_tuple());
    if(boost::get<0>(*residuals.begin()) > BASELINE_VARIATION) {
      for(vector<boost::tuple<double,int> >::iterator iter = residuals.begin(); iter != residuals.begin()+num_to_kill; iter++) indicies_to_kill.push_back(boost::get<1>(*iter));
      
      sort(indicies_to_kill.begin(),indicies_to_kill.end());
      
      for(vector<int>::reverse_iterator riter = indicies_to_kill.rbegin(); riter != indicies_to_kill.rend(); riter++) {
	samples_close_to_neighbors.erase(samples_close_to_neighbors.begin()+(*riter));
      }
      repeat = true;
      num_repeats++;
      if(samples_close_to_neighbors.size() < 10) num_to_kill = 1;
      else if(samples_close_to_neighbors.size() < 20) num_to_kill = 3;
    }
    residuals.clear();
    indicies_to_kill.clear();
  } while(repeat);

  //make the baseline subtracted trace
  for(int i = 0; i < (int)raw_trace.size(); i++) {
    bsub_trace.push_back(raw_trace[i] - baseline);
  }

  used_thorough_bsub = true;
  
}

void Trace::Improve_Num_Pulses() {
  //try to remove spurious second triggers caused by ringing of a large signal
  
  if(num_pulses < 2) return; //if there aren't multiple triggers, quit this method

  int cur_pulse = 0; //index of current pulse under analysis

  //testing purposes only
  // cout << num_pulses << endl;
  // cout << "Trig: " << endl;
  // for(int i = 0; i < num_pulses; i++) {
  //   cout << trig_times[i] << endl;
  // }
  // cout << "LED: " << endl;
  // for(int i = 0; i < num_pulses; i++) {
  //   cout << led_times[i] << endl;
  // }
  
  //look at all pulses.  if any pulse could be explained by the ringing of a prior pulse, remove it from trig_times and led_times
  for(int i = 1; i < num_pulses; i++) {
    if(fabs(trig_times[i] - trig_times[cur_pulse]) < RINGING_TIME_WINDOW) { //within ringing time window
      if(fabs(phmax[i]) < fabs(phmax[cur_pulse])*RINGING_THRESH*0.01) { //if the pulse is smaller than RINGING_THRESH % of previous pulse
	//kill the pulse from the interpolator
	if((int)inter_keeper.size() > 0) {
	  for(int j = (int)inter_keeper.size()*2-2; j >= 0; j-=2) {
	    if(inter_bounds[j] < trig_times[i] && inter_bounds[j+1] > trig_times[i]) {
	      inter_keeper.erase(inter_keeper.begin()+j/2);
	      inter_bounds.erase(inter_bounds.begin()+j+1);
	      inter_bounds.erase(inter_bounds.begin()+j);
	      break;
	    }
	  }
	}

	//kill the pulse from the vectors	
	trig_times.erase(trig_times.begin()+i);
	led_times.erase(led_times.begin()+i);
	phmax.erase(phmax.begin()+i);
	num_pulses--;
	i--;
      }
      //if it's big enough to be it's own pulse, set it to cur_pulse
      else {
	cur_pulse = i;
      }
    }
    //if there aren't any within the RINGING_TIME_WINDOW, set the next one to cur_pulse
    else {
      cur_pulse = i;
    }
  }
}

void Trace::Make_Charges() {

  if(trig_times.size() == 0) return; //if there's no pulses, quit method

  //set the integration windows (low, high for short gate, high for long gate)
  vector<double> gate_low, gate_high_s, gate_high_l;
  for(int i = 0; i < (int)trig_times.size(); i++) {
    //use the parameters to set the windows
    gate_low.push_back(trig_times[i]-TIME_TO_WALK_BACK_Q);
    gate_high_s.push_back(trig_times[i]+TIME_TO_WALK_FORWARD_QS);
    gate_high_l.push_back(trig_times[i]+TIME_TO_WALK_FORWARD_QL);
    //initialize the vectors of charges, the overlap vector, and the in_window vector
    qs.push_back(0);
    ql.push_back(0);
    overlap.push_back(0); //default - no overlap
    in_window.push_back(1); //default - within window
  }
  

  //check for overlaps and to see if the gates are within the trace
  for(int i = 0; i < (int)trig_times.size(); i++) {
    //look for overlaps, iterate over all other pulses
    for(int j = i+1; j < (int)trig_times.size(); j++) {
      //if the low end is less than the high end of the preceeding pulse, set overlaps to true (1)
      if(gate_low[j] < gate_high_l[i]) {
	overlap[i] = 1;
	overlap[j] = 1;
	break;
      }
    }
    //if the high end of the gate is out of the window of the trace, set it to the boundary and set in_window to false
    if(gate_high_l[i] > (int)raw_trace.size()) {
      in_window[i] = 0;
      gate_high_l[i] = raw_trace.size()-1;
    }
    if(gate_high_s[i] > (int)raw_trace.size()) {
      in_window[i] = 0;
      gate_high_s[i] = raw_trace.size()-1;
    }
    //if the low end is too early, set it to the boundary and set in_window to false
    if(gate_low[i] < 0) {
      in_window[i] = 0;
      gate_low[i] = 0;
    }
  }

  //use a faster method for trapezoidal integration (assuming a linear interpolation)
  if(gate_low.size() > 0) {
    //xx is sample, yy is bsub_trace value
    double integral = 0;
    int below_index, upper_index;
    double new_gate_low;
    //loop over each integration window
    for(int i = 0; i < (int)gate_low.size(); i++) {
      integral = 0;
      //make integral for the short gate
      for(int j = ceil(gate_low[i])+1; j < floor(gate_high_s[i]); j++) {
	integral+= bsub_trace[j];
      }
      //boundary conditions - low
      below_index = floor(gate_low[i]);
      upper_index = ceil(gate_low[i]);
      integral+=0.5*bsub_trace[upper_index];
      integral += (0.5*bsub_trace[upper_index] + 
		   0.5*((gate_low[i] - below_index)*bsub_trace[upper_index]+
			(upper_index - gate_low[i])*bsub_trace[below_index]))*(upper_index-gate_low[i]);
      
      //boundary conditions - high
      below_index = floor(gate_high_s[i]);
      upper_index = ceil(gate_high_s[i]);
      integral+=0.5*bsub_trace[below_index];
      integral += (0.5*bsub_trace[below_index] + 
		   0.5*((gate_high_s[i] - below_index)*bsub_trace[upper_index]+
			(upper_index - gate_high_s[i])*bsub_trace[below_index]))*(gate_high_s[i] - below_index);
      
      qs[i] = fabs(integral);

      //do ql integral
      new_gate_low = gate_high_s[i];

      for(int j = ceil(new_gate_low)+1; j < floor(gate_high_l[i]); j++) {
	integral+= bsub_trace[j];
      }
      //boundary conditions - low
      below_index = floor(new_gate_low);
      upper_index = ceil(new_gate_low);
      integral+=0.5*bsub_trace[upper_index];
      integral += (0.5*bsub_trace[upper_index] + 
		   0.5*((new_gate_low - below_index)*bsub_trace[upper_index]+
			(upper_index - new_gate_low)*bsub_trace[below_index]))*(upper_index-new_gate_low);
      
      //boundary conditions - high
      below_index = floor(gate_high_l[i]);
      upper_index = ceil(gate_high_l[i]);
      integral+=0.5*bsub_trace[below_index];
      integral += (0.5*bsub_trace[below_index] + 
		   0.5*((gate_high_l[i] - below_index)*bsub_trace[upper_index]+
			(upper_index - gate_high_l[i])*bsub_trace[below_index]))*(gate_high_l[i] - below_index);
      
      ql[i] = fabs(integral);

    }
  }

}

void Trace::Make_Pulse_Amplitude() {

  if((int)led_times.size()==0) return; //if no pulses, quit method
  else if(!CS_fit_done) Do_CS_Fit(false);

  return;

  /*
  if((int)led_times.size()==0) return; //if no pulses, quit method
  vector<double> xx; //sample number
  vector<double> yy; //baseline subtracted amplitude
  int index = 0;
  double maxval = 0;
  //loop over pulses
  for(int i = 0; i < (int)led_times.size(); i++) {
    xx.clear();
    yy.clear();
    maxval = 0;
    index = 0;
    if(saturated[i] == 0) {
      //loop over samples within short gate of the trig time
      for(int j = (int)led_times[i]-TIME_TO_WALK_BACK_Q; j < (int)led_times[i] + TIME_TO_WALK_FORWARD_QS; j++) {
	if(j >= 0 && j < (int)bsub_trace.size()) {
	  xx.push_back((double)j); //set x to be the sample number
	  if(bsub_trace[j] > 0) yy.push_back(-1*(double)bsub_trace[j]);
	  else yy.push_back((double)bsub_trace[j]); //set y to be the bsub trace	  
	  if(fabs(bsub_trace[j]) > maxval && fabs(bsub_trace[j]) >= LED_THRESHOLD) {
	    maxval = fabs(bsub_trace[j]);
	    index = j;
	  }
	  else if(fabs(bsub_trace[j]) < maxval && 
		  (fabs(bsub_trace[j]) > LED_THRESHOLD || (j > 0 && fabs(bsub_trace[j-1]) >= LED_THRESHOLD))) {
	    maxval = 1e6;
	  }
	}
      }
      double guess_time = index;
      //end of trace condition
      if(fabs(guess_time - xx.back()) < .001 && fabs(xx.back() - (double)raw_trace.size() + 1) < .001) {
	xx.push_back((double)raw_trace.size());
	yy.push_back(0);
      }
      while(fabs(yy.back()) > fabs(bsub_trace[guess_time])) {
	xx.pop_back();
	yy.pop_back();
      }
      while(fabs(yy[0]) > fabs(bsub_trace[guess_time])) {
	xx.erase(xx.begin());
	yy.erase(yy.begin());
      }

      if(xx.size() < 3 || 
	 guess_time < xx[0]+.01 || guess_time > xx.back() - .01) {
	if(print_uncommon_errors) cout << "Error in finding the phmax - something wrong with endpoints for event " << event_number << endl;
	phmax.push_back(-1);	
      }
      else if (xx[0] == guess_time) {
	phmax.push_back(yy[0]);
      }
      else {
	
	ROOT::Math::Interpolator inter(xx,yy, ROOT::Math::Interpolation::kCSPLINE); //ROOT interpolator, use cubic spline
	//cout << "inter evals: " << endl;
	//cout << inter.Eval(xx[0]) << " " << inter.Eval(guess_time) << " " << inter.Eval(xx.back()) << endl;
	//cout << "min range: " << endl;
	//cout << guess_time << " " << xx[0]+.1 << " " << xx.back() - .1 << endl;
	
	
	ROOT::Math::Functor1D func_inter(&inter,&ROOT::Math::Interpolator::Eval);
	//cout << func_inter(ceil(led_times[i])) << " " << func_inter(floor(led_times[i]+interp_len-1)) << " " << func_inter(led_times[i]+3) << endl;
      // get the minimum value
	ROOT::Math::GSLMinimizer1D minBrent;
	if(fabs(func_inter(guess_time)) < fabs(func_inter(xx[0]+.01)) ||
	   fabs(func_inter(guess_time)) < fabs(func_inter(xx.back()-.01))) {
	  if(print_uncommon_errors) cout << "Error in finding the phmax - something wrong with endpoints for event " << event_number << endl;
	  phmax.push_back(-1);
	}
	else {
	  minBrent.SetFunction(func_inter,guess_time,xx[0]+.01,xx.back()-.01);
	  bool result = minBrent.Minimize(1000,0.001,0.001);
	  if(!result) {
	    cout << "Minimizer failed for pulse " << event_number << endl;
	  }
	  phmax.push_back(fabs(minBrent.FValMinimum()));
	}
      }
    }
    else {
      //cout << "saturated: " << event_number << endl;
      phmax.push_back(fabs(baseline));
    }
   // std::cout << "Found minimum: x = " << minBrent.XMinimum() 
   //           << "  f(x) = " << minBrent.FValMinimum() << std::endl;
  }
  */
}

void Trace::Reset_Values() {

  //clear all vectors
  num_pulses = 0;
  raw_trace.clear();
  deriv.clear();
  led_times.clear();
  trig_times.clear();
  baseline = 0;
  bsub_trace.clear();
  overlap.clear();
  in_window.clear();
  qs.clear();
  ql.clear();
  phmax.clear();
  
  for(int i = 0; i < (int)inter_keeper.size(); i++) delete inter_keeper[i];
  inter_keeper.clear();
  inter_bounds.clear();
  CS_fit_done = false;

}

void Trace::Process() {

  //process a pulse by calling the above methods in this order
  good_trace = true;

  //if using DERIV_THRESHOLD, do this order
  if(threshold_type == 1) {
    Make_Deriv();
    Make_Num_Pulses_Deriv_Threshold();
    if(num_pulses == 0) return;
    Make_Trig_Times();
    if(!Make_Baseline_Sub_Deriv_Threshold()) {
      good_trace = false;
      return;
    }
  }
  else if(threshold_type == 0) {  //else, do this order
    if(!Make_Baseline_Sub_LED_Rough()) Make_Baseline_Sub_LED_Thorough();
    Make_Num_Pulses_LED();
    Make_Deriv();
    if(num_pulses == 0) return;
    Make_Trig_Times();    
  }  
  else {
    cout << "Bad value of threshold_type " << threshold_type << endl;
    return;
  }
  Make_Pulse_Amplitude();
  Improve_Num_Pulses();
  Make_Charges();
}
