// JMM, Jan 2015
// This class processes an entire trace file.  It is configured to work with the
// default build of wavedump from CAEN. It uses the Trace class to handle
// processing of the traces themselves.  The WaveFile class mainly deals with
// reading and parsing the output of wavedump.

#ifndef _WaveFile_incl
#define _WaveFile_incl

#include <math.h>
#include <openssl/md5.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "Trace.h"

using namespace std;

class WaveFile
{
 public:
  // default constructors
  WaveFile(string, bool print2 = true);
  WaveFile(bool print2 = true);
  ~WaveFile();

  // get/set methods
  inline string Get_Filename() { return filename; }
  inline void Set_Filename(string filename2) { filename = filename2; }
  inline Trace *Get_A_Trace_Ptr() { return a_trace_ptr; }
  inline int Get_Record_Length() { return record_length; }
  inline void Set_Record_Length(int record_length2)
  {
    record_length = record_length2;
  }
  inline int Get_Cur_Trace() { return cur_trace; }
  inline void Set_Cur_Trace(int trace2) { Absolute_Skip_To_Trace(trace2); }
  inline void Set_Cur_Trace_Offset(int cur_trace_offset2)
  {
    cur_trace_offset = cur_trace_offset2;
  }
  inline bool Get_Is_Binary() { return is_binary; }
  inline int Get_CAEN_Evno() { return caen_evno; }
  inline int Get_Num_Traces() { return num_traces; }
  inline void Set_Param_Map(map<string, int> param_map2)
  {
    param_map = param_map2;
  }
  inline map<string, int> Get_Param_Map() { return param_map; }
  inline void Set_Det_No(uint32_t det_id_no2) { det_id_no = det_id_no2; }

  // actual methods
  void Initialize();  // called by the constructors
  bool Is_Binary(const void *,
                 size_t);  // checks if the waveforms are binary or ASCII
  bool Open_File();        // opens the wave file
  void
  Check_For_Header();  // attempts to see if there is a header for each event
  void Find_Record_Length();     // attempts to get the record length for each
                                 // event (if there is a header)
  void Find_Number_Of_Traces();  // calculates the total number of traces in the
                                 // file using the record lenght
  bool Fill_Binary_Buffer();
  void Clear_Binary_Buffer();
  bool Initialize_File();  // calls the above methods in the correct order to
                           // initialize a file
  int Get_Next_Trace(
      Trace *cur_trace_ptr =
          NULL);  // reads the next trace and stores it into a new Trace object
  void Delete_Trace();               // deletes that Trace object
  void Process_Trace();              // process the Trace object
  void Absolute_Skip_To_Trace(int);  // skips to a particular trace in the file
                                     // (absolute skip, not relative skip)

  // define error messages for returning from Get_Next_Trace()
  static const int Get_Next_Trace_Success = 0;
  static const int Get_Next_Trace_EOF = -1;
  static const int Get_Next_Trace_Checksum_Failure = -2;

 private:
  FILE *infile;          // the file that we're reading the traces from
  int record_length;     // the record length from the header
  int num_traces;        // the total number of traces in the file
  int cur_trace;         // the index of the current trace, as kept by WaveFile
  int cur_trace_offset;  // the number of total traces in previous WaveFiles, as
                         // given from WaveFileHandler
  int caen_evno;         // the event number according to the header
  bool use_headers;      // whether or not the traces have headers
  bool is_binary;        // whether or not the file is in binary

  // define header information for default wavedump binary headers
  static const int binary_header_entries = 6;  // 6 uint_32's
  static const int binary_time_offset_pos =
      6;  // where the time offset is located in the header
  static const int binary_caen_evno_pos =
      5;  // where the event number is recorded in the header
  static const int binary_blk_transfer_id_pos =
      2;  // where the event number is recorded in the header

  // define header information for default wavedump ascii headers
  static const int ascii_header_lines = 7;  // 7 lines of header in ascii
  static const int ascii_time_offset_line =
      6;  // where the time offset is located in the header
  static const int ascii_caen_evno_line =
      4;  // where the event number is located in the header

  string filename;     // the name of the file
  Trace *a_trace_ptr;  // a Trace object

  // map of the params, indexed by a string of their variable name in Trace.h
  map<string, int> param_map;

  bool print;

  // for using a RAM buffer with binary files to improve speed
  long buffer_size;  // in bytes
  unsigned char *binary_buffer;
  long buffer_size_max;  // in bytes
  long cur_buffer_pt;
  long event_buffer_start_pt;

  unsigned char md5_from_file[MD5_DIGEST_LENGTH];

  uint32_t det_id_no;
};
#endif
