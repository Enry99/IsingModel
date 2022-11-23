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
#include <FL/Fl_Check_Button.H>
#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <thread>
#include <time.h>
#include <cstdarg>
#include <random>

//EXTERN functions/variables (defined in window.cpp)#############
extern int CreateMyWindow(int argc, char** argv);
extern void activateButtons();
//extern Fl_Check_Button* enable_gravity;
extern Fl_Check_Button* enable_file_output;
extern double FPS_display_width, FPS_display_height;
constexpr int FPS_default = 72;
//###############################################################



//variables declarations

//ISING MODEL VARIABLES##########################################
long long int 
Nspins, //NxN grid
step_i, 
Nsteps;
double J = 1;
double H_field_ext = 0;
double mu = 1;
constexpr double kb = 1;
double Tc = 2 * J / (kb * std::log(1 + std::sqrt(2)));
double T_div_Tc;

std::vector<int> spinArray;
std::mt19937 generator, generator2;
std::bernoulli_distribution bernoulli_dist;
std::uniform_int_distribution<int>* uniformdist_site;
std::uniform_real_distribution<double> uniformdist_accept(0, 1);
std::vector<std::array<double, 2>> Magnetization_data;
std::vector<std::array<double, 2>> H_field_ext_data;
std::ofstream Magnetization_stream;
enum initializations
{
    RANDOM_INIT,
    UNIFORM_UP_INIT,
    UNIFORM_DOWN_INIT
};
int SPIN_INITIALIZATION = 0;
//###############################################################



//ANIMATION VARIABLES############################################
double steps_per_second;
double accumulator = 0;
int values[3]{}; //values tied to sliders
bool run_animation = false; //run starts the animation when the first frame is ready
bool pause_animation = false; //pause is controlled by button
bool
SHOW_FPS = true;

std::chrono::steady_clock::time_point 
idle_prev_time = std::chrono::steady_clock::now(),
previous_time, 
FPS_previous_time; //clock for fps count, is updated every second
int frames_counter = 0, FPS = 0;
//###############################################################



//functions declarations#########################################
void setInitialConditions();
void initialize_spins();
void startAlgorithm();
void animationLoop();
void evolve();
void drawSpinLattice();
void drawFPS(std::string);
void drawSteps(std::string);
//void keyboardFunction(unsigned char, int, int);
//#################################################################
//END OF DECLARATIONS




//OPENGL loop stuff#########################
/*
void keyboardFunction(unsigned char key, int, int)
{

    switch (key)
    {
    case 'r': SPIN_INITIALIZATION = RANDOM_INIT; break;
    case 'u': SPIN_INITIALIZATION = UNIFORM_UP_INIT; break;
    case 'd': SPIN_INITIALIZATION = UNIFORM_DOWN_INIT; break;
    case 27: exit(0);   break;// exit program when [ESC] key presseed
    default:   break;
    }

}
*/

void startAlgorithm()
{
    step_i = 0; accumulator = 0;
    run_animation = true;
    previous_time = std::chrono::steady_clock::now();
}

void animationLoop()
{

    static int previousSpins = -1;
    static int previousInit = -1;
#ifdef SLEEP

    std::this_thread::sleep_for(std::chrono::microseconds((int)(1'000'000. / FPS_default) - (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - idle_prev_time)).count()));
    idle_prev_time = std::chrono::steady_clock::now();

    steps_per_second = values[2];

    if (run_animation) evolve();
    else
    {
        if (previousSpins != values[0] || previousInit != SPIN_INITIALIZATION)
        {
            initialize_spins();
            previousSpins = values[0];
            previousInit = SPIN_INITIALIZATION;
        }
    }

#else
    if ((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - idle_prev_time)).count() * 1.e-6 >= 1. / FPS_default)
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
//##########################################



//drawing functions#########################
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

    glBegin(GL_QUADS);
    glColor3f(0, 0, 0);
    glVertex2f(0, 0);
    glVertex2f(0.0001 * Fl::w(), 0);
    glVertex2f(0.0001 * Fl::w(), 0.04);
    glVertex2f(0, 0.04);
    glEnd();

    glColor3f(0, 1, 0);
    glRasterPos2f(0.007, 0.015);
    for (auto i : text) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, i);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}

void drawSteps(std::string text)
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
    glRasterPos2f(0.007, 0.030);
    for (auto i : text) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, i);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}

