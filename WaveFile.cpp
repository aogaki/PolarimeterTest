// JMM, Jan 2015

#include "WaveFile.h"

WaveFile::WaveFile(string filename2, bool print2)
{
  Initialize();
  Set_Filename(filename2);
  print = print2;
}

WaveFile::WaveFile(bool print2)
{
  Initialize();
  print = print2;
}

WaveFile::~WaveFile()
{
  if (is_binary && binary_buffer != NULL) free(binary_buffer);
  if (infile != NULL) fclose(infile);
}

void WaveFile::Initialize()
{
  // called by both constructors

  // initialize all the variables
  infile = NULL;
  record_length = -1;
  num_traces = -1;
  cur_trace = 0;
  cur_trace_offset = 0;
  use_headers = false;
  is_binary = false;
  filename = "null";
  a_trace_ptr = NULL;
  caen_evno = -1;
  binary_buffer = NULL;

  // buffer_size_max = 50000000; //worst
  // buffer_size_max = 50000; //better
  buffer_size_max = 5000;  // same as 50000
}

bool WaveFile::Is_Binary(const void *data, size_t len)
{
  // used by Open_File to check if it is binary or ASCII
  return true;
  return memchr(data, '\0', len) != NULL;
}

bool WaveFile::Open_File()
{
  // opens a waveform file
  // checks to see if it is binary or ASCII
  // returns true on success, false on failure

  // check if file is binary or ascii.  try binary first
  infile = fopen(filename.c_str(), "rb");
  if (infile != NULL) {
    cout << "File " << filename << " opened successfully" << endl;

    // read the first 4096*2 bytes, see if they look like binary or ascii
    short test_binary[4096];
    if (fread(test_binary, 2, 4096, infile) != 4096) {
      cout << "Error in performing the test to see if the file is binary"
           << endl;
      return false;
    }
    if (Is_Binary(test_binary, 4096 * 100)) {
      // if first 4096*2 bytes are consistent with binary, then call the file
      // binary
      is_binary = true;
      if (print) cout << "File opened as binary" << endl;
      rewind(infile);  // go back to the beginning of the file
      // adjust param map for record length
      param_map["RECORD_LENGTH"] = param_map["RECORD_LENGTH"] * 2;
      return true;
    } else {
      // if not, close the file and reopen as ASCII
      is_binary = false;
      fclose(infile);
      infile = fopen(filename.c_str(), "r");
      if (print) cout << "File opened as ASCII" << endl;
      return true;
    }
  } else {
    // if the file doesn't exist
    cout << "File " << filename << " cannot be opened by WaveFile.cpp" << endl;
    return false;
  }
}

void WaveFile::Check_For_Header()
{
  // try to find out if each event has a header

  if (is_binary) {
    // if it's binary, read the header in.  if the header has any large values
    // in it, then it's not a header.  else it is a header
    uint32_t header[binary_header_entries];
    if (fread(header, 4, binary_header_entries, infile) !=
        binary_header_entries) {
      cout << "Error attempting to read the header in Check_For_Header() "
           << endl;
    }
    rewind(infile);  // go back to the beginning of the file after the read
    // testing
    // for(int i = 0; i < binary_header_entries; i++) printf("%i ",header[i]);
    // printf("\n");

    // if the 4th item is large, then it's probably not a header
    if (header[3] > 10000) {
      use_headers = false;
      cout << "Each event does not have a header" << endl;
    } else {
      use_headers = true;
      cout << "Each event does have a header" << endl;
    }
  } else {
    // if it's an ascii file, read in the first character.  if it's a number,
    // then there aren't headers.  if it's a letter, there are headers

    char header;
    if (fread(&header, 1, 1, infile) != 1) {
      cout << "Error attempting to read the header in Check_For_Header() "
           << endl;
    }
    rewind(infile);  // go back to the beginning of the file
    if (isalpha(header)) {
      use_headers = true;
      if (print) cout << "Each event does have a header" << endl;
    } else {
      use_headers = false;
      if (print) cout << "Each event does not have a header" << endl;
    }
  }
}

