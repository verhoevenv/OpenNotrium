#include "engine.h"
#include <SDL.h>
#include <SDL_image.h>
#include "func.h"
#include <physfs.h>
#include <memory>

Engine::Engine()
:   window(nullptr)
,   glcontext(0)
,   width(640), height(480), bpp(32)
,   fullscreen(false)
,   ignoreMouseMotion(2) // ...
,   clear_a(0), clear_r(0), clear_g(0), clear_b(0)
,   blend_src(grBLEND_SRCALPHA)
,   blend_dst(grBLEND_INVSRCALPHA)
,   freeSlotInList(false)
,   currtexture(0) {

    //perhaps this stuff should go into startframe?
    for(int i=0;i<4;i++){
        vertex_r[i] = 1;
        vertex_g[i] = 1;
        vertex_b[i] = 1;
        vertex_a[i] = 1;
    }
}

void Engine::startFrame(){
    //printf("%d\n",SDL_GetTicks());
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D,0);
    render_target = mainTarget;
    rotate_angle = 0;
    Quads_SetSubset(0,0,1,1);
}

void Engine::System_Start(){
    SDL_Event event;
    bool running = true;
    while(running) {
        startFrame();
        running = (*framefunc)();
        glFlush();
        SDL_GL_SwapWindow(window);

        //mousewheel is a bit weird
        mousestate.lZ = 0;
        mousestate.lX = 0;
        mousestate.lY = 0;

        //erase the clicked keys
        keys_clicked.clear();

        while( SDL_PollEvent( &event ) ) {
            switch( event.type ) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                //key = event.key.keysym.sym;
                keys_clicked.insert(event.key.keysym.sym);
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch(event.button.button){
                case SDL_BUTTON_LEFT:
                    mousestate.btn_left = true;
                    break;
                case SDL_BUTTON_RIGHT:
                    mousestate.btn_right = true;
                    break;
                case SDL_BUTTON_MIDDLE:
                    mousestate.btn_middle = true;
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch(event.button.button){
                case SDL_BUTTON_LEFT:
                    mousestate.btn_left = false;
                    break;
                case SDL_BUTTON_RIGHT:
                    mousestate.btn_right = false;
                    break;
                case SDL_BUTTON_MIDDLE:
                    mousestate.btn_middle = false;
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) { // wheel up
                    mousestate.lZ += 1;
                } else if (event.wheel.y < 0) { // wheel down
                    mousestate.lZ -= 1;
                }
                break;
            case SDL_MOUSEMOTION:
                if(ignoreMouseMotion > 0){
                    ignoreMouseMotion--;
                } else {
                    mousestate.lX = event.motion.xrel;
                    mousestate.lY = event.motion.yrel;
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    if (focusgainedfunc)
                        (*focusgainedfunc)();
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    if (focuslostfunc)
                        (*focuslostfunc)();
                }
                break;
            }
        }
    }
}

void Engine::System_Initiate(const char *argv0){
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 ) {
        exit(1);
    }

    /* Set the minimum requirements for the OpenGL window */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    Uint32 flags = SDL_WINDOW_OPENGL;
    if(fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;

    this->window = SDL_CreateWindow("Notrium",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          width, height,
                          flags);

    if( window == nullptr ) {
        exit(1);
    }

    this->glcontext = SDL_GL_CreateContext(window);

    System_GrabInput();

    PHYSFS_init(argv0);
    PHYSFS_setSaneConfig("monkkonen","notrium",nullptr,0,0); //Perhaps we should allow packages here. Not now.

    setup_opengl();
    startFrame();
}

void Engine::setup_opengl()
{
    //float aspect = (float)width / (float) height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, width, height, 0, -2, 2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glShadeModel(GL_SMOOTH);
    glClearColor(clear_r,clear_g,clear_b,clear_a);
    glDisable(GL_DEPTH_TEST); //since we do Z sorting ourselves, it seems
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    //TODO: probably need to recreate textures here and do other context recreation stuff

    if(Texture_Create("EngineMainTarget",width,height))
        mainTarget = Texture_Get("EngineMainTarget");
}