void drawSpinLattice()
{
    if (!frames_counter) FPS_previous_time = std::chrono::steady_clock::now();

    if (spinArray.empty()) return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, Nspins, 0, Nspins);
    glMatrixMode(GL_MODELVIEW);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    constexpr int side = 1;


    glBegin(GL_QUADS);
    for (size_t i = 0; i < Nspins; ++i)
        for (size_t j = 0; j < Nspins; ++j)
        {
            if (spinArray[Nspins * i + j] > 0)
            {
                glColor3f(1, 1, 1);

                glVertex2f(i, j);
                glVertex2f(i + side, j);
                glVertex2f(i + side, j + side);
                glVertex2f(i, j + side);
            }
        }
    glEnd();


    ++frames_counter;
    if ((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - FPS_previous_time)).count() * 1.e-6 >= 1) //updated every second
    {
        FPS = frames_counter;
        frames_counter = 0;
    }
    if (SHOW_FPS) drawFPS(std::string("FPS: ") + std::to_string(FPS));
    drawSteps(std::string("Step: ") + std::to_string(step_i / 1'000'000) + std::string("M"));

    //glutSwapBuffers();
    glFlush();
}
//###########################################





//ACTUAL ISING MCMC simulation###############
void setInitialConditions()
{
    initialize_spins();
    Tc = 2 * J / (kb * std::log(1 + std::sqrt(2)));
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    generator2.seed(std::chrono::system_clock::now().time_since_epoch().count());
    uniformdist_site = new std::uniform_int_distribution<int>(0, Nspins * Nspins - 1); //U( [a,b] <- (b included))

    if (enable_file_output->value())
    {
        std::ofstream initial_values_stream("initial_values.txt");
        const char* slider_names[] = {
            "N of NxN grid = ",
            "Nsteps = ",
            "StepsPerSecond = ",
            "T/Tc max =",
            "H_ext = "
        };
        for (int i = 0; i < 3; ++i)
        {
            initial_values_stream << slider_names[i] << values[i] << '\n';
        }
        initial_values_stream << slider_names[3] << T_div_Tc << '\n';
        initial_values_stream << slider_names[4] << H_field_ext << '\n';
        initial_values_stream.clear();
        initial_values_stream.close();

        Magnetization_stream.open("Magnetization.txt");
        Magnetization_stream << "Step\t\tT/Tc\t\tH_ext\t\tMagnetization\n";
    }

    Magnetization_data.reserve(Nsteps/values[2] * FPS_default);
    H_field_ext_data.reserve(Nsteps/ values[2] * FPS_default);
}

void initialize_spins()
{
    Nspins = values[0];
    Nsteps = values[1];
    spinArray.resize(Nspins * Nspins);
    switch (SPIN_INITIALIZATION)
    {
    case RANDOM_INIT:
    {
        for (int i = 0; i < spinArray.size(); ++i) spinArray[i] = bernoulli_dist(generator) ? 1 : -1;
        break;
    }
    case UNIFORM_UP_INIT:
    {
        for (int i = 0; i < spinArray.size(); ++i) spinArray[i] = 1;
        break;
    }
    case UNIFORM_DOWN_INIT:
    {
        for (int i = 0; i < spinArray.size(); ++i) spinArray[i] = -1;
        break;
    }
    default:
        break;
    }

}

void evolve()
{
    if (step_i < Nsteps)
    {
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




        while (accumulator >= 1 / steps_per_second && step_i < Nsteps)
        {

            //Metropolis-Hastings MCMC
            size_t i = (*uniformdist_site)(generator); //select spin

            //PBC --> not at boundary ? normal neighbours : pbc neighbours
            size_t up = (i + Nspins) < Nspins * Nspins ? i + Nspins : i + Nspins - Nspins * Nspins;
            size_t down = i >= Nspins ? i - Nspins : i - Nspins + Nspins * Nspins;
            size_t sx = i % Nspins ? i - 1 : i - 1 + Nspins;
            size_t dx = (i + 1) % Nspins ? i + 1 : i + 1 - Nspins;

            double deltaE = 2 * spinArray[i] * (J * (spinArray[sx] + spinArray[dx] + spinArray[up] + spinArray[down]) + mu * H_field_ext);


            if (deltaE <= 0) spinArray[i] *= -1;
            else if (uniformdist_accept(generator2) < std::exp(-deltaE / (kb * T_div_Tc * Tc))) spinArray[i] *= -1;

            /////////////////////////////////////////////////////////
            accumulator -= 1 / steps_per_second;
            ++step_i;
        }


        double M = 0;
        for (auto i : spinArray) M += i;
        M /= spinArray.size();
        Magnetization_data.push_back(std::array<double, 2>({ double(step_i), M }));
        H_field_ext_data.push_back(std::array<double, 2>({ double(step_i), H_field_ext }));

        if (enable_file_output->value())
        {
            Magnetization_stream << step_i << '\t' << T_div_Tc << '\t' << H_field_ext << '\t' << M << '\n';
        }

    }
    else
    {
        std::cout << "\nSimulazione terminata.\n";

        delete uniformdist_site;

        if (enable_file_output->value())
        {

            Magnetization_stream.clear();
            Magnetization_stream.close();
        }

        run_animation = false;
        activateButtons();
    }

}
//###########################################


int main (int argc, char **argv) {

        CreateMyWindow(argc, argv);
}
