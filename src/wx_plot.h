// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// $Id$

#ifndef WX_PLOT__H__
#define WX_PLOT__H__

#ifdef __GNUG__
#pragma interface
#endif

#define INVALID -1234565 //i know it is ugly 

#include <map>
#include <limits.h>

class wxConfigBase;

struct Point;
class MainPlot;
class Rect;

enum Mouse_mode_enum   { mmd_zoom, mmd_bg, mmd_add, mmd_range, mmd_peak }; 
enum Mouse_act_enum    { mat_start, mat_stop, mat_move, mat_cancel };

enum Aux_plot_kind_enum 
{ 
    apk_empty, 
    apk_diff, 
    apk_diff_stddev, 
    apk_diff_y_proc,
    apk_peak_pos
};

extern wxMouseEvent dummy_mouse_event;

void clear_buffered_sum();

class Plot_common
{
public:
    fp xUserScale, xLogicalOrigin; 
    bool buffer_enabled;
    std::vector<std::vector<fp> > buf;
    fp plot_y_scale;
    std::vector<std::string> zoom_hist;

    Plot_common() : xUserScale(1.), xLogicalOrigin(0.), plot_y_scale(1e3) {}
    int x2X (fp x) {return static_cast<int>((x - xLogicalOrigin) * xUserScale);}
    fp X2x (int X) { return X / xUserScale + xLogicalOrigin; }
    int dx2dX (fp dx) { return static_cast<int>(dx * xUserScale); }
    fp dX2dx (int dX) { return dX / xUserScale; }
};

class FPlot : public wxPanel
{
public:
    FPlot (wxWindow *parent, Plot_common &comm)
       : wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER),
         yUserScale(1.), yLogicalOrigin(0.), 
         common (comm), mouse_press_X(INVALID), mouse_press_Y(INVALID), 
         vlfc_prev_x(INVALID)   {}
         
    ~FPlot() {}
    wxColour get_bg_color() { return backgroundBrush.GetColour(); }
    virtual void save_settings(wxConfigBase *cf) = 0;
    void change_zoom(std::string s);
    void previous_zoom(int n=1);

protected:
    wxBrush backgroundBrush;
    wxPen activeDataPen, inactiveDataPen;
    wxPen xAxisPen;
    wxBrush activeDataBrush, inactiveDataBrush;
    wxColour colourTextForeground, colourTextBackground;
    int point_radius;
    bool line_between_points;
    bool x_axis_visible, tics_visible;
    fp yUserScale, yLogicalOrigin; 
    Plot_common &common;
    int mouse_press_X, mouse_press_Y;
    int vlfc_prev_x;

    void move_view_horizontally (bool on_left);
    void perhaps_it_was_silly_zoom_try() const;
    void draw_dashed_vert_lines (int x1, int x2 = INVALID);
    bool vert_line_following_cursor(Mouse_act_enum ma, int x=0, int x0=INVALID);
    void draw_tics (wxDC& dc, const Rect &v, 
                    const int x_max_tics, const int y_max_tics, 
                    const int x_tic_size, const int y_tic_size);
    fp get_max_abs_y (fp (*compute_y)(std::vector<Point>::const_iterator));
    void draw_data (wxDC& dc, 
                    fp (*compute_y)(std::vector<Point>::const_iterator));
    virtual void read_settings(wxConfigBase *cf) = 0;
    int y2Y (fp y) {  fp t = (y - yLogicalOrigin) * yUserScale;
                      return (fabs(t) < SHRT_MAX ? static_cast<int>(t) 
                                                 : t > 0 ? SHRT_MAX : SHRT_MIN);
                   }
    fp Y2y (int Y) { return Y / yUserScale + yLogicalOrigin; }
    int x2X (fp x) { return common.x2X(x); }
    fp X2x (int X) { return common.X2x(X); }

    DECLARE_EVENT_TABLE()
};

class MainPlot : public FPlot 
{
public:
    MainPlot (wxWindow *parent, Plot_common &comm); 
    ~MainPlot() {}
    void OnPaint(wxPaintEvent &event);
    void Draw(wxDC &dc);
    void OnLeaveWindow (wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent &event);
    void OnButtonDown (wxMouseEvent &event);
    void OnButtonUp (wxMouseEvent &event);
    void OnKeyDown (wxKeyEvent& event);
    void OnSize (wxSizeEvent& event);
    void set_scale();

