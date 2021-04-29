/// <summery>
/// <para>This is the CervPro cervical proprioception test.</para>
/// <para>The patient sits down 90 cm from the camera and will perform
/// standared CPE test. The camera will track the patient's movement with
/// facial recognition. The difference from the starting to ending positions
/// will then be reported as an angle in degrees.</para>
/// <para>A GUI is included for the physical therapist's ease of use. There
/// are two main windows: one that shows the facial tracking in process, and
/// one that shows the points of the face on that frame (while running the test)
/// and then the final angle difference at the end. the second window has
/// a start/stop button for the test as well as a menu. The menu includes
/// instructions on how to use the machine, and options for modifying/improving
/// the test.</para>
/// </summery>
#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/threads.h>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <iterator>
#include <time.h>

#ifndef NULL
#define NULL 0
#endif

using namespace dlib;
using namespace std;

// based on hat sizes for adults, values in cm
const std::vector<double> circumference_male = {55, 58, 60, 62, 64};
const std::vector<double> circumference_female = {55, 57, 59, 61, 63};
const double middle_circumference = 59;

const long double PI = atan(1) * 4;

const unsigned short FRAMES_PER_VELOCITY_CALC = 15;

const double ANGLE_CORRECTION = 3.50711;

const string FILE_NAME = "output_log.txt";
const string SAVE_PATH = "~/Desktop";

void Set_Circumference(double c);

/// <summery>
/// This class defines the window that will be used to display resutls.
/// </summery>
class my_window : public drawable_window
{

private:
  label output;
  button btn;
  menu_bar mbar;
  string output_text_copy;
  bool log;
  size_t frames;

  /// <summery>
  /// This produces a message box from pressing the Instruction menu option.
  /// </summery>
  void ShowInstructions()
  {
    using namespace message_box_helper;
    string str = "";
    str += "Thank you for chosing CervPro.\n\r";
    str += "The video feed is a live feed from the camera with an overlay\r\n ";
    str += "showing if a face has been detected. This overlay shows that\r\n";
    str += "the program is able to track the face, and by extention\r\n";
    str += "perform the needed calculations.\n\r";
    str += "The text shows what values are being ovserved by the camera\r\n";
    str += " and the results of the performed calculations of the movement\r\n of the head.\n\r";
    str += "The below button is used to start and stop the program. After\r\n";
    str += " pressing the button and causing the program to start, readings\r\n";
    str += " will begin being displayed. When the button is pressed again\r\n";
    str += " the program will make all final calculations and show how far\r\n";
    str += " off the patients head was from where he/she started.";
    box_win *win = new box_win("Instructions", str);
    win->set_size(410, 300);
  }

  void ShowOptions()
  {
    options_window temp;
    temp.wait_until_closed();
  }

  void Toggle_Log()
  {
    log = !log;
  }

  void BtnPress()
  {
    if (btn_pressed)
    { // end test
    }
    else
    { // begin test
      btn.set_name("stop");
    }
    btn_pressed = !btn_pressed;
  }

public:
  bool btn_pressed;
  double circumference;

  /// <sumery>
  /// This creates the data window that is 400 x 500.
  /// It has a button, an output lable, and a menu bar with one item.
  /// </sumery>
  my_window() : output(*this), btn(*this), mbar(*this), log(true), frames(0),
                circumference(middle_circumference)
  {

    //button setup
    btn.set_pos(/*x*/ 175, 410 /*y*/);
    btn.set_name("start");
    btn_pressed = false;
    btn.set_click_handler(*this, &my_window::BtnPress);

    //label setup
    output.set_pos(btn.left() - 150, btn.bottom() - 400);
    output_text_copy = "";

    //menu bar setup
    mbar.set_number_of_menus(2);
    mbar.set_menu_name(0, "Help", 'H');
    mbar.menu(0).add_menu_item(menu_item_text("Instructions", *this, &my_window::ShowInstructions, 'I'));
    mbar.set_menu_name(1, "Options", 'O');
    mbar.menu(1).add_menu_item(menu_item_text("Toggle Log", *this, &my_window::Toggle_Log, 'L'));
    //mbar.menu(1).add_menu_item(menu_item_text("Menu Window", *this, &my_window::ShowOptions, 'M'));

    ofstream fout(FILE_NAME, ofstream::trunc);
    fout << "\n\r";
    fout.close();

    //Window setup
    set_title("CervPro Diagnostic");
    set_size(380, 480);
    show();
  }

  /// <summery>
  /// When a window is destroyed close it.
  /// </summery>
  ~my_window() { close_window(); }

  void update_log(const std::list<point> &points)
  {
    ++frames;
    string temp = "";
    size_t counter = 0;
    for (point p : points)
    {
      ++counter;
      if (counter % 5 == 0)
        temp += "\n\r";
      temp += "\t";
      temp += "(" + to_string(p.x()) + ", " + to_string(p.y()) + ") ";
    }
    //output.set_text(output_text_copy);
    if (log)
    {
      ofstream fout(FILE_NAME, ofstream::app);
      fout << temp << "\n\r";
      fout.close();
    }
  }

