#ifndef SLIDER_INPUT_INT_H
#define SLIDER_INPUT_INT_H
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Slider.H>
#include <stdio.h>

class SliderInput_Int : public Fl_Group {
    Fl_Int_Input* input;
    Fl_Slider* slider;
    int* handled_value;

    // CALLBACK HANDLERS
    //    These 'attach' the input and slider's values together.
    //
    void Slider_CB2()
    {
        static int recurse = 0;
        if (recurse) return;
        else
        {
            recurse = 1;
            char s[80];
#if defined(WIN32)
            sprintf_s(s, "%li", int(slider->value()));
#else
            sprintf(s, "%li", slider->value());
#endif
            // fprintf(stderr, "SPRINTF(%d) -> '%s'\n", (int)(slider->value()+.5), s);
            input->value(s);          // pass slider's value to input
            if (handled_value) *handled_value = slider->value();
            recurse = 0;
        }
    }

    static void Slider_CB(Fl_Widget* w, void* data) {
        ((SliderInput_Int*)data)->Slider_CB2();
    }

    void Input_CB2()
    {
        static int recurse = 0;
        if (recurse) return;
        else {
            recurse = 1;
            int val = 0;
#if defined(WIN32)
            if (sscanf_s(input->value(), "%li", &val) != 1) val = 0;
#else
            if (sscanf(input->value(), "%li", &val) != 1) val = 0;
#endif
            // fprintf(stderr, "SCANF('%s') -> %d\n", input->value(), val);
            int min = slider->minimum(); //if value is outside range, set it to the nearest bound of the slider's range
            int max = slider->maximum();
            if (val < min) val = min;
            else if (val > max) val = max;
            slider->value(val);         // pass input's value to slider

            char s[80];
#if defined(_WIN32) || defined(WIN32)
            sprintf_s(s, "%li", val);
#else
            sprintf(s, "%li", val);
#endif

            input->value(s);

            if (handled_value) *handled_value = val;

            recurse = 0;
        }
    }
    static void Input_CB(Fl_Widget* w, void* data) {
        ((SliderInput_Int*)data)->Input_CB2();
    }

public:
    // CTOR
    SliderInput_Int(double x, double y, double w, double h, const char* l = 0, int* handled_val = nullptr) : Fl_Group(x, y, w, h, l), handled_value(handled_val) {
        int in_w = 90;
        input = new Fl_Int_Input(x, y, in_w, h);
        input->callback(Input_CB, (void*)this);
        input->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);

        slider = new Fl_Slider(x + in_w, y, w - in_w, h);
        slider->type(FL_HOR_NICE_SLIDER);
        slider->callback(Slider_CB, (void*)this);

        this->step(1);

        //bounds(1, 10);     // some usable default
        //value(5);          // some usable default
        end();             // close the group
    }

    // MINIMAL ACCESSORS
    int  value() const { return slider->value(); }
    void value(int val) { slider->value(val); Slider_CB2(); }
    void minumum(int val) { slider->minimum(val); }
    int  minumum() const { return slider->minimum(); }
    void maximum(int val) { slider->maximum(val); }
    int maximum() const { return slider->maximum(); }
    void bounds(int low, int high) { slider->bounds(low, high); }
    void step(int dN) { slider->step(dN); }


    ~SliderInput_Int()
    {
        delete input;
        delete slider;
        //do not delete handled_value since it is an external variable
    }

};

#endif

