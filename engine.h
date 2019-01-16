#ifndef __ENGINE_H__
#define __ENGINE_H__

//It would be fun if anything implementation-specific (like GL or SDL stuff) would move out of this file

#include <string>
#include <set>
#include <vector>
#include "keys.h"
#include "blendstate.h"

typedef
bool (*ProgramCallback)(void);

class MouseState {
public:
    //position
    int lX;
    int lY;
    int lZ; //mousewheel

    //are the buttons down?
    bool btn_left;
    bool btn_right;
    bool btn_middle;
};

class EngineTexture {
public:
    std::string name;
    GLuint opengl_id;
    int width,height;
    bool draw_flipped; //needed to account for the difference in origin location
    bool deleted;
};

class Engine
{
    SDL_Window* window;
    SDL_GLContext glcontext;
    int width,height,bpp;
    bool fullscreen;
    float clear_r,clear_g,clear_b,clear_a;

    MouseState mousestate;
    int ignoreMouseMotion;

    std::set<Key> keys_clicked;

    BlendState blend_src;
    BlendState blend_dst;

    float rotate_angle;
    float vertex_r[4];
    float vertex_g[4];
    float vertex_b[4];
    float vertex_a[4];

    float texcoor_tl_u, texcoor_tl_v, texcoor_br_u, texcoor_br_v;

    int render_target;
    int activetexture;

    static const int maxtextures = 1000;
    EngineTexture textures[maxtextures];
    bool freeSlotInList;
    int currtexture;
    int mainTarget; //texture for the screen rendertarget

    ProgramCallback framefunc;
    ProgramCallback focuslostfunc;
    ProgramCallback focusgainedfunc;

    void setup_opengl();
    void setglColor(int index);
    void Draw4V(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
    int createNewTexture(const std::string& name, int width, int height);
    void startFrame();
public:
    Engine();

    /// no engine copying
    Engine(const Engine& other) = delete;

    /**
    * Save a screenshot in BMP format, under the given filename.
    * I think it's supposed to be a relative pathname
    */
    void System_SaveScreenshot(const std::string& filename);

    /**
    * Copies relevant stuff into the state. We need to make a copy anyway, so just return the object.
    */
    MouseState getMouseState();
    void ShowCursor(bool state);

    bool Key_Down(Key key);
    bool Key_Click(Key key);

    void System_SetState_Blending(bool state);
    void System_SetState_BlendSrc(BlendState state);
    void System_SetState_BlendDst(BlendState state);
    void System_SetState_FrameFunc(bool (*func)());
    void System_SetState_FocusLostFunc(bool (*func)());
    void System_SetState_FocusGainFunc(bool (*func)());
    void System_SetState_Windowed(bool state);
    void System_SetState_ScreenWidth(int w);
    void System_SetState_ScreenHeight(int h);
    void System_SetState_ScreenBPP(int bpp);
    void System_SetState_Title(const char* title);

    void System_GrabInput();
    void System_ReleaseInput();

    /**
    * Initialises the system with the given states, starts engine, etc. Does not enter the main loop yet.
    * The argv[0] is required on some systems to know the path to the current directory.
    */
    void System_Initiate(const char *argv0);

    /**
    * Enters main loop. Return false from the frame function to exit this loop.
    */
    void System_Start();

    /**
    * Restores context to what it was before we initiated.
    */
    void System_Shutdown();

    /**
    * Sets active render target. This can be screen (value -1) or a texture (texture handle integer).
    */
    bool System_SetRenderTarget(int tex_id = -1);

    /**
    * Clears current render target with given color. Render target can be the screen or a texture. Use System_SetRenderTarget method to set active render target.
    */
    void System_ClearScreen(float r, float g, float b, float a);


    /**
    * Sets texture active. All pixels of quads rendered between Quads_Begin() and Quads_End() are fetched from this texture.
    */
    void Texture_Set(int tex_slot);

    /**
    * Deletes a texture in given texture slot (texture handle).
    */
    void Texture_Delete(int tex_slot);

    /**
    * Gets texture handle (texture slot) of texture with given id string, or -1 if id string doesn't match any texture.
    * This uses linear time in the amount of textures created (sicko interface).
    */
    int Texture_Get(const std::string& id);

    /**
    * Creates a render target texture with given id string, width and height.
    */
    bool Texture_Create(const std::string& id, int width, int height);

    /**
    * Loads a texture either from a "normal file" from work directory. False is returned if file wasn't found or it was wrong format or corrupted.
    * Supported image formats are PNG, BMP, JPG, a lot more (not TIFF though)
    */
    bool Texture_Load(const std::string& id, char *filename);

    /**
    * Sets quad rotation around its center, in radians
    */
    void Quads_SetRotation(float angle);

    void Quads_SetColor(float r, float g, float b, float a);

    /**
    * Sets quad texture coordinates in active texture. Top left (tl) of the texture is 0,0 and bottom right (br) is (1,1).
    * Example: To set upper half of a texture as subset: Quads_SetSubset(0,0, 1, 0.5f);
    */
    void Quads_SetSubset(float tl_u, float tl_v, float br_u, float br_v);

    void Quads_Begin();
    void Quads_End();

    void Quads_Draw(float x, float y, float width, float height);
    void Quads_SetColorVertex(int vertex, float r, float g, float b, float a);
    void Quads_Draw4V(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);
    //void Quads_RenderText(float x, float y, char *text);

    //TODO: other file stuff (reading, writing, ...)
    //bool File_Exists_ReadDir(std::string filename);
    bool File_Exists(const std::string& filename);
    bool File_IsDirectory(const std::string& filename);
    std::vector<std::string> File_ListDirectory(const std::string& dir);

    /**
    * Returns the amount of milliseconds the program has been running. Might wrap around after ~ 50 or 25 days.
    */
    long Time_GetTicks();
};

#endif
