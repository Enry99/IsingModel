//#define CHANGE_SYSTEM_TIMER_RESOLUTION
#define SLEEP
#ifndef SLEEP
extern bool goDraw;
#endif
#if defined(_WIN32) || defined(WIN32)
#define _USE_MATH_DEFINES
#endif


#include "slider_input.h"
#include "AxisRangeInput.h"
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/glut.H>
//#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <vector>
#include <iostream>
#include <fstream>
#include <array>
#include <math.h>
#include <stdio.h>
#include <string.h>


//extern functions/variables (declared in main.cpp)
extern void setInitialConditions();
extern void startAlgorithm();
extern void displaySpinningTop();
extern void animationLoop();
extern void keyboardFunction(unsigned char, int, int);
extern void setAxisRange();
extern void drawGraph();
extern std::vector<std::array<double, 4>> rotation_data;
extern std::vector<double> Energy_data;
extern std::ofstream Energy_stream;
extern double values[4];
extern bool run_animation;
extern bool pause_animation;
extern double xmin_graph;
extern double xmax_graph;
extern double ymin_graph;
extern double ymax_graph;
extern int last_index;
extern int graphID;
extern const char* menu_labels[1];
extern bool autorange;


//Function declarations
void idleSpinningTop(void*);
void idleGraph(void*);
void graphmouseFunction(int, int, int, int);
void graphmouseWheelFunction(int);
void graphmouseMotionCallback(int, int);
void createMenu(int);
void activateButtons();
void deactivateButtons();
void start_callback(Fl_Widget*);
void pause_callback(Fl_Widget*);
void stop_callback(Fl_Widget*);
void reset_callback(Fl_Widget*);
void main_window_cb(Fl_Widget*, void*);
void autorangeButtonCallback(Fl_Widget*);
void graphChoiceMenuCallback(Fl_Widget*);
int CreateMyWindow(int, char**);


//Variables declarations
double zoom_scale_factor = 1;
int lastX = 0;
int lastY = 0;
int graphlastX = 0;
int graphlastY = 0;
bool mousePressed = false;
double FPS_display_width, FPS_display_height;

//sliders default values
constexpr int FPS_default = 72;
constexpr int
Nspins_def = 4,
MaxSteps_def = 1000,
stepspersec_def = 10;
constexpr bool enable_gravity_def = true;
constexpr bool enable_file_output_def = false;

//right-click menu labels
char h_label[] = "Hide spinning top [h]";


//interface elements
class MyGlutWindow;
class MyGlutWindow2;
Fl_Window * main_window;
MyGlutWindow* spinning_top_window;
MyGlutWindow2* graph_window;
Fl_Button * start_button, * stop_button, * pause_button, * reset_button;
Fl_Text_Display * vector_legend_box;
SliderInput 
* Nspins_slider,
* MaxSteps_slider,
* StepsPerSecond_slider,
* FPS_slider;
Fl_Check_Button * enable_gravity;
Fl_Check_Button* enable_file_output;
Fl_Check_Button* persistent_trail;
Fl_Check_Button* autorange_button;
Fl_Check_Button* x_zoom_only;
Fl_Choice* menu;
AxisRangeInput* axis_boxes;

//end of declarations_____________________________


//Spinning top window class
class MyGlutWindow : public Fl_Glut_Window {

    void init()
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);

        //right-click menu on main viewport
        glutCreateMenu(createMenu);
        glutAddMenuEntry(h_label, 'h');
        glutAttachMenu(GLUT_RIGHT_BUTTON);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);  
    }

    void FixViewport(int W, int H) 
    {
        if (W == 0 || H == 0) return;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, pixel_w(), pixel_h());  //Use the whole window for rendering

        FPS_display_width = (double)W / Fl::w() / (0.5 * 0.9), //size is fixed (does not vary if resized)
        FPS_display_height = (double)H / Fl::h() / (0.8 * 0.95);

        glMatrixMode(GL_MODELVIEW);
    }

    void draw() // DRAW METHOD
    {
        this->make_current();
        static bool first_time = true;
        if (first_time) { valid(1); init();  FixViewport(w(), h()); first_time = false; }
       
        displaySpinningTop();
    }

    void resize(int X, int Y, int W, int H) 
    {
        this->make_current();
        Fl_Gl_Window::resize(X, Y, W, H);
        FixViewport(W, H);
        redraw();
    }