  void update_text(const string &update)
  {
    if (frames % 50 == 0)
      output_text_copy = "";
    output_text_copy += update;
    output.set_text(output_text_copy);
    if (log)
    {
      ofstream fout(FILE_NAME, ofstream::app);
      //fout.open(FILE_NAME, app); // by default clears the file if it already exists
      fout << output_text_copy << "\n\r";
      fout.close();
    }
  }

  void update_log(const string &update)
  {
    if (log)
    {
      ofstream fout(FILE_NAME, ofstream::app);
      fout << output_text_copy << "\n\r";
      fout.close();
    }
  }

  /// <summery>
  /// This method is used for finding the best way to calculate the best
  /// way to find the ending difference.
  /// </summery>
  void update_text(const std::vector<double> final_values)
  {
    output_text_copy = "";
    output.set_text(output_text_copy);
    for (double d : final_values)
    {
      string str = "Final angle difference: " + to_string(d) + "\r\n";
      update_text(str);
    }
  }
};

std::list<std::list<point>> stored_faces;
my_window data_win;

void Set_Circumference(double c) { data_win.circumference = c; }

double radius(double circumference)
{
  return circumference / (2 * PI);
}

double Euclidean_Distance(point p1, point p2)
{
  return sqrt(pow(p1.x() - p2.x(), 2) + pow(p1.y() - p2.y(), 2));
}

double Euclidean_Distance(point p)
{
  return sqrt(pow(p.x(), 2) + pow(p.y(), 2));
}

double pixel_to_real(int dist)
{
  return (90 * dist * 0.635) / (0.413774 * 480);
}

double angular_correction(double angle)
{
  return 0.0000020278 * pow(angle, 7) - 0.0001340304 * pow(angle, 6) + 0.0032900011 * pow(angle, 5) - 0.0356139393 * pow(angle, 4) + 0.143281671 * pow(angle, 3) + 0.0795877567 * pow(angle, 2) + 0.0272896557 * angle + 0.0076780359;
  return angle;
}

double final_calculation(const size_t &framecount)
{
  auto begining = stored_faces.begin()->begin();
  auto ending = stored_faces.rbegin()->begin();
  advance(begining, 33);
  advance(ending, 33);
  // returns in radians, so convert to degrees
  double angle = atan(pixel_to_real(Euclidean_Distance(*begining, *ending)) / (/*h*/ radius(data_win.circumference))) * 180 / PI;
  return abs(angular_correction((angle - ANGLE_CORRECTION) * 0.6));
}

double avg(std::vector<double> container)
{
  double result = 0;
  std::vector<double>::size_type i = 0;
  for (i; i < container.size(); ++i)
  {
    result += container[i];
  }
  return result / i;
}

/// <summery>
/// This function calculates the final difference between
/// </summery>
std::vector<double> final_calculation()
{
  std::vector<double> result;
  size_t counter = 0;
  double distances = 0;
  for (auto i = stored_faces.begin()->begin(), j = stored_faces.rbegin()->begin(); counter < stored_faces.begin()->size(); ++i, ++j, ++counter)
  {
    double distance = Euclidean_Distance(*i, *j);
    result.push_back(abs(atan(pixel_to_real(distance) / (radius(data_win.circumference) / 2)) * 180 / PI) - ANGLE_CORRECTION);
  }
  result.push_back(avg(result));
  result.push_back(atan(pixel_to_real(distances / stored_faces.size()) / (radius(data_win.circumference) / 2)) * 180 / PI);
  return result;
}

double angular_velocity(int start_frame, time_t time_passed)
{
  auto temp = stored_faces.begin();
  advance(temp, start_frame);
  auto begining = temp->begin();
  auto end = stored_faces.rbegin()->begin();
  auto old = stored_faces.begin()->begin();
  advance(end, 33);
  advance(old, 33);
  double theta = atan(pixel_to_real(Euclidean_Distance(*begining, *end)) / (/*h*/ radius(data_win.circumference))) * 180 / PI;
  double rho = atan(pixel_to_real(Euclidean_Distance(*old, *begining)) / (/*h*/ radius(data_win.circumference))) * 180 / PI;
  theta = angular_correction(abs(theta - ANGLE_CORRECTION) * 0.6);
  rho = angular_correction(abs(rho - ANGLE_CORRECTION) * 0.6);
  return abs((theta - rho) / time_passed);
}

/// <summery>
/// Calculate the velocity from start to end over the time_passed.
/// </summery>
/// <return>The angular velocity</return>
double angular_velocity(int start, int end, time_t time_passed)
{
  auto start_itr = stored_faces.begin();
  auto end_itr = stored_faces.begin();
  advance(start_itr, start);
  advance(end_itr, end);
  auto start_point = start_itr->begin();
  auto end_point = end_itr->end();
  advance(start_point, 33);
  advance(end_point, 33);
  double theta = atan(pixel_to_real(Euclidean_Distance(*start_point, *end_point)) / radius(data_win.circumference)) * 180 / PI;
  theta = angular_correction(abs(theta - ANGLE_CORRECTION) * 0.6) * 2;
  return abs(theta) / time_passed;
}