void WaveFile::Find_Record_Length()
{
  // find the record length from the header

  if (is_binary) {
    if (param_map["APPEND_MD5"] == 1) {
      param_map["RECORD_LENGTH"] += 16;
    }
  }

  if (use_headers) {
    if (is_binary) {
      param_map["RECORD_LENGTH"] += 24;
      // if it's binary, the first entry (2 bytes) in the header should
      // correspond to the record length
      uint32_t header0[1];
      if (fread(header0, 4, 1, infile) != 1) {
        cout << "Error attempting to read the binary header in "
                "Find_Record_Length() "
             << endl;
        raise(SIGTERM);
      }
      rewind(infile);  // go back to the beginning of the file
      record_length = header0[0];
      if (print)
        cout << "Assuming # of bytes per event is first two bytes in the "
                "header: "
             << record_length << " bytes per event" << endl;
      // check if it matches the input file
      if (record_length != param_map["RECORD_LENGTH"])
        cout << "NOTE: This does not match the value found in the input "
                "parameter file "
             << filename << ".info of " << param_map["RECORD_LENGTH"] << endl;
    } else {
      // if it's ASCII, the first line should tell the record length, and be
      // formatted by Record Length: %i
      int num_read = 0;
      num_read = fscanf(infile, "Record Length: %i", &record_length);
      if (num_read != 1) {
        // error - header not formatted as expected
        cout << "The first line of the header should read 'Record Length: XX', "
                "where XX is an integer"
             << endl;
        raise(SIGTERM);
      }
      rewind(infile);  // go back to the beginning of the file
    }
    if (print) {
      if (!is_binary) {
        cout << "From reading the header, the record length is: "
             << record_length << endl;
      } else {
        if (param_map["APPEND_MD5"] == 1) {
          cout << "From reading the header, the record length is: "
               << (record_length - binary_header_entries * 4 - 16) / 2
               << " samples" << endl;
        } else {
          cout << "From reading the header, the record length is: "
               << (record_length - binary_header_entries * 4) / 2 << " samples"
               << endl;
        }
      }
    }
    // check to see if it matches the input file
    if (record_length != param_map["RECORD_LENGTH"])
      cout << "NOTE: This does not match the value found in the input "
              "parameter file "
           << filename << ".info of " << param_map["RECORD_LENGTH"] << endl;
  } else {
    // not using headers, so assume the record length was correctly indicated in
    // the input file
    record_length = param_map["RECORD_LENGTH"];
    if (print) {
      if (!is_binary) {
        cout << "Could not get the record length from the header, assuming it "
                "is "
             << record_length << endl;
      } else {
        cout << "Could not get the record length from the header, assuming it "
                "is "
             << (record_length - binary_header_entries * 4) / 2
             << " samples, or " << record_length << " bytes per trace" << endl;
      }
    }
  }
}

void WaveFile::Find_Number_Of_Traces()
{
  // get the total number of traces in the file using the file size and the
  // record length

  if (is_binary) {
    // need to know how many bytes are in the file
    fseek(infile, 0, SEEK_END);
    long lSize = ftell(infile);  // get number of bytes to current position in
                                 // the file (now set to SEEK_END - EOF)
    rewind(infile);              // go back to the beginning of the file
    num_traces =
        (lSize) /
        (record_length);  // lSize is in bytes, 2 bytes per word so divide by 2.
                          // record length includes header if there is a header
    cout << "Number of traces in this file: " << num_traces << endl;
  } else {
    // ascii - need to know how many lines are in the file
    // get one character at a time, look for \n or EOF
    unsigned int number_of_lines = 0;
    int ch;
    while (EOF != (ch = getc(infile))) {
      if ('\n' == ch) {
        number_of_lines++;
      }
    }
    rewind(infile);  // go back to the beginning of the file
    // record length does not include ASCII header
    if (use_headers)
      num_traces = number_of_lines / (record_length + ascii_header_lines);
    else
      num_traces = number_of_lines / (record_length);
    cout << "Number of traces in this file: " << num_traces << endl;
  }
}

bool WaveFile::Fill_Binary_Buffer()
{
  // should depend on position within file!! if near end, only read last bit of
  // file
  unsigned long total_file_size = num_traces * record_length * 2.0;  // in bytes
  size_t result;
  if (total_file_size < buffer_size_max) {
    buffer_size = total_file_size;
  } else {
    buffer_size = ((int)(buffer_size_max / record_length)) * (record_length) +
                  record_length;
  }
  if (binary_buffer == NULL)
    binary_buffer = (unsigned char *)malloc(sizeof(char) * buffer_size);

  if (binary_buffer == NULL) {
    cout << "Memory error - can't make binary buffer" << endl;
    return false;
  }

  // copy the file into the buffer:
  result = fread(binary_buffer, 1, buffer_size, infile);
  if ((int)result != buffer_size &&
      !(feof(infile) && (int)result < buffer_size)) {
    cout << "Reading error in creating binary buffer" << endl;
    return false;
  }
  cur_buffer_pt = 0;
  return true;
}

