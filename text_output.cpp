#include "text_output.h"

void text_output::initialize_text_output(Engine *grim, resource_handler *resources, float screen_width, float screen_height){
	this->grim=grim;
	this->screen_width=screen_width;
	this->screen_height=screen_height;
	this->resources=resources;


	message_timer[0]=0;
	message_timer[1]=0;
	message_timer[2]=0;
	message_timer[3]=0;
	message_timer[4]=0;
}


//renders text
void text_output::write(int font, const string& text, float size, float x0,float y0, float x1, float y1, bool inverse_color, float r, float g, float b, float alpha){

	float letter_width=16;
	float letter_height=16;
	float x_cursor=0;
	int kirjainleveys=0;
	float letter_x,letter_y;
	int nume=0;

	int rivi_korkeus=20*size;


	if(font==-1)font=default_font;



	grim->System_SetState_Blending(true);
	if(inverse_color){
		grim->System_SetState_BlendSrc(grBLEND_INVSRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	}
	else{
		grim->System_SetState_BlendSrc(grBLEND_SRCALPHA);
		grim->System_SetState_BlendDst(grBLEND_INVSRCALPHA);
	}

	grim->Quads_SetColor(r,g,b,alpha);

	//no lining
	if((x1<=x0)||(y1<=y0)){
		write_line(font, x0, y0, text,size);
		return;
	}

	//grim->Quads_SetRotation(0);
	resources->Texture_Set(font);
	grim->Quads_Begin();

	//lining
	int rivin_pituus=(x1-x0);
	int sana_alku=0;
	int sana_loppu=1;
	int teksti_pituus=text.length();
	int rivi=0;
	int a=0;

	//kirjain kerrallaan kunnes tulee loppu tai väli
	while(true){
		if(a>=teksti_pituus)break;
			//väli
			if((text[a]==' ')||(a==teksti_pituus-1)){
				if(rivi*rivi_korkeus>y1-y0)return;//alareuna tuli vastaan, lopetetaan

				if(x_cursor>rivin_pituus){//on menty rivin reunan yli, vaihdetaan riviä
					x_cursor=0;
					rivi++;
				}

				//tulostetaan sana tähän
				sana_loppu=a+1;
				for(int b=sana_alku;b<sana_loppu;b++){
					find_letter_width(text[b],&nume,&kirjainleveys);


					if(nume>0){
						letter_x=((nume%16)*0.0625f);
						letter_y=((int)(nume/16)*0.125f);

						grim->Quads_SetSubset(letter_x,letter_y,letter_x+0.0625f,letter_y+0.125f);
						grim->Quads_Draw(x0+x_cursor, y0+rivi*rivi_korkeus, letter_width*size, letter_height*size);
					}

					//rivinvaihto
					if(kirjainleveys==-1)x_cursor=rivin_pituus;


					x_cursor+=(kirjainleveys+3)*size*0.5f;
				}
				//seuraavan sanan alku on tämän loppu
				sana_alku=a+1;
				sana_loppu=a+1;

			}
		a++;
	}

	grim->Quads_End();

}


void text_output::find_letter_width(char kirjain, int *nume, int *kirjainleveys){

		switch (kirjain)
		{
			case ' ':{*kirjainleveys=16;*nume=0;break;}
			case 'a':{*kirjainleveys=16;*nume=30;break;}
			case 'b':{*kirjainleveys=16;*nume=31;break;}
			case 'c':{*kirjainleveys=13;*nume=32;break;}
			case 'd':{*kirjainleveys=16;*nume=33;break;}
			case 'e':{*kirjainleveys=16;*nume=34;break;}
			case 'f':{*kirjainleveys=13;*nume=35;break;}
			case 'g':{*kirjainleveys=16;*nume=36;break;}
			case 'h':{*kirjainleveys=16;*nume=37;break;}
			case 'i':{*kirjainleveys=6;*nume=38;break;}
			case 'j':{*kirjainleveys=6;*nume=39;break;}
			case 'k':{*kirjainleveys=18;*nume=40;break;}
			case 'l':{*kirjainleveys=6;*nume=41;break;}
			case 'm':{*kirjainleveys=22;*nume=42;break;}
			case 'n':{*kirjainleveys=16;*nume=43;break;}
			case 'o':{*kirjainleveys=16;*nume=44;break;}
			case 'p':{*kirjainleveys=16;*nume=45;break;}
			case 'q':{*kirjainleveys=16;*nume=46;break;}
			case 'r':{*kirjainleveys=14;*nume=47;break;}
			case 's':{*kirjainleveys=13;*nume=48;break;}
			case 't':{*kirjainleveys=11;*nume=49;break;}
			case 'u':{*kirjainleveys=16;*nume=50;break;}
			case 'v':{*kirjainleveys=16;*nume=51;break;}
			case 'w':{*kirjainleveys=22;*nume=52;break;}
			case 'x':{*kirjainleveys=16;*nume=53;break;}
			case 'y':{*kirjainleveys=16;*nume=54;break;}
			case 'z':{*kirjainleveys=16;*nume=55;break;}
			case 'å':{*kirjainleveys=16;*nume=56;break;}
			case 'ä':{*kirjainleveys=16;*nume=57;break;}
			case 'ö':{*kirjainleveys=16;*nume=58;break;}
			case '0':{*kirjainleveys=16;*nume=59;break;}
			case '1':{*kirjainleveys=8;*nume=60;break;}
			case '2':{*kirjainleveys=16;*nume=61;break;}
			case '3':{*kirjainleveys=16;*nume=62;break;}
			case '4':{*kirjainleveys=18;*nume=63;break;}
			case '5':{*kirjainleveys=16;*nume=64;break;}
			case '6':{*kirjainleveys=16;*nume=65;break;}
			case '7':{*kirjainleveys=16;*nume=66;break;}
			case '8':{*kirjainleveys=16;*nume=67;break;}
			case '9':{*kirjainleveys=16;*nume=68;break;}
			case '-':{*kirjainleveys=13;*nume=69;break;}
			case '.':{*kirjainleveys=8;*nume=70;break;}
			case '/':{*kirjainleveys=14;*nume=71;break;}
			case ':':{*kirjainleveys=8;*nume=72;break;}
			case '´':{*kirjainleveys=6;*nume=73;break;}
			case 0x27:{*kirjainleveys=6;*nume=73;break;}
			case '?':{*kirjainleveys=16;*nume=74;break;}
			case ',':{*kirjainleveys=8;*nume=75;break;}
			case '!':{*kirjainleveys=8;*nume=76;break;}
			case '(':{*kirjainleveys=10;*nume=77;break;}
			case ')':{*kirjainleveys=10;*nume=78;break;}
			case '+':{*kirjainleveys=13;*nume=79;break;}
			case '=':{*kirjainleveys=16;*nume=80;break;}
			case '\\':{*kirjainleveys=-1;*nume=0;break;}

			case 'A':{*kirjainleveys=16;*nume=1;break;}
			case 'B':{*kirjainleveys=16;*nume=2;break;}
			case 'C':{*kirjainleveys=16;*nume=3;break;}
			case 'D':{*kirjainleveys=16;*nume=4;break;}
			case 'E':{*kirjainleveys=13;*nume=5;break;}
			case 'F':{*kirjainleveys=13;*nume=6;break;}
			case 'G':{*kirjainleveys=16;*nume=7;break;}
			case 'H':{*kirjainleveys=16;*nume=8;break;}
			case 'I':{*kirjainleveys=7;*nume=9;break;}
			case 'J':{*kirjainleveys=15;*nume=10;break;}
			case 'K':{*kirjainleveys=17;*nume=11;break;}
			case 'L':{*kirjainleveys=13;*nume=12;break;}
			case 'M':{*kirjainleveys=22;*nume=13;break;}
			case 'N':{*kirjainleveys=18;*nume=14;break;}
			case 'O':{*kirjainleveys=16;*nume=15;break;}
			case 'P':{*kirjainleveys=16;*nume=16;break;}
			case 'Q':{*kirjainleveys=16;*nume=17;break;}
			case 'R':{*kirjainleveys=16;*nume=18;break;}
			case 'S':{*kirjainleveys=14;*nume=19;break;}
			case 'T':{*kirjainleveys=16;*nume=20;break;}
			case 'U':{*kirjainleveys=16;*nume=21;break;}
			case 'V':{*kirjainleveys=16;*nume=22;break;}
			case 'W':{*kirjainleveys=23;*nume=23;break;}
			case 'X':{*kirjainleveys=16;*nume=24;break;}
			case 'Y':{*kirjainleveys=16;*nume=25;break;}
			case 'Z':{*kirjainleveys=16;*nume=26;break;}
			case 'Å':{*kirjainleveys=16;*nume=27;break;}
			case 'Ä':{*kirjainleveys=16;*nume=28;break;}
			case 'Ö':{*kirjainleveys=16;*nume=29;break;}
		}
}

//draws text
int text_output::write_line(int font, float x, float y, const string& text, float size){

	float letter_width=16;
	float letter_height=16;
	float x_cursor=0;
	int kirjainleveys=0;
	float letter_x,letter_y;
	int nume=0;

	if(font==-1)font=default_font;


	//grim->Quads_SetRotation(0);
	resources->Texture_Set(font);
	grim->Quads_Begin();
	//draw the letters
	int text_length=text.length();
	for(int a=0;a<text_length;a++){

		find_letter_width(text[a],&nume,&kirjainleveys);


		letter_x=((nume%16)*0.0625f);
		letter_y=((int)(nume/16)*0.125f);

		grim->Quads_SetSubset(letter_x,letter_y,letter_x+0.0625f,letter_y+0.125f);
		grim->Quads_Draw(x+x_cursor, y, letter_width*size, letter_height*size);

		x_cursor+=(kirjainleveys+3)*size*0.5f;

	}
	grim->Quads_End();

	return x_cursor;

}


void text_output::message(float timer, float fade_time, const string& message){

	int a;

	if(!accept_messages)return;

	//find if this message is already in the queue
	for(a=0;a<5;a++){
		//if it is, just set the time for it
		if(message_timer[a]>0)
		if(message==message_text[a]){
			message_timer[a]=timer;
			message_fade_time[a]=fade_time;
			return;
		}
	}

	//find the first free slot
	for(a=0;a<5;a++){
		//put the message there
		if(message_timer[a]<=0){
			message_timer[a]=timer;
			message_fade_time[a]=fade_time;
			message_text[a]=message;
			return;
		}
	}


	//no free slot found, find slot with lowest time
	int lowest=0;
	float lowest_time=message_timer[0];
	for(a=1;a<5;a++){
		if(message_timer[a]<lowest_time){
			lowest_time=message_timer[a];
			lowest=a;
		}
	}
	//put the message there
	message_timer[lowest]=timer;
	message_fade_time[lowest]=fade_time;
	message_text[lowest]=message;

}

void text_output::draw_messages(float elapsed){
	//floating message system
	grim->System_SetState_Blending(true);
	int rivi=0;
	for(int a=0;a<5;a++){
		//some message to show
		if(message_timer[a]>0){
			message_timer[a]=message_timer[a]-elapsed;
			float fade=message_timer[a]/message_fade_time[a];
			if(fade>1)fade=1;
			if(fade<0)fade=0;
			write(default_font,message_text[a],1.3f,20, 20+rivi*20,screen_width-60,screen_height,false,1,1,1,fade);
			rivi++;
		}
	}
}

void text_output::draw_line(float x,float y, float x2, float y2, float width, float fade0, float fade1, float r, float g, float b){
	resources->Texture_Set(line_texture);
	grim->Quads_Begin();
	//grim->Quads_SetColor(1,1,1,1);
	grim->Quads_SetColorVertex(0,r,g,b,fade0);
	grim->Quads_SetColorVertex(1,r,g,b,fade0);
	grim->Quads_SetColorVertex(2,r,g,b,fade1);
	grim->Quads_SetColorVertex(3,r,g,b,fade1);

	float dq=atan2(x-x2,y-y2);
	float sin=sincos.table_sin(dq+pi/2);
	float cos=sincos.table_cos(dq+pi/2);
	grim->Quads_Draw4V(x+sin*width,y+cos*width,x-sin*width,y-cos*width,
		x2-sin*width,y2-cos*width,x2+sin*width,y2+cos*width);
	grim->Quads_End();
}