/// <summery>
/// Find the point with the largest value
/// </summery>
/// <return>Largest point value</return>
int max_point()
{
  int result = -1;
  double max_val = 0;
  size_t counter = 0;

  for (auto itr : stored_faces)
  {
    auto temp = itr.begin();
    advance(temp, 33);
    double d_temp = Euclidean_Distance(*temp);
    if (d_temp > max_val)
    {
      max_val = d_temp;
      result = counter;
    }
    ++counter;
  }
  return result;
}

int min_point()
{
  int result = -1;
  double min_val = INFINITY;
  size_t counter = 0;

  for (auto itr : stored_faces)
  {
    auto temp = itr.begin();
    advance(temp, 33);
    double d_temp = Euclidean_Distance(*temp);
    if (d_temp < min_val)
    {
      min_val = d_temp;
      result = counter;
    }
    ++counter;
  }

  return result;
}

int main()
{
  try
  {
    cv::VideoCapture cap(0, cv::CAP_V4L);
    thread_pool &global_pool = default_thread_pool();

    if (!cap.isOpened())
    {
      return 1;
    }

    image_window win;

    // Load face detection and pose estimation models.
    frontal_face_detector detector = get_frontal_face_detector();
    shape_predictor pose_model;
    deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

    size_t frame_count = 0;
    size_t frames_checked = frame_count;
    unsigned short btn_presses = 0;

    time_t start;
    time_t end;
    time_t true_start, true_end;

    start = time(NULL);

    // Grab and process frames until the main window is closed by the user.
    while (!win.is_closed())
    {
      // Grab a frame
      cv::Mat temp;
      if (!cap.read(temp))
      {
        break;
      }

      cv_image<bgr_pixel> cimg(temp);

      // Detect faces
      std::vector<rectangle> faces = detector(cimg);
      // Find the pose of each face.
      std::list<full_object_detection> shapes;
      for (unsigned long i = 0; i < faces.size(); ++i)
        shapes.push_back(pose_model(cimg, faces[i]));

      if (data_win.btn_pressed && !shapes.empty())
      {

        if (frame_count == 0)
          true_start = time(NULL);
        /// <summery>
        /// This function copies the values from temp and stores them in stored_faces.
        /// </summery>
        // lambda example
        global_pool.add_task_by_value([=, &shapes, frame_count]() {
          std::list<point> temp2;
          auto temp3 = shapes.begin();
          //advance(temp3, frame_count);
          for (size_t i = 0; i < temp3->num_parts(); ++i)
          {
            temp2.push_back(temp3->part(i));
          }
          stored_faces.push_back(temp2);
          data_win.update_log("\r\nFrame count: " + to_string(frame_count));
          data_win.update_log(temp2);
        });

        if (frame_count % FRAMES_PER_VELOCITY_CALC == 0 && frame_count != 0)
        {
          // angular difference %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
          end = time(NULL);
          time_t time_diff = difftime(end, start);

          data_win.update_text("\n\rAngular Velocity: " + to_string(angular_velocity(frames_checked, time_diff)) + " degrees / sec");
          frames_checked = frame_count; // update the counter
          start = time(NULL);           // restart the timer
        }

        ++frame_count;

        // only increment it once because the button was only pressed once
        if (btn_presses == 0)
          ++btn_presses;
        else
        {
          // other calculations
        }
      }

      if (btn_presses == 1 && !data_win.btn_pressed)
      {
        true_end = time(NULL);
        ++btn_presses;
        win.close_window();

        global_pool.wait_for_all_tasks();

        // overall velocity
        double max_angle_v = angular_velocity(min_point(), max_point(), difftime(true_end, true_start));
        string temp_string = "\n\rOverall velocity: " + to_string(max_angle_v) + " degrees / sec";
        data_win.update_text(temp_string);

        // final calculations
        double angle_dif = final_calculation(frame_count);
        string final_calc_str = "\n\rFinal angle difference: " + to_string(angle_dif) + " degrees";
        data_win.update_text(final_calc_str);
        //data_win.update_text(final_calculation());
        break;
      }

      // Display it all on the screen
      win.clear_overlay();
      win.set_image(cimg);
      win.add_overlay(render_face_detections(std::vector<full_object_detection>(shapes.begin(), shapes.end())));
    }
    stringstream ss;
    ss << "cp -f " << FILE_NAME << " " << SAVE_PATH;
    system(ss.str().c_str());
    ss.ignore(255);
    ss << "rm " << FILE_NAME;
    system(ss.str().c_str());
    data_win.wait_until_closed();
  }
  catch (serialization_error &e)
  {
    return -5;
  }
  catch (exception &e)
  {
    //cout << e.what() << endl;
    return -6;
  }
}
