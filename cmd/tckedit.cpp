/*******************************************************************************
    Copyright (C) 2014 Brain Research Institute, Melbourne, Australia
    
    Permission is hereby granted under the Patent Licence Agreement between
    the BRI and Siemens AG from July 3rd, 2012, to Siemens AG obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to possess, use, develop, manufacture,
    import, offer for sale, market, sell, lease or otherwise distribute
    Products, and to permit persons to whom the Software is furnished to do
    so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/


#include <string>
#include <vector>

#include "command.h"
#include "exception.h"
#include "mrtrix.h"

#include "thread/exec.h"
#include "thread/queue.h"

#include "dwi/tractography/file.h"
#include "dwi/tractography/properties.h"
#include "dwi/tractography/roi.h"

#include "dwi/tractography/editing/editing.h"
#include "dwi/tractography/editing/loader.h"
#include "dwi/tractography/editing/receiver.h"
#include "dwi/tractography/editing/worker.h"



using namespace MR;
using namespace App;
using namespace MR::DWI;
using namespace MR::DWI::Tractography;
using namespace MR::DWI::Tractography::Editing;




void usage ()
{

  AUTHOR = "Robert E. Smith (r.smith@brain.org.au)";

  DESCRIPTION
  + "perform various editing operations on track files.";

  ARGUMENTS
  + Argument ("tracks_in",  "the input track file(s)").type_file().allow_multiple()
  + Argument ("tracks_out", "the output track file").type_file();

  OPTIONS
  + ROIOption
  + LengthOption
  + ResampleOption
  + TruncateOption;
};






void update_output_step_size (Tractography::Properties& properties, const int upsample_ratio, const int downsample_ratio)
{
  if (upsample_ratio == 1 && downsample_ratio == 1)
    return;
  float step_size = 0.0;
  if (properties.find ("output_step_size") == properties.end())
    step_size = (properties.find ("step_size") == properties.end() ? 0.0 : to<float>(properties["step_size"]));
  else
    step_size = to<float>(properties["output_step_size"]);
  properties["output_step_size"] = step_size * float(downsample_ratio) / float(upsample_ratio);
}





void run ()
{

  const size_t num_inputs = argument.size() - 1;
  const std::string output_path = argument[num_inputs];

  // Make sure configuration is sensible
  if (get_options("tck_weights_in").size() && num_inputs > 1)
    throw Exception ("Cannot use per-streamline weighting with multiple input files");
  // TODO Anything else?

  // Get the consensus streamline properties from among the multiple input files
  Tractography::Properties properties;
  size_t count = 0;
  std::vector<std::string> input_file_list;

  for (size_t file_index = 0; file_index != num_inputs; ++file_index) {

    input_file_list.push_back (argument[file_index]);

    Properties p;
    Tractography::Reader<float> reader (argument[file_index], p);

    for (std::vector<std::string>::const_iterator i = p.comments.begin(); i != p.comments.end(); ++i) {
      bool present = false;
      for (std::vector<std::string>::const_iterator j = properties.comments.begin(); !present && j != properties.comments.end(); ++j)
        present = (*i == *j);
      if (!present)
        properties.comments.push_back (*i);
    }

    // ROI paths are ignored - otherwise tckedit will try to find the ROIs used
    //   during streamlines generation!

    size_t this_count = 0, this_total_count = 0;

    for (Properties::const_iterator i = p.begin(); i != p.end(); ++i) {
      if (i->first == "count") {
        this_count = to<float>(i->second);
      } else if (i->first == "total_count") {
        this_total_count += to<float>(i->second);
      } else {
        Properties::iterator existing = properties.find (i->first);
        if (existing == properties.end())
          properties.insert (*i);
        else if (i->second != existing->second)
          existing->second = "variable";
      }
    }

    count += this_count;

  }

  DEBUG ("estimated number of input tracks: " + str(count));

  load_rois (properties);

  // Some properties from tracking may be overwritten by this editing process
  Editing::load_properties (properties);

  // Parameters that the worker threads need to be aware of, but do not appear in Properties
  Options opt = get_options ("upsample");
  const int upsample   = opt.size() ? int(opt[0][0]) : 1;
  opt = get_options ("downsample");
  const int downsample = opt.size() ? int(opt[0][0]) : 1;

  // Parameters that the output thread needs to be aware of
  opt = get_options("number");
  const size_t number = opt.size() ? size_t(opt[0][0]) : 0;
  opt = get_options ("skip");
  const size_t skip   = opt.size() ? size_t(opt[0][0]) : 0;

  Loader loader (input_file_list);
  Worker worker (properties, upsample, downsample);
  // This needs to be run AFTER creation of the Worker class
  // (worker needs to be able to set max & min number of points based on step size in input file,
  //  receiver needs "output_step_size" field to have been updated before file creation)
  update_output_step_size (properties, upsample, downsample);
  Receiver receiver (output_path, properties, count, number, skip);

  Thread::run_queue (
      loader, 
      Thread::batch (Tractography::Streamline<>()),
      Thread::multi (worker), 
      Thread::batch (Tractography::Streamline<>()),
      receiver);

}