    void OnPopupShowXX (wxCommandEvent& event);
    void OnPopupBuffer (wxCommandEvent& event);
    void OnPopupColor (wxCommandEvent& event);
    void OnInvertColors (wxCommandEvent& event);
    void OnPopupRadius (wxCommandEvent& event);
    void OnZoomAll (wxCommandEvent& event);
    void OnPeakInfo (wxCommandEvent& event);
    void OnPeakDelete (wxCommandEvent& event);
    void OnPeakShowTree (wxCommandEvent& event);
    void cancel_mouse_press();
    void save_settings(wxConfigBase *cf);
    void read_settings(wxConfigBase *cf);
    void set_mouse_mode(Mouse_mode_enum m);
    void update_mouse_hints();
    Mouse_mode_enum get_mouse_mode() const { return mode; }
    void add_peak(fp height, fp ctr, fp hwhm);
    void change_peak_parameters(fp height, fp ctr, fp hwhm);

private:
    Mouse_mode_enum mode, basic_mode;
    static const int max_phase_pens = 8;
    static const int max_peak_pens = 24;
    static const int max_radius = 4; //size of data point
    bool peaks_visible, phases_visible, sum_visible, data_visible; 
    wxPen sumPen, bg_pointsPen;
    wxPen phasePen[max_phase_pens], peakPen[max_peak_pens];
    int pressed_mouse_button;
    bool ctrl;
    std::vector<wxPoint> peaktops;
    int over_peak;

    void draw_x_axis (wxDC& dc, std::vector<Point>::const_iterator first,
                                   std::vector<Point>::const_iterator last);
    void draw_background_points (wxDC& dc);
    void draw_sum (wxDC& dc, std::vector<Point>::const_iterator first,
                   std::vector<Point>::const_iterator last);
    void draw_phases (wxDC& dc, std::vector<Point>::const_iterator first,
                      std::vector<Point>::const_iterator last);
    void draw_peaks (wxDC& dc, std::vector<Point>::const_iterator first,
                     std::vector<Point>::const_iterator last);
    void buffer_peaks (std::vector<Point>::const_iterator first,
                       std::vector<Point>::const_iterator last);
    void draw_peaktops (wxDC& dc);
    void prepare_peaktops();
    void look_for_peaktop (wxMouseEvent& event);
    void show_peak_info (int n);
    void show_popup_menu (wxMouseEvent &event);
    void show_peak_menu (wxMouseEvent &event);
    void peak_draft (Mouse_act_enum ma, wxMouseEvent &event =dummy_mouse_event);
    void move_peak (Mouse_act_enum ma, wxMouseEvent &event = dummy_mouse_event);
    void draw_peak_draft (int X_mid, int X_hwhm, int Y);
    bool rect_zoom (Mouse_act_enum ma, wxMouseEvent &event = dummy_mouse_event);
    void draw_rect (int X1, int Y1, int X2, int Y2);
    bool has_mod_keys(const wxMouseEvent& event); 
    void add_peak_in_range(fp xmin, fp xmax);
    bool visible_peaktops(Mouse_mode_enum mode);

    DECLARE_EVENT_TABLE()
};


class DiffPlot : public FPlot
{
public:
    DiffPlot (wxWindow *parent, Plot_common &comm) : FPlot (parent, comm), 
              y_zoom(1.), y_zoom_base(1.) { }
    ~DiffPlot() {}
    void OnPaint(wxPaintEvent &event);
    void Draw(wxDC &dc);
    void OnLeaveWindow (wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent &event);
    void OnSize (wxSizeEvent& event);
    void OnLeftDown (wxMouseEvent &event);
    void OnLeftUp (wxMouseEvent &event);
    void OnRightDown (wxMouseEvent &event);
    void OnMiddleDown (wxMouseEvent &event);
    void OnKeyDown (wxKeyEvent& event);
    bool cancel_mouse_left_press();
    void set_scale();
    void OnPopupPlot (wxCommandEvent& event);
    void OnPopupColor (wxCommandEvent& event);
    void OnPopupYZoom (wxCommandEvent& event);
    void OnPopupYZoomFit (wxCommandEvent& event);
    void OnPopupYZoomAuto (wxCommandEvent& event);
    void save_settings(wxConfigBase *cf);
    void read_settings(wxConfigBase *cf);

private:
    Aux_plot_kind_enum kind;
    fp y_zoom, y_zoom_base;
    bool auto_zoom_y;
    wxCursor cursor;
    static const int move_plot_margin_width = 20;

    void draw_diff (wxDC& dc, std::vector<Point>::const_iterator first,
                                std::vector<Point>::const_iterator last);
    void draw_zoom_text(wxDC& dc);
    void fit_y_zoom();

    DECLARE_EVENT_TABLE()
};

#endif //PLOT__H__