void Engine::System_SaveScreenshot(const std::string& filename){
    //the main problem here is that textures are stored upside-down
    //to get a proper screenshot, we have to flip the image
    //we can do this ourselves in memory, but why not let opengl work for us...

    //we create a temporary texture that will hold the inverted screenshot
    if(!Texture_Create("EngineScreenshotTempSurface",width,height))
        return;

    //we render our screen to the temporary texture
    int temptext = Texture_Get("EngineScreenshotTempSurface");
    System_SetRenderTarget(temptext);
    Quads_SetSubset(0,1,1,0); //this needs to change
    Texture_Set(mainTarget);
    Quads_Begin();
    Quads_Draw(0,0,width,height);
    Quads_End();

    //we render the temporary target to the screen - inverted
    System_SetRenderTarget(-1);
    Quads_SetSubset(0,0,1,1);
    Texture_Set(temptext);
    Quads_Begin();
    Quads_Draw(0,0,width,height);
    Quads_End();

    //we store the (now inverted) framebuffer
    SDL_Surface *shot = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            0x00FF0000, 0x0000FF00, 0x000000FF, 0);
#else
            0x000000FF, 0x0000FF00, 0x00FF0000, 0);
#endif
    glReadPixels (0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE,
            shot->pixels);
    SDL_SaveBMP ( shot, filename.c_str());
    SDL_FreeSurface (shot);

    //we render the first state of the screen to the screen again
    Quads_SetSubset(0,1,1,0);
    Texture_Set(temptext);
    Quads_Begin();
    Quads_Draw(0,0,width,height);
    Quads_End();

    //and we delete our temporary texture
    Texture_Delete(temptext);
}

MouseState Engine::getMouseState(){
    return MouseState(mousestate);
}

void Engine::ShowCursor(bool state){
    if(state)
        SDL_ShowCursor(SDL_ENABLE);
    else
        SDL_ShowCursor(SDL_DISABLE);
}

bool Engine::Key_Down(Key key){
    // Note: SDL_GetKeyboardState provides scan code indexed state,
    // rather than keycode indexed (hence the conversion)
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    return keys[SDL_GetScancodeFromKey(key)];
}

bool Engine::Key_Click(Key key){
    std::set<Key>::iterator it = keys_clicked.find(key);
    return it != keys_clicked.end();
}

void Engine::System_SetState_Blending(bool state){
    if(state){
        glEnable(GL_BLEND);
        glBlendFunc(blend_src, blend_dst);
    } else {
        glDisable(GL_BLEND);
    }
}

void Engine::System_SetState_BlendSrc(BlendState state){
    blend_src = state;
    glBlendFunc(blend_src, blend_dst);
}

void Engine::System_SetState_BlendDst(BlendState state){
    blend_dst = state;
    glBlendFunc(blend_src, blend_dst);
}

void Engine::System_SetState_FrameFunc(bool (*func)()){
    framefunc = func;
}

void Engine::System_SetState_FocusLostFunc(bool (*func)()){
    focuslostfunc = func;
}

void Engine::System_SetState_FocusGainFunc(bool (*func)()){
    focusgainedfunc = func;
}

void Engine::System_SetState_Windowed(bool state){
    fullscreen = !state;
}

void Engine::System_SetState_ScreenWidth(int w){
    width = w;
}

void Engine::System_SetState_ScreenHeight(int h){
    height = h;
}

void Engine::System_SetState_ScreenBPP(int bits){
    bpp = bits;
}

void Engine::System_SetState_Title(const char* newtitle){
    SDL_SetWindowTitle(this->window, newtitle);
}

void Engine::System_Shutdown(){
    SDL_GL_DeleteContext(glcontext);
    SDL_Quit();
}

