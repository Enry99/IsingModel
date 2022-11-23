#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <vector>
#include <array>
#include "AxisRangeInput.h"

extern Fl_Choice* menu;
extern Fl_Check_Button* autorange_button;
extern AxisRangeInput* axis_boxes;
extern std::vector<std::array<double,2>> Magnetization_data;
extern std::vector<std::array<double, 2>> H_field_ext_data;
extern unsigned long long int step_i;

double xmin_graph = -1;
double xmax_graph = 1;
double ymin_graph = -1;
double ymax_graph = 1;
bool autorange = true;
double data_xMax;
double data_xMin;
double data_yMax;
double data_yMin;
unsigned long long last_index = 0; //reset when changing plot or restart algo
int graphID = -1;

const char* graph_menu_labels[]
{
    "None",
    "Magnetization (t)",
    "Hysteresis (M vs H)"
};

enum graphMenuItems
{
    NONE,
    MAGNETIZATION_T,
    HYSTERESYS
};


void setAxisRange()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(xmin_graph, xmax_graph, ymin_graph, ymax_graph);
    if(autorange) axis_boxes->set_text_values(xmin_graph, xmax_graph, ymin_graph, ymax_graph);
    glMatrixMode(GL_MODELVIEW);
}

template<class T>
void xvsy_plot(std::vector<T>& x_data, int x_index, std::vector<T>& y_data, int y_index)
{
    if (autorange_button->value() && y_data.size() && x_data.size())
    {
        if (last_index == 0)
        {
            data_xMax = x_data[0][x_index];
            data_xMin = x_data[0][x_index];
            data_yMax = y_data[0][y_index];
            data_yMin = y_data[0][y_index];
        }
        for (; last_index < y_data.size(); ++last_index)
        {
            data_xMax = (std::max)(data_xMax, x_data[last_index][x_index]);
            data_xMin = (std::min)(data_xMin, x_data[last_index][x_index]);
            data_yMax = (std::max)(data_yMax, y_data[last_index][y_index]);
            data_yMin = (std::min)(data_yMin, y_data[last_index][y_index]);
        }

        xmax_graph = data_xMax + abs(data_xMax) * 0.1;
        xmin_graph = data_xMin - abs(data_xMin) * 0.1;
        ymax_graph = data_yMax + abs(data_yMax) * 0.1;
        ymin_graph = data_yMin - abs(data_yMin) * 0.1;

        setAxisRange();
    }

    //draw axis
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex2f(xmin_graph, 0);
    glVertex2f(xmax_graph, 0);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex2f(0, ymin_graph);
    glVertex2f(0, ymax_graph);
    glEnd();

    if (x_data.size() && y_data.size())
    {
        glPointSize(2);
        glBegin(GL_POINTS);      
        glColor3f(0, 1, 0);

        for (int i = 0; i < x_data.size(); ++i)
        {
            glVertex2f(x_data[i][x_index], y_data[i][y_index]);
        }
        glEnd();
        glPointSize(1);

    }
}


void drawGraph() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (graphID)
    {

    case MAGNETIZATION_T:
        xvsy_plot(Magnetization_data, 0, Magnetization_data, 1);
        break;
    case HYSTERESYS:
        xvsy_plot(H_field_ext_data, 1, Magnetization_data, 1);
        break;
    default:
        //draw axis
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex2f(xmin_graph, 0);
        glVertex2f(xmax_graph, 0);
        glEnd();

        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex2f(0, ymin_graph);
        glVertex2f(0, ymax_graph);
        glEnd();
    }

    glFlush();

}


