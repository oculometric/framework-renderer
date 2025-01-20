#pragma once

#include <windows.h>
#include <string>

#pragma warning( disable : 4100)

#include "stui.h"
#include "stui_extensions.h"

// encapsulates easy debug output and alerts
class FDebug
{
	friend class FApplication;
private:
	HWND window;

	stui::Page* page = nullptr;
	stui::TextArea* log_text_area = nullptr;

	stui::VerticalBox* render_stats_box = nullptr;
	stui::Label* frame_time_label = nullptr;
	stui::Label* framerate_label = nullptr;
	stui::Label* clear_timing_label = nullptr;
	stui::Label* shadows_timing_label = nullptr;
	stui::Label* uniforms_timing_label = nullptr;
	stui::Label* batching_timing_label = nullptr;
	stui::Label* objects_timing_label = nullptr;
	stui::Label* postprocessing_timing_label = nullptr;
	stui::Label* gizmos_timing_label = nullptr;
	stui::Label* objects_count_label = nullptr;
	stui::Label* meshes_count_label = nullptr;
	stui::Label* triangles_drawn_label = nullptr;
	stui::Label* lights_count_label = nullptr;
	stui::Label* camera_forward_label = nullptr;

	stui::VerticalBox* physics_stats_box = nullptr;
	stui::Label* tick_time_label = nullptr;
	stui::Label* components_count_label = nullptr;

	stui::Spinner* spinner = nullptr;
	stui::Button* quit_button = nullptr;

	FDebug(HWND window_handle);
	static void set(FDebug* debug);			// assigns an instance pointer to the static reference in the source file

public:
	static FDebug* get();					// gets the value of the static reference from the source file

	inline void setFrameTime(float f) { frame_time_label->text = format("{:4f}ms", f); framerate_label->text = format("{:3f}fps", 1.0f / f); }
	inline void setTimings(float clear, float shadows, float uniforms, float batching, float objects, float post, float gizmos)
	{
		clear_timing_label->text = format("{:4f}ms", clear);
		shadows_timing_label->text = format("{:4f}ms", shadows);
		uniforms_timing_label->text = format("{:4f}ms", uniforms);
		batching_timing_label->text = format("{:4f}ms", batching);
		objects_timing_label->text = format("{:4f}ms", objects);
		postprocessing_timing_label->text = format("{:4f}ms", post);
		gizmos_timing_label->text = format("{:4f}ms", gizmos);
	}
	inline void setCounts(int objects, int meshes, int triangles, int lights)
	{
		objects_count_label->text = format("{}", objects);
		meshes_count_label->text = format("{}", meshes);
		triangles_drawn_label->text = format("{}", triangles);
		lights_count_label->text = format("{}", lights);
	}
	inline void setCameraForward(float x, float y, float z)
	{
		camera_forward_label->text = format("{:2f} {:2f} {:2f}", x, y, z);
	}

	inline void setTickTime(float f) { tick_time_label->text = format("{:4f}ms", f); }
	inline void setComponentCount(int comps) { components_count_label->text = format("{}", comps); }

	static void update();
	static void console(std::string s);		// prints a string to both stdout and the visual studio immediate window
	static void dialog(std::string s); 		// shows a string as an onscreen dialog box
};