void Engine::System_GrabInput(){
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Engine::System_ReleaseInput(){
    SDL_SetRelativeMouseMode(SDL_FALSE);
}

bool Engine::System_SetRenderTarget(int tex_id){
    if(tex_id == -1)
        tex_id = mainTarget;

    //copy the current buffer to the previous render target
    EngineTexture* tex = &textures[render_target];
    glBindTexture( GL_TEXTURE_2D, tex->opengl_id );
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, tex->width, tex->height, 0);
    tex->draw_flipped = true;

    //set up the correct stuff for rendering to the new target
    tex = &textures[tex_id];
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0,0,tex->width,tex->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, tex->width, tex->height, 0, -2, 2);
    glMatrixMode(GL_MODELVIEW);


    //restore state?
    System_SetState_Blending(false);
    Texture_Set(tex_id);
    Quads_SetColor(1,1,1,1);
    Quads_Begin();
    Quads_Draw(0,0,tex->width,tex->height);
    Quads_End();

    render_target = tex_id;
    return true;
}

void Engine::System_ClearScreen(float r, float g, float b, float a){
    clear_r = r;
    clear_g = g;
    clear_b = b;
    clear_a = a;
    glClearColor(clear_r,clear_g,clear_b,clear_a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::Texture_Set(int tex_slot){
    activetexture = tex_slot;
    glBindTexture( GL_TEXTURE_2D, textures[tex_slot].opengl_id );
}


void Engine::Texture_Delete(int tex_slot){
    textures[tex_slot].deleted = true;
    glDeleteTextures(1,&(textures[tex_slot].opengl_id));
    freeSlotInList = true;
}


int Engine::Texture_Get(const std::string& id){
    for(int i=0;i<maxtextures;i++){
        if(textures[i].name == id)
            return i;
    }
    return -1;
}

bool Engine::Texture_Create(const std::string& id, int width, int height){
    createNewTexture(id,width,height);

    // Stored Data
    auto data = std::unique_ptr<GLubyte[]>(new GLubyte[width * height * 4]);

    memset(data.get(), 0, width * height * 4);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data.get());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    return true;
}


bool Engine::Texture_Load(const std::string& id, char *filename){
    SDL_Surface *surface;
    GLenum texture_format;

    surface = IMG_Load(filename);
    if(!surface)
        return false;

    Uint8 nOfColors = surface->format->BytesPerPixel;
    if (nOfColors == 4)     // contains an alpha channel
    {
            if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGBA;
            else
                    texture_format = GL_BGRA;
    } else if (nOfColors == 3)     // no alpha channel
    {
            if (surface->format->Rmask == 0x000000ff)
                    texture_format = GL_RGB;
            else
                    texture_format = GL_BGR;
    } else {
            //image in strange format
            return false;
    }

    createNewTexture(id,surface->w, surface->h);

      glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );
    // assuming the texture was copied to OpenGL, the surface is no longer needed
    SDL_FreeSurface(surface);
    return true;
}

int Engine::createNewTexture(const std::string& name, int width, int height){
    GLuint id;
    glGenTextures( 1, &id );

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    int newtext;
    if(freeSlotInList) {
        bool found = false;
        int i = 0;
        while(!found && i < currtexture){
            if(textures[i].deleted){
                found = true;
                newtext = i;
            }
            i++;
        }
        if(!found){
            newtext = currtexture;
            currtexture++;
            freeSlotInList = false;
        }
    } else {
        newtext = currtexture;
        currtexture++;
    }

    if(newtext >= maxtextures)
        return -1;

    textures[newtext].name = name;
    textures[newtext].opengl_id = id;
    textures[newtext].width = width;
    textures[newtext].height = height;
    textures[newtext].draw_flipped = false;
    textures[newtext].deleted = false;

    return newtext;
}

void Engine::Quads_SetRotation(float angle){
    //glRotate needs to be called after glTranslate to define local rotation, so store it for now.
    rotate_angle = angle * 180 / pi;
}

void Engine::Quads_SetColor(float r, float g, float b, float a){
    for(int i=0;i<4;i++)
        Quads_SetColorVertex(i,r,g,b,a);
}