public:


    // OPENGL WINDOW CONSTRUCTOR
    MyGlutWindow(int X, int Y, int W, int H, const char* L = 0, bool use_mouse_keyboard = false) : Fl_Glut_Window(X, Y, W, H, L)
    {
        mode(FL_RGB | FL_DEPTH);
        Fl::add_idle(idleSpinningTop, this);
        end();
    }

};


//graph window class
class MyGlutWindow2 : public Fl_Glut_Window {

    void init()
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glEnable(GL_DEPTH_TEST);
    }

    void FixViewport(int W, int H) {
        if (W == 0 || H == 0) return;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, pixel_w(), pixel_h());  //Use the whole window for rendering
        glMatrixMode(GL_MODELVIEW);

    }
    // DRAW METHOD
    void draw()
    {
        this->make_current();
        static bool first_time = true;
        if (first_time) { valid(1); init();  FixViewport(w(), h()); first_time = false; }

        setAxisRange();
        drawGraph();
    }

    void resize(int X, int Y, int W, int H)
    {
        this->make_current();
        Fl_Gl_Window::resize(X, Y, W, H);
        FixViewport(W, H);
        redraw();
    }

public:


    // OPENGL WINDOW CONSTRUCTOR
    MyGlutWindow2(int X, int Y, int W, int H, const char* L = 0) : Fl_Glut_Window(X, Y, W, H, L)
    {
        
        mode(FL_RGB | FL_DEPTH);
        Fl::add_idle(idleGraph, this);
        this->mouse = graphmouseFunction;
        this->motion = graphmouseMotionCallback;
        end();
    }

};


void idleSpinningTop(void*)
{
    animationLoop(); //updates data (evolve)
#ifdef SLEEP  
    spinning_top_window->redraw();  //draws using displaySpinningTop
#else
    if (goDraw) {
        spinning_top_window->redraw();
        goDraw = false;
    }
#endif
}


void idleGraph(void*)
{    
    graph_window->redraw();
}


void graphmouseFunction(int button, int state, int x, int y)
{
    graphlastX = x;
    graphlastY = y;
    mousePressed = state == GLUT_DOWN;
    if (Fl::event() == FL_MOUSEWHEEL || Fl::event() == FL_ZOOM_GESTURE) graphmouseWheelFunction(Fl::event_dy());
}


void graphmouseWheelFunction(int wheel_motion)
{
    float zoom_scale = 0.03;
    
    float delta_x = xmax_graph - xmin_graph;
    float delta_y = ymax_graph - ymin_graph;
    
    xmin_graph -= zoom_scale * delta_x * wheel_motion;
    xmax_graph += zoom_scale * delta_x * wheel_motion;

    if (!x_zoom_only->value())
    {
        ymin_graph -= zoom_scale * delta_y * wheel_motion;
        ymax_graph += zoom_scale * delta_y * wheel_motion;
    }
    axis_boxes->set_text_values(xmin_graph, xmax_graph, ymin_graph, ymax_graph);
   
}


void graphmouseMotionCallback(int xpos, int ypos)
{
    if (mousePressed) //mousePressed is updated by mouseFunction
    {
        float dx = graphlastX - xpos; 
        float dy = ypos - graphlastY;
        graphlastX = xpos;
        graphlastY = ypos;

        float motion_scale = 1.;
        float delta_x = xmax_graph - xmin_graph;
        float delta_y = ymax_graph - ymin_graph;
        float xSensitivity = motion_scale * delta_x / graph_window->pixel_w();
        float ySensitivity = motion_scale * delta_y / graph_window->pixel_h();
        dx *= xSensitivity;
        dy *= ySensitivity;

        xmin_graph += dx, xmax_graph += dx;
        ymin_graph += dy, ymax_graph += dy;
    }
    axis_boxes->set_text_values(xmin_graph, xmax_graph, ymin_graph, ymax_graph);
}


