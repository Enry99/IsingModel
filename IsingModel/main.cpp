//#define DATA_OUTPUT
#define SLEEP
#ifndef SLEEP
bool goDraw = true;
#endif
#if defined(WIN32)|| defined(_WIN32)
#define _USE_MATH_DEFINES
#endif

#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/glut.H>
//#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Check_Button.H>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <sstream>
#include <cmath>
#include <iostream>
#include <thread>
#include <time.h>
#include <cstdarg>

#include <exception>
#include <stdexcept>

//extern functions/variables (defined in window.cpp)
extern int CreateMyWindow(int argc, char** argv);
extern void activateButtons();
extern Fl_Check_Button* enable_gravity;
extern Fl_Check_Button* enable_file_output;
extern Fl_Check_Button* persistent_trail;
extern double FPS_display_width, FPS_display_height;

//variables declarations
int Nspins; //NxN grid
unsigned long long int step_i, Nsteps;
double steps_per_second;
double accumulator = 0;
double values[4]{}; //values tied to sliders
bool run_animation = false; //run starts the animation when the first frame is ready
bool pause_animation = false; //pause is controlled by button
bool
DRAW_SUN = true,
DRAW_FLOOR = true,
SHOW_FPS = true,
DRAW_TRAIL = true,
DRAW_AXIS = true,
DRAW_TOP = true,
DRAW_VECTORS = false,
hide_vectors, //hides vectors when animation is stopped
rainbow = false;
std::chrono::steady_clock::time_point 
idle_prev_time = std::chrono::steady_clock::now(),
previous_time, 
FPS_previous_time; //clock for fps count, is updated every second
int frames_counter = 0, FPS = 0;
std::vector<std::array<double, 4>> rotation_data;
std::vector<double> Energy_data;
std::ofstream Energy_stream;

double color = 0;

//functions declarations
void keyboardFunction(unsigned char, int, int);
void drawFPS(std::string);
void displaySpinningTop();
void setInitialConditions();
void evolve();
void startAlgorithm();
void animationLoop();




void keyboardFunction(unsigned char key, int, int)
{

    switch (key)
    {
    case 'h': DRAW_TOP = !DRAW_TOP;  break;
    case 27: exit(0);   break;// exit program when [ESC] key presseed
    default:   break;
    }

}


void drawFPS(std::string text)
{

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, FPS_display_width, FPS_display_height, 0.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0, 1, 0);
    glRasterPos2f(0.007, 0.015);
    for (auto i : text) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, i);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}


void displaySpinningTop()
{
    if (!frames_counter) FPS_previous_time = std::chrono::steady_clock::now();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, values[0], 0, values[0]);
    glMatrixMode(GL_MODELVIEW);

    //std::cout << step_i << '\n';

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int side = 1;

    for (int i = 0; i < values[0]; ++i)
    {
        for (int j = 0; j < values[0]; ++j)
        {
            glColor3f(abs(sin(i * color + j)), 0, 0);
            glBegin(GL_POLYGON);
            glVertex2f(i, j);
            glVertex2f(i + side, j);
            glVertex2f(i + side, j + side);
            glVertex2f(i, j + side);
            glEnd();
        }
    }
       
    
    ++frames_counter;
    if((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - FPS_previous_time)).count()*1.e-6 >=1) //updated every second
    {
        FPS = frames_counter;
        frames_counter = 0;
    }
    if(SHOW_FPS) drawFPS( std::string("FPS: ") + std::to_string( FPS ) );

    //glutSwapBuffers();
    glFlush();
}

void setInitialConditions()
{    
    Nspins = values[0];
    Nsteps = values[1];

    if(enable_file_output->value())
    {
        std::ofstream initial_values_stream("initial_values.txt");
        const char* slider_names[4] = {
            "N of NxN grid = ",
            "mass = ",
            "tf = ",
            "FPS max"
        };
        for (int i = 0; i < 4; ++i)
        {
            initial_values_stream << slider_names[i] << values[i] << '\n';
        }
        initial_values_stream.clear();
        initial_values_stream.close();

        Energy_stream.open("Energy.txt");
    }

    rotation_data.reserve(Nsteps);
}

void evolve()
{
    if (step_i < Nsteps && run_animation)
    {
        //RUNGE-KUTTA LOOP
        auto new_time = std::chrono::steady_clock::now();
        double frame_time = std::chrono::duration_cast<std::chrono::microseconds>(new_time - previous_time).count() * 1.e-6;
        previous_time = new_time;
        //frame_time e' il tempo tra un frame e l'altro. l'algoritmo avanza del numero
        //intero di timestep più vicino a frame_time (in difetto)

        if (!pause_animation) accumulator += frame_time;
        //accumulator accumula il tempo che la simulazione deve calcolare.
        //a ogni step viene decrementato di dt. se e' rimasto un resto
        //questo aumenta fino a poter costutire un intero delta_t
        //quindi ogni tanto viene calcolato un timestep in più tra un frame
        //e il successivo, ma e' r_top_quatstanza trascurabile.
        //in pratica ogni volta viene mandato fuori un fotogramma,
        //poi si guarda quanto tempo e' passato, si evolve fino al nuovo tempo e si manda fuori il nuovo fotogramma

        


        while (accumulator >= 1/steps_per_second && step_i < Nsteps)
        {
            double E = sin( 0.1 *step_i);

            Energy_data.push_back(E);

            if (enable_file_output->value())
            {
                Energy_stream << step_i << '\t' << E << '\n';
            }

            accumulator -= 1 / steps_per_second;
            ++step_i;
        }

        color = step_i;

        rotation_data.push_back({ 0, 0, 0, 0 });
    }
    else
    {
        std::cout << "\nSimulazione terminata.\n";

        if (enable_file_output->value())
        {

            Energy_stream.clear();
            Energy_stream.close();
        }

        run_animation = false; hide_vectors = true;
        activateButtons();
    }

}

void startAlgorithm()
{
    step_i = 0; accumulator = 0;
    run_animation = true;
    previous_time = std::chrono::steady_clock::now();
}

void animationLoop()
{
#ifdef SLEEP
    
    std::this_thread::sleep_for(std::chrono::microseconds((int)(1'000'000 / values[3]) - (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - idle_prev_time)).count()));
    idle_prev_time = std::chrono::steady_clock::now();

    steps_per_second = values[2];
    
    if (run_animation) evolve();
    else
    {
        //insert code to visualize initial configuration !
        color = 1;
    }

#else
    if ((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - idle_prev_time)).count() * 1.e-6 >= 1. / values[3])
    {
        idle_prev_time = std::chrono::steady_clock::now();
        
        if (!run_animation)
        {

        }
        else evolve();
        goDraw = true;
    }
#endif
         
}

int main (int argc, char **argv) {

        CreateMyWindow(argc, argv);
}