void Engine::Quads_SetSubset(float tl_u, float tl_v, float br_u, float br_v){
    texcoor_tl_u = tl_u;
    texcoor_tl_v = tl_v;
    texcoor_br_u = br_u;
    texcoor_br_v = br_v;
}

void Engine::Quads_Begin(){
    // nop
}
void Engine::Quads_End(){
    // nop
}

void Engine::Quads_Draw(float x, float y, float width, float height){
    glLoadIdentity();
    glTranslatef(x+width/2.0,y+width/2.0,0);
    glRotatef(rotate_angle,0,0,1);
    glTranslatef(-width/2.0,-width/2.0,0);
    Draw4V(0,0,width,0,width,height,0,height);
}

void Engine::Quads_SetColorVertex(int vertex, float r, float g, float b, float a){
    vertex_r[vertex] = r;
    vertex_g[vertex] = g;
    vertex_b[vertex] = b;
    vertex_a[vertex] = a;
}

void Engine::Quads_Draw4V(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3){
    glLoadIdentity();
    Draw4V(x0, y0, x1, y1, x2, y2, x3, y3);
}

void Engine::Draw4V(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3){
    EngineTexture *tex = &textures[activetexture];

    glBegin(GL_QUADS);
    setglColor(0);
    if(tex->draw_flipped)
        glTexCoord2f(texcoor_tl_u, 1 - texcoor_tl_v);
    else
        glTexCoord2f(texcoor_tl_u, texcoor_tl_v);
    glVertex3f(x0, y0, -1);

    setglColor(3);
    if(tex->draw_flipped)
        glTexCoord2f(texcoor_tl_u, 1 - texcoor_br_v);
    else
        glTexCoord2f(texcoor_tl_u, texcoor_br_v);
    glVertex3f(x3, y3, -1);

    setglColor(2);
    if(tex->draw_flipped)
        glTexCoord2f(texcoor_br_u, 1 - texcoor_br_v);
    else
        glTexCoord2f(texcoor_br_u, texcoor_br_v);
    glVertex3f(x2, y2, -1);

    setglColor(1);
    if(tex->draw_flipped)
        glTexCoord2f(texcoor_br_u, 1 - texcoor_tl_v);
    else
        glTexCoord2f(texcoor_br_u, texcoor_tl_v);
    glVertex3f(x1, y1, -1);
    glEnd();
}

void Engine::setglColor(int index){
    if(vertex_a[index] >= 0)
        glColor4f(vertex_r[index],vertex_g[index],vertex_b[index],vertex_a[index]);
    else
        glColor4f(1,1,1,1);
}

bool Engine::File_Exists(const std::string& filename){
    return (PHYSFS_exists(filename.c_str()) != 0);
}

bool Engine::File_IsDirectory(const std::string& filename){
    return (PHYSFS_isDirectory(filename.c_str()) != 0);
}

std::vector<std::string> Engine::File_ListDirectory(const std::string& dir){
    char **rc = PHYSFS_enumerateFiles(dir.c_str());
    char **i;

    std::vector<std::string> vec;

    for (i = rc; *i != NULL; i++){
        vec.push_back(*i);
    }

    PHYSFS_freeList(rc);

    return vec;
}

long Engine::Time_GetTicks(){
    //TODO: add performancetimer if possible
// 	if (perf_flag)
// 	   QueryPerformanceCounter((LARGE_INTEGER *) &cur_time);
// 	else
// 	   cur_time=GetTickCount();
//
// 				// is there a performance counter available?
// 				LONGLONG perf_cnt;
// 				if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)) {
// 					// yes, timer choice flag
// 					perf_flag=true;
// 					// set scaling factor
// 					time_factor=1.0/perf_cnt;
// 					// read initial time
// 					QueryPerformanceCounter((LARGE_INTEGER *) &last_time);
// 				} else {

    return SDL_GetTicks();
}