void graphChoiceMenuCallback(Fl_Widget* f)
{
    graphID = ((Fl_Choice*)f)->value();
    last_index = 0; //reset for autorange
}


void autorangeButtonCallback(Fl_Widget* f)
{
    autorange = ((Fl_Check_Button*)f)->value();
    if (autorange)
    {
        axis_boxes->deactivate();
        x_zoom_only->deactivate();
    }
    else
    {
        axis_boxes->activate();
        x_zoom_only->activate();
    }
  
}


void createMenu(int key) { 
    keyboardFunction((unsigned char)key, 0, 0); 
}


void activateButtons()
{
    MaxSteps_slider->activate();
    Nspins_slider->activate();
}


void deactivateButtons()
{
    MaxSteps_slider->deactivate();
    Nspins_slider->deactivate();
}


void start_callback(Fl_Widget* f) {

    Energy_data.clear();
    last_index = 0;

    pause_animation = false, pause_button->clear();

    deactivateButtons();

    setInitialConditions();

    startAlgorithm();

}


void pause_callback(Fl_Widget* f) { 
    pause_animation= !pause_animation; 
}


void stop_callback(Fl_Widget* f) {

    run_animation = false, pause_animation = false, pause_button->clear();

    rotation_data.clear();

    if (enable_file_output->value())
    {
        Energy_stream.close();
    }
    activateButtons();
}


void reset_callback(Fl_Widget* f)
{
    Nspins_slider->value(Nspins_def),
    MaxSteps_slider->value(MaxSteps_def),
    StepsPerSecond_slider->value(stepspersec_def),
    FPS_slider->value(FPS_default),
    enable_gravity->value(enable_gravity_def);
}


void main_window_cb(Fl_Widget* widget, void*)
{
#if defined SLEEP && (defined(_WIN32) || defined(WIN32)) && defined CHANGE_SYSTEM_TIMER_RESOLUTION
    timeEndPeriod(1);
#endif

    return exit(EXIT_SUCCESS);
}


