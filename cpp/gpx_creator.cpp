// Include the header-only libcluon file.
#include "cluon-complete.hpp"

// Include our self-contained message definitions.
#include "example.hpp"
#include "opendlv-standard-message-set.hpp"

// We will use iostream to print the received message.
#include <iostream>

#include <string>
#include <fstream>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

// Function prototypes 
void writeMessageToFile(std::ofstream &fd, cluon::data::Envelope);
int fileType(std::string);
int genGPXfile(std::string);
bool updateGPXString(std::string&, cluon::data::Envelope, double, bool);

int main(int argc, char **argv) {

  if (argc != 2) {
    std::cerr << "Use: " << argv[0] << " FULLPATHFILENAME" << std::endl;
    exit(-1);
  }

  //time_interval = 5.0;
  //start_time_relative = 0.0;

  std::string filename_in = argv[1];
  //std::string filename_in = "testdir";
  int type = fileType(filename_in);

  // If directory, cut all eligible files into the given interval
  if (type == 0){
    
  }
  // If filename_in is a file then cut it
  else if (type == 1){
    genGPXfile(filename_in);
  }
  else if (type == 2){
    std::cout << "interpreted filename as invalid rec file" << std::endl;
  }
  return 0;
}


// Write envelope data to file
void writeMessageToFile(std::ofstream &fd, cluon::data::Envelope env){
  std::string data = cluon::serializeEnvelope(std::move(env));
  fd.write(&data[0], data.length());
}

// Determines if given filename is a .rec-file or a directory
// Also checks if it is the output from a previous split
// Returns 0 if filename refers to a directory
// 1 if it is a valid .rec-file
// 2 if it is an invalid .rec-file
// NOTE: This function is allergic to filenames that contains the string
// '_rec'
int fileType(std::string filename){
  char *current_token = strtok(&filename[0], "_.");

  if (current_token != NULL){
    while(current_token != NULL){

      if (strcmp(current_token, "out") == 0){
        return 2;
      }
      else if (strcmp(current_token, "rec") == 0){
        return 1;
      }
      current_token = strtok(NULL, "_.");
    }
  }
  return 0;
}

bool updateGPXString(std::string& gpxString, cluon::data::Envelope env, double timestamp, std::string file, bool firstGPS) {
  
  // Unpack the content...
  auto msg = cluon::extractMessage<opendlv::proxy::GeodeticWgs84Reading>(std::move(env));

  // ...and access the data fields.
  // Create GPX file.
  // Change into timestamp into ISO8601
  time_t curr_time = (time_t)timestamp;
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&curr_time));

  // Fetch geolocations
  std::string lat = std::to_string(msg.latitude());
  std::string lon = std::to_string(msg.longitude());
  //std::string ele = std::to_string(msg.altitude());
  std::cout << std::to_string(msg.longitude()) << std::endl;
  // Build first input into GPX file.
  // example from:  
  // https://wiki.openstreetmap.org/wiki/GPX
  if (firstGPS){
    gpxString += "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n";
    gpxString += "<gpx version=\"1.0\"> \n";
    gpxString += "  <name>" + file + "</name>\n";
    gpxString += "  <trk><name>" + file + "</name><number>1</number><trkseg>\n";
    
    firstGPS = false;
  }
  
  gpxString += "    <trkpt lat=\"" + lat + "\" lon=\"" + lon + "\"><ele>0</ele><time>" + buf + "</time></trkpt>\n";

  return firstGPS;
}
// Cut file 'filename_in' and put output in 'filename_in_out'
int genGPXfile(std::string filename_in){
  std::string tmpstr;
  tmpstr += filename_in;

  
  //std::cout << filename_in << std::endl;

  std::size_t found = filename_in.find_last_of("/\\");
  std::string path =  filename_in.substr(0,found);
  std::string file = filename_in.substr(found+1);
  std::string file_name = strtok(&file[0], ".");
  std::string ending = ".gpx";
  std::string gps_out = file_name + ending;
  std::string gpsname_out = path + "/" + gps_out;
  std::ofstream f_out2(gpsname_out, std::ios::out | std::ios::binary);

  if (!f_out2){
    std::cout << "Cannot open output file! " << std::endl;
    exit(-1);
  }

  bool firstEnvelope = true;
  bool firstGPS = true;
  double timestamp, start_time, stop_time;

  // We will use cluon::Player to parse and process a .rec file.
  constexpr bool AUTO_REWIND{false};
  constexpr bool THREADING{false};
  //cluon::Player player(std::string(argv[1]), AUTO_REWIND, THREADING);
  cluon::Player player(filename_in, AUTO_REWIND, THREADING);

  std::string gpxString = "";
  // Now, we simply loop over the entries.
  while (player.hasMoreData()) {
    auto entry = player.getNextEnvelopeToBeReplayed();
    // The .first field indicates whether we have a valid entry.
    if (entry.first) {
      // Get the Envelope with the meta-information.
      cluon::data::Envelope env = entry.second;

      // Get timestamp
      timestamp = env.sampleTimeStamp().seconds() + 1e-6*env.sampleTimeStamp().microseconds();

        // Check whether it is of type GeodeticWgs84Reading.
      if (env.dataType() == opendlv::proxy::GeodeticWgs84Reading::ID()) {
        firstGPS = updateGPXString(gpxString, env, timestamp, file_name, firstGPS);
      }

    }
  }
    
  gpxString += "  </trkseg></trk>\n";
  gpxString += "</gpx>";
  
  f_out2.write(&gpxString[0], gpxString.length());
  f_out2.close();
  return 0;
}