void WaveFile::Clear_Binary_Buffer()
{
  if (is_binary && binary_buffer != NULL) free(binary_buffer);
}

bool WaveFile::Initialize_File()
{
  // opens a file formatted according to default build of wavedump

  bool works;

  works = Open_File();
  if (!works) return false;  // can't open file, exit
  Check_For_Header();
  Find_Record_Length();
  Find_Number_Of_Traces();

  return true;
}

int WaveFile::Get_Next_Trace(Trace *cur_trace_ptr)
{
  // get the next trace in the file, and send it to a new Trace object

  vector<short> trace;  // the trace will be stored here
  short aword;
  trace.clear();
  uint32_t trig_time_offset;  // the time offset from the header (if it exists)
                              // will be stored here
  uint32_t blk_transfer_id;   // the block transfer id from the header (if it
                              // exists) will be stored here

  cur_trace++;  // keep track of the current trace number
  // EOF condition
  if (cur_trace > num_traces) {
    //    cout << filename << " Reached end" << endl;
    trace.push_back(-1);
    return WaveFile::Get_Next_Trace_EOF;
  }

  if (!use_headers) {
    trig_time_offset = 0;  // if no headers, set this to zero
    blk_transfer_id = 0;
  }

  if (is_binary) {
    // check if we need to refill the buffer
    if (cur_buffer_pt >= buffer_size || binary_buffer == NULL)
      Fill_Binary_Buffer();
    event_buffer_start_pt = cur_buffer_pt;

    // do binary reads
    if (use_headers) {
      // read the caen_evno and trig time offset
      uint32_t new_caen_evno;

      long caen_evno_pos = cur_buffer_pt + (binary_caen_evno_pos - 1) * 4;
      // try to get the caen event number
      new_caen_evno = binary_buffer[caen_evno_pos + 0] |
                      (binary_buffer[caen_evno_pos + 1] << 8) |
                      (binary_buffer[caen_evno_pos + 2] << 16) |
                      (binary_buffer[caen_evno_pos + 3] << 24);

      //      cout << "evno: " << new_caen_evno << endl;

      // initialization condition for caen event number
      if (caen_evno == -1)
        caen_evno = new_caen_evno;
      else if (caen_evno == (int)new_caen_evno - 1) {
        caen_evno = new_caen_evno;  // if it has incremented by 1 from the
                                    // previous event, it is OK
      } else if (caen_evno == pow(2, 24) - 1 && (int)new_caen_evno == 0) {
        caen_evno = 0;
      } else {
        // it has skipped or done something goofy, flag it
        cout << "CAEN event number seems off.  CAEN evno should be: "
             << caen_evno << " but is " << new_caen_evno
             << " actual event number: " << cur_trace << endl;
        cout << "Ignore this warning if you stopped and started different runs"
             << endl;
        caen_evno = new_caen_evno;
        if (cur_buffer_pt >= buffer_size)
          cout << "Buffer problem? cur_buffer_pt: " << cur_buffer_pt << " "
               << buffer_size << endl;
        // raise(SIGTERM);
      }

      long trig_time_offset_pos =
          cur_buffer_pt + (binary_time_offset_pos - 1) * 4;

      // try to read the time offset from the header
      uint32_t new_trig_time_offset;
      new_trig_time_offset = binary_buffer[trig_time_offset_pos] |
                             (binary_buffer[trig_time_offset_pos + 1] << 8) |
                             (binary_buffer[trig_time_offset_pos + 2] << 16) |
                             (binary_buffer[trig_time_offset_pos + 3] << 24);

      trig_time_offset = new_trig_time_offset;
      //      cout << "time offset: " << trig_time_offset << endl;

      // try to read the blk transfer id from the header
      long blk_transfer_id_pos =
          cur_buffer_pt + (binary_blk_transfer_id_pos - 1) * 4;
      uint32_t new_blk_transfer_id;
      new_blk_transfer_id = binary_buffer[blk_transfer_id_pos] |
                            (binary_buffer[blk_transfer_id_pos + 1] << 8) |
                            (binary_buffer[blk_transfer_id_pos + 2] << 16) |
                            (binary_buffer[blk_transfer_id_pos + 3] << 24);

      blk_transfer_id = new_blk_transfer_id;

      // skip header
      cur_buffer_pt += binary_header_entries * 4;

      // read the file and push back each short to the trace vector.  note that
      // the header is included in the record length, so you have to subtract it
      // out or the for loop will go too long
      for (int i = 0; i < (record_length - 4 * binary_header_entries) / 2;
           i++) {
        aword = binary_buffer[cur_buffer_pt] |
                (binary_buffer[cur_buffer_pt + 1] << 8);
        //	aword = (binary_buffer[cur_buffer_pt] << 8) |
        // binary_buffer[cur_buffer_pt+1];
        trace.push_back(aword);
        // cout << aword << " " << cur_buffer_pt << endl;
        cur_buffer_pt += 2;
      }
    } else {
      // no headers to worry about, just read the file and push back each short
      // to the trace vector
      for (int i = 0; i < record_length / 2; i++) {
        aword = binary_buffer[cur_buffer_pt] |
                (binary_buffer[cur_buffer_pt + 1] << 8);
        trace.push_back(aword);
        cur_buffer_pt += 2;
      }
    }
    // if we're appending MD5, pop the last 8 words off of the trace vector,
    // store as checksum
    if (param_map["APPEND_MD5"] == 1) {
      short current_short;
      for (int i = 0; i < 8; i++) {
        current_short = trace.back();
        md5_from_file[14 - 2 * i] = (unsigned char)(current_short);
        md5_from_file[14 - 2 * i + 1] = (unsigned char)(current_short >> 8);
        trace.pop_back();
      }
      /*
      for(int i = 0; i < 16; i++) {
        printf("%02hx",md5_from_file[i]);
      }
      printf("\n");
      */

      // compare to MD5 checksum using event_buffer_start_pt
      unsigned char md5_calculated[MD5_DIGEST_LENGTH];
      bool checksum_failure = false;
      MD5(&binary_buffer[event_buffer_start_pt], record_length - 16,
          md5_calculated);
      for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        if (md5_calculated[i] - md5_from_file[i] != 0) {
          checksum_failure = true;
          break;
        }
      }
      if (checksum_failure) {
        cout << "Checksum failed for event: " << cur_trace << endl;
        return WaveFile::Get_Next_Trace_Checksum_Failure;
      }
    }
  } else {
    // get ready to read an ASCII file

    short a_value;  // a place to store numbers from a read
    int line_read;  // if we were able to read the line
    int new_caen_evno;
    char aline[200];  // place to store a particular line
    fpos_t pos;
    if (use_headers) {
      // check caen_evno and get trig time offset
      fgetpos(infile, &pos);  // get the current position

      // get to the correct line for caen event number
      for (int i = 0; i < ascii_caen_evno_line - 1; i++) {
        if (strcmp(fgets(aline, 200, infile), aline) != 0)
          cout << "Ascii read error" << endl;  // read each line and ignore it
        if (!isalpha(aline[0]))
          i--;  // if there's any weird \n errors, skip that line
      }
      // try to read caen event number
      line_read = fscanf(infile, "Event Number: %i", &new_caen_evno);
      if (line_read != 1) {  // if it didn't read correctly
        cout << "Error reading ascii line!" << endl;
        if (strcmp(fgets(aline, 200, infile), aline) != 0)
          cout << "Ascii read error" << endl;
        puts(aline);
      } else {
        // it did read correctly
        if (caen_evno == -1)
          caen_evno = new_caen_evno;  // initialization condition
        else if (caen_evno == new_caen_evno - 1)
          caen_evno = new_caen_evno;  // incremented by 1 - OK
        else {
          // skipped a number somewhere, flag it
          cout << "CAEN event number seems off.  CAEN evno should be: "
               << caen_evno << " but is " << new_caen_evno << endl;
          raise(SIGTERM);
        }
      }
      fsetpos(infile, &pos);  // go back to the start of the header

      // now, read the trig time offset
      // get to the correct line
      for (int i = 0; i < ascii_time_offset_line - 1; i++) {
        if (strcmp(fgets(aline, 200, infile), aline) != 0)
          cout << "Ascii read error" << endl;
        if (!isalpha(aline[0])) i--;
      }
      // try to read it
      line_read = fscanf(infile, "Trigger Time Stamp: %ui", &trig_time_offset);
      if (line_read != 1) {  // read failed
        cout << "Error reading ascii line!" << endl;
        if (strcmp(fgets(aline, 200, infile), aline) != 0)
          cout << "Ascii read error" << endl;
        puts(aline);
      }
      // now skip the rest of the header
      for (int i = 0; i < ascii_header_lines - ascii_time_offset_line; i++) {
        if (strcmp(fgets(aline, 200, infile), aline) != 0)
          cout << "Ascii read error" << endl;
        if (!isalpha(aline[0])) i--;
      }
      // read in the trace line by line
      for (int i = 0; i < record_length; i++) {
        line_read =
            fscanf(infile, "%hi",
                   &a_value);  // read a line and store the value as a_value
        if (line_read != 1) {  // error - record length is wrong, something is
                               // formatted incorrectly
          cout << "Error reading ascii line!" << endl;
          if (strcmp(fgets(aline, 200, infile), aline) != 0)
            cout << "Ascii read error" << endl;
          puts(aline);
          if (strcmp(fgets(aline, 200, infile), aline) != 0)
            cout << "Ascii read error" << endl;
          puts(aline);
          if (strcmp(fgets(aline, 200, infile), aline) != 0)
            cout << "Ascii read error" << endl;
          puts(aline);
          cout << "Record length error?" << endl;
          raise(SIGTERM);
        }
        trace.push_back(a_value);  // add a_value to the trace
      }
    } else {
      // ASCII, no headers, just read in the trace
      for (int i = 0; i < record_length; i++) {
        line_read = fscanf(infile, "%hi", &a_value);
        if (line_read != 1)
          cout << "Error reading ascii line!" << endl;  // formatting is wrong
        trace.push_back(a_value);
      }
    }
  }
  // EOF condition
  // if(feof(infile)) {
  //   cout << "End of file" << endl;
  //   return WaveFile::Get_Next_Trace_EOF;
  // }

  if (cur_trace_ptr == NULL) {
    a_trace_ptr = new Trace(trace, param_map, trig_time_offset,
                            cur_trace);  // make the new trace object
    a_trace_ptr->Set_Blk_Transfer_ID(blk_transfer_id);
    a_trace_ptr->Set_Det_No(det_id_no);

  } else {
    // overwrite current trace object to save time
    cur_trace_ptr->Reset_Values();
    cur_trace_ptr->Set_Raw_Trace(trace);
    cur_trace_ptr->Set_Cur_Trace(cur_trace + cur_trace_offset);
    cur_trace_ptr->Set_Trig_Time_Offset(trig_time_offset);
    cur_trace_ptr->Set_Blk_Transfer_ID(blk_transfer_id);
    cur_trace_ptr->Set_Det_No(det_id_no);
    a_trace_ptr = cur_trace_ptr;
  }

  return WaveFile::Get_Next_Trace_Success;  // success
}

void WaveFile::Delete_Trace()
{
  // delete the current Trace object
  if (a_trace_ptr != NULL) delete a_trace_ptr;
}

void WaveFile::Process_Trace()
{
  // tell the current Trace object to process the trace
  a_trace_ptr->Process();
}

void WaveFile::Absolute_Skip_To_Trace(int trace_id)
{
  // skip to a particular trace (trace_id) in the file

  if (caen_evno != -1)
    caen_evno +=
        trace_id - cur_trace;  // make sure to change caen_evno as needed
  cur_trace = trace_id;
  // if the trace_id is past the EOF, exit method
  if (cur_trace > num_traces) {
    cout << "Reached end" << endl;
    return;
  }

  if (is_binary) {
    fseek(infile, record_length * trace_id,
          SEEK_SET);  // skip ahead to the right trace from the beginning of the
                      // file
    Fill_Binary_Buffer();
  } else {
    // ASCII
    char aline[500];
    rewind(infile);  // go back to the beginning of the file
    if (use_headers) {
      // if there's headers, add that to the record length, and skip the right
      // number of lines
      for (int i = 0; i < (ascii_header_lines + record_length) * trace_id;
           i++) {
        if (strcmp(fgets(aline, 500, infile), aline) != 0)
          cout << "Ascii read error in skipping" << endl;
        if (!isalnum(aline[0])) i--;
      }
    } else {
      // no headers - just skip the record_length lines per trace
      for (int i = 0; i < record_length * trace_id; i++) {
        if (strcmp(fgets(aline, 500, infile), aline) != 0)
          cout << "Ascii read error in skipping" << endl;
        if (!isalnum(aline[0])) i--;
      }
    }
  }
  // EOF condition
  if (feof(infile)) {
    cout << "End of file" << endl;
  }
}