int CreateMyWindow(int argc, char** argv) {
    
#if defined SLEEP && (defined(_WIN32) || defined(WIN32)) && defined CHANGE_SYSTEM_TIMER_RESOLUTION
    timeBeginPeriod(1); //allows better control over FPS in Windows. 
    //Use with caution, since it changes the minimum resolution of periodic timers of the whole system and thus increases cpu usage.
#endif

    Fl::scheme("gtk+"); //style, comment to get traditional look

    Fl::use_high_res_GL(1); //for Apple retina display
    
    //SETTING CONTAINER WINDOW_________________________________________________________________________________   
    int w_ext = 0.9 * Fl::w(); //container window sizes
    int h_ext = 0.9 * Fl::h(); 
    double x0 = w_ext * 0.80; //x position of sliders (pixel count from main window's left edge)
    double y0 = h_ext * 0.04;
    double delta_h = h_ext * 0.055; //height step between sliders' upper-left corner 
    double slider_width = w_ext * 0.16; 
    double slider_height = h_ext * 0.027; // 0.042;
    double button_width = w_ext * 0.15;
    double button_height = h_ext * 0.1;
    double xbuttons = 0.6 * w_ext;
    double y_buttons = 0.85 * h_ext;

    main_window = new Fl_Window((Fl::w()- w_ext)/2, (Fl::h()-h_ext)/2, w_ext, h_ext, "Lagrange spinning top");
    main_window->resizable(main_window);
    main_window->callback(main_window_cb);
    main_window->show(argc, argv);
    main_window->begin();
    

    //SETTING SUBWINDOWS AND SLIDERS___________________________________________________________________________

    //subwindows
    spinning_top_window = new MyGlutWindow(0.020 * w_ext, 0.025 * h_ext, 0.45 * w_ext, 0.95 * h_ext, "Top_window", true);   //9./16. * 0.95 * w_ext
    graph_window = new MyGlutWindow2(x0 - 1.88 * slider_width, 0.025 * h_ext, 0.9 * slider_width+ x0 - 0.94 * slider_width - (x0 - 1.88 * slider_width) , 0.5 * main_window->h());


    //sliders
    Nspins_slider = new SliderInput(x0, y0, slider_width, slider_height, "N for NxN grid", &values[0]);
    MaxSteps_slider = new SliderInput(x0, y0 += delta_h, slider_width, slider_height, "Nsteps", &values[1]);
    StepsPerSecond_slider = new SliderInput(x0, y0 += delta_h, slider_width, slider_height, "StepsPerSecond", &values[2]);
    FPS_slider = new SliderInput(x0, y0 += delta_h, slider_width, slider_height, "FPS max", &values[3]);


    //buttons
    start_button = new Fl_Button(x0, y_buttons, button_width, button_height, "@#>\tStart");
    start_button->color(FL_GREEN);
    pause_button = new Fl_Toggle_Button(x0 - 0.94 * slider_width, y_buttons, button_width, button_height, "@#||\tPause/Unpause");
    pause_button->color(FL_YELLOW);
    stop_button = new Fl_Button(x0 - 1.88 * slider_width, y_buttons, button_width, button_height, "@#square\tStop");
    stop_button->color(FL_RED);
    reset_button = new Fl_Button(x0 - 1.88 * slider_width, y_buttons - 0.75* button_height, button_width, 0.6*button_height, "Reset sliders");
    reset_button->color(FL_CYAN);
    enable_gravity = new Fl_Check_Button(x0 - 0.7*slider_width, 0.78 * h_ext, 0.4*slider_width, slider_height, "Enable gravity");
    enable_file_output = new Fl_Check_Button(x0 - 0.7*slider_width, 0.65 * h_ext, 0.5*slider_width, slider_height, "Output data on file");
    autorange_button = new Fl_Check_Button(x0 - 0.7 * slider_width, 0.028 * h_ext + 0.5 * main_window->h(), 0.5 * slider_width, slider_height, "Auto range");
    x_zoom_only = new Fl_Check_Button(x0 - 0.7 * slider_width, 0.028 * h_ext + 0.5 * main_window->h()+slider_height, 0.5 * slider_width, slider_height, "X axis only zoom");
    menu = new Fl_Choice(x0 - 1.88 * slider_width, 0.028 * h_ext + 0.5 * main_window->h(), 0.9*slider_width, slider_height);
    axis_boxes = new AxisRangeInput(x0 - 1.88 * slider_width, 0.045 * h_ext + 0.5 * main_window->h() + slider_height, 1.1*slider_width, slider_height,
        &xmin_graph, &xmax_graph, &ymin_graph, &ymax_graph, "xmin, xmax, ymin, ymax");
    axis_boxes->deactivate();
    x_zoom_only->deactivate();

    for (auto i : menu_labels) menu->add(i);
    menu->callback(graphChoiceMenuCallback);
    autorange_button->callback(autorangeButtonCallback);

    //setting default values and bounds
    Nspins_slider->bounds(4, 1000);
    Nspins_slider->value(Nspins_def);

    MaxSteps_slider->bounds(100, 1'000'000);
    MaxSteps_slider->value(MaxSteps_def);

    StepsPerSecond_slider->bounds(1, 1000);
    StepsPerSecond_slider->value(stepspersec_def);

    FPS_slider->bounds(20, 360);
    FPS_slider->step(1);
    FPS_slider->value(FPS_default);

    enable_gravity->value(enable_gravity_def);
    enable_file_output->value(enable_file_output_def);

    start_button->callback(start_callback);
    pause_button->callback(pause_callback);
    stop_button->callback(stop_callback);
    reset_button->callback(reset_callback);
    autorange_button->value(1);
    x_zoom_only->value(0);

    main_window->end();
    
    main_window->show();
    spinning_top_window->show();
    graph_window->show();

    return (Fl::run());
}