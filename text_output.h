#ifndef __TEXT_OUTPUT_H__
#define __TEXT_OUTPUT_H__

//#include "..\grim_api\grim.h"
#include "engine.h"
#include "resource_handler.h"
#include "sinecosine.h"

class text_output;

class text_output
{
protected:
	float screen_width;
	float screen_height;
	Engine *grim;
	resource_handler *resources;

	float message_timer[5];//timer for how long to show message
	float message_fade_time[5];//how long until the message fades
	std::string message_text[5];

	sinecosine sincos;

public:
	int default_font;
	int line_texture;
	void initialize_text_output(Engine *engine, resource_handler *resources, float screen_width, float screen_height);
	void write(int font, const std::string& text, float size, float x0,float y0, float x1, float y1, float r, float g, float b, float alpha);
	void find_letter_width(char kirjain, int *nume, int *kirjainleveys);
	int write_line(int font, float x, float y, const std::string& text, float size);
	bool accept_messages;
	void message(float timer, float fade_time, const std::string& message);
	void draw_messages(float elapsed);
	void draw_line(float x,float y, float x2, float y2, float width, float fade0, float fade1, float r, float g, float b);
};


#endif
