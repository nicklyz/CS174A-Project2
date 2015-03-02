﻿// anim.cpp version 5.0 -- Template code for drawing an articulated figure.  CS 174A.
#include "../CS174a template/Utilities.h"
#include "../CS174a template/Shapes.h"

std::stack<mat4> mvstack;

int g_width = 800, g_height = 800 ;
float zoom = 1 ;

int animate = 0, recording = 0, basis_to_display = -1;
double TIME = 0;

const int RAISE_ARM = 1;
const int LOWER_ARM = 2;
const int SHOOT_ARM = 3;
const int RUN_ARM   = 4;

const int BEND_LEG  = 1;
const int RUN_LEG   = 2;

const unsigned X = 0, Y = 1, Z = 2;

vec4 eye( 0, 0, 15, 1), ref( 0, 0, 0, 1 ), up( 0, 1, 0, 0 );	// The eye point and look-at point.

mat4	orientation, model_view;
ShapeData cubeData, sphereData, coneData, cylData;				// Structs that hold the Vertex Array Object index and number of vertices of each shape.
GLuint	texture_cube, texture_basketball, texture_jersey;
GLint   uModelView, uProjection, uView,
		uAmbient, uDiffuse, uSpecular, uLightPos, uShininess,
		uTex, uEnableTex;
vec4 unRotatedPoint;

void init()
{
#ifdef EMSCRIPTEN
    GLuint program = LoadShaders( "vshader.glsl", "fshader.glsl" );								// Load shaders and use the resulting shader program
    TgaImage coolImage ("challenge.tga");    
    TgaImage basketballImage("earth.tga");
    TgaImage jerseyImage("jersey.tga");

#else
	GLuint program = LoadShaders( "../my code/vshader.glsl", "../my code/fshader.glsl" );		// Load shaders and use the resulting shader program
    TgaImage coolImage ("../my code/challenge.tga");    
    TgaImage basketballImage("../my code/basketball.tga");
    TgaImage jerseyImage("../my code/jersey.tga");
#endif
    glUseProgram(program);

	generateCube(program, &cubeData);		// Generate vertex arrays for geometric shapes
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView		= glGetUniformLocation( program, "View"       );
    uAmbient	= glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse	= glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular	= glGetUniformLocation( program, "SpecularProduct" );
    uLightPos	= glGetUniformLocation( program, "LightPosition"   );
    uShininess	= glGetUniformLocation( program, "Shininess"       );
    uTex		= glGetUniformLocation( program, "Tex"             );
    uEnableTex	= glGetUniformLocation( program, "EnableTex"       );

    glUniform4f( uAmbient,    0.2,  0.2,  0.2, 1 );
    glUniform4f( uDiffuse,    0.6,  0.6,  0.6, 1 );
    glUniform4f( uSpecular,   0.2,  0.2,  0.2, 1 );
    glUniform4f( uLightPos,  15.0, 15.0, 30.0, 0 );
    glUniform1f( uShininess, 100);

    glEnable(GL_DEPTH_TEST);
    
    glGenTextures( 1, &texture_cube );
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, coolImage.width, coolImage.height, 0,
                 (coolImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, coolImage.data );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    
    glGenTextures( 1, &texture_basketball );
    glBindTexture( GL_TEXTURE_2D, texture_basketball );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, basketballImage.width, basketballImage.height, 0,
                 (basketballImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, basketballImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    glGenTextures( 1, &texture_jersey );
    glBindTexture( GL_TEXTURE_2D, texture_jersey );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, jerseyImage.width, jerseyImage.height, 0,
                 (jerseyImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, jerseyImage.data );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    glUniform1i( uTex, 0);	// Set texture sampler variable to texture unit 0
	
	glEnable(GL_DEPTH_TEST);
}

struct color{ color( float r, float g, float b) : r(r), g(g), b(b) {} float r, g, b;};
std::stack<color> colors;
void set_color(float r, float g, float b)
{
	colors.push(color(r, g, b));

	float ambient  = 0.2, diffuse  = 0.6, specular = 0.2;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1 );
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1 );
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1 );
}

int mouseButton = -1, prevZoomCoord = 0 ;
vec2 anchor;
void myPassiveMotionCallBack(int x, int y) {	anchor = vec2( 2. * x / g_width - 1, -2. * y / g_height + 1 ); }

void myMouseCallBack(int button, int state, int x, int y)	// start or end mouse interaction
{
    mouseButton = button;
   
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
        mouseButton = -1 ;
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
        prevZoomCoord = -2. * y / g_height + 1;

    glutPostRedisplay() ;
}

void myMotionCallBack(int x, int y)
{
	vec2 arcball_coords( 2. * x / g_width - 1, -2. * y / g_height + 1 );
	 
    if( mouseButton == GLUT_LEFT_BUTTON )
    {
	   orientation = RotateX( -10 * (arcball_coords.y - anchor.y) ) * orientation;
	   orientation = RotateY(  10 * (arcball_coords.x - anchor.x) ) * orientation;
    }
	
	if( mouseButton == GLUT_RIGHT_BUTTON )
		zoom *= 1 + .1 * (arcball_coords.y - anchor.y);
    glutPostRedisplay() ;
}

void idleCallBack(void)
{
    if( !animate ) return;
	double prev_time = TIME;
    TIME = TM.GetElapsedTime() ;
	if( prev_time == 0 ) TM.Reset();
    glutPostRedisplay() ;
}

void drawCylinder()	//render a solid cylinder oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
    glBindTexture( GL_TEXTURE_2D, texture_jersey);
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
    glUniform1i( uEnableTex, 0);
}

void drawCone()	//render a solid cone oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}

void drawCube()		// draw a cube with dimensions 1,1,1 centered around the origin.
{
//	glBindTexture( GL_TEXTURE_2D, texture_cube );
//    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
//    glUniform1i( uEnableTex, 0 );
}

void drawSphere()	// draw a sphere with radius 1 centered around the origin.
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
}

void drawBasketball() {
    glBindTexture( GL_TEXTURE_2D, texture_basketball);
    glUniform1i( uEnableTex, 1);
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
    glUniform1i( uEnableTex, 0 );
}
int basis_id = 0;
void drawOneAxis()
{
	mat4 origin = model_view;
	model_view *= Translate	( 0, 0, 4 );
	model_view *= Scale(.25) * Scale( 1, 1, -1 );
	drawCone();
	model_view = origin;
	model_view *= Translate	( 1,  1, .5 );
	model_view *= Scale		( .1, .1, 1 );
	drawCube();
	model_view = origin;
	model_view *= Translate	( 1, 0, .5 );
	model_view *= Scale		( .1, .1, 1 );
	drawCube();
	model_view = origin;
	model_view *= Translate	( 0,  1, .5 );
	model_view *= Scale		( .1, .1, 1 );
	drawCube();
	model_view = origin;
	model_view *= Translate	( 0,  0, 2 );
	model_view *= Scale(.1) * Scale(   1, 1, 20);
    drawCylinder();	
	model_view = origin;
}

void drawAxes(int selected)
{
	if( basis_to_display != selected ) 
		return;
	mat4 given_basis = model_view;
	model_view *= Scale		(.25);
	drawSphere();
	model_view = given_basis;
	set_color( 0, 0, 1 );
	drawOneAxis();
	model_view *= RotateX	(-90);
	model_view *= Scale		(1, -1, 1);
	set_color( 1, 1, 1);
	drawOneAxis();
	model_view = given_basis;
	model_view *= RotateY	(90);
	model_view *= Scale		(-1, 1, 1);
	set_color( 1, 0, 0 );
	drawOneAxis();
	model_view = given_basis;
	
	colors.pop();
	colors.pop();
	colors.pop();
	set_color( colors.top().r, colors.top().g, colors.top().b );
}

void drawGround(){
	mvstack.push(model_view);
    set_color( .0, .8, .0 );
    model_view *= Translate	(0, -10, 0);									drawAxes(basis_id++);
    model_view *= Scale		(100, 0.5, 100);								drawAxes(basis_id++);
    drawCube();
	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
}

void drawShapes()
{
	mvstack.push(model_view);

    model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
    model_view *= Scale		( 3, 3, 3 );									drawAxes(basis_id++);
    set_color( .8, .0, .8 );
    drawCube();

    model_view *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);
    model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
    set_color( 0, 1, 0 );
    drawCone();

    model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
    set_color( 1, 1, 0 );
    drawCylinder();

	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
	
    model_view *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);

	drawGround();
}

void drawBall()
{
    // This function defines a sequence of events of the ball
    // 1. in player's hand
    // 2. first shot
    // 3. in player's hand
    // 4. second shot
    // 5. dunk
    
    float height = 4.0f;
    float progress = 0.0f;
    
    float firstWaitEnd = 5.0f;
    float firstShootEnd = 8.0f;
    float secondWaitEnd = 10.f;
    float secondShootEnd = 13.0f;
    float runEnd = 15.0f;
    float jumpEnd = 18.0f;
    
    drawBasketball();
}

void drawHead()
{
    set_color(1.0f, 0, 0);
    mvstack.push(model_view);
    model_view *= Translate(0, 3, 0);
    drawSphere();
    
    model_view = mvstack.top(); mvstack.pop();
}

void drawManArm(float angle, float z_angle)    // draw a single arm
{
    mvstack.push(model_view);
    // upper arm
    model_view *= Translate(0, -1, 0);
    model_view *= Scale(0.1, 2.0, 0.1);
    drawCube(); // upper arm
    model_view *= Scale(10, 0.5, 10);
    model_view *= Translate(0, -1, 0);
    model_view *= RotateZ(z_angle);
    model_view *= RotateX(-angle);
   
    // lower arm
    model_view *= Translate(0, -1, 0);
    model_view *= Scale(0.1, 2.0, 0.1);
    drawCube();
    model_view = mvstack.top(); mvstack.pop();
}

void drawManBodyArms(int action, float progress, bool withBall)
{
    // action defines the movement of the arm:
//    const int RAISE_ARM = 1;
//    const int LOWER_ARM = 2;
//    const int SHOOT_ARM = 3;
//    const int RUN_ARM = 4;
    
    set_color(0.8f, 1.0f, 1.0f);
    
    // Right upper arm
    mvstack.push(model_view);
    model_view *= Translate(0, 2, 0);
    
    switch (action) {
        case RAISE_ARM:
            model_view *= RotateZ(-30);
            model_view *= RotateX(-80 * progress);
            drawManArm(50, 30);
            
            model_view = mvstack.top();
            model_view *= Translate(0, 2, 0);
            model_view *= RotateZ(30);
            model_view *= RotateX(-80 * progress);
            drawManArm(50, -30);
            if (withBall) {
                model_view = mvstack.top();
                model_view *= Translate(0, 2, 0);
                model_view *= RotateX(-80 * progress);
                model_view *= Translate(0, -2, 0);
                model_view *= RotateX(-50);
                model_view *= Translate(0, -2, 0);
                //model_view *= Scale(0.8, 0.8, 0.8);
                drawBall();
            }
            break;
        case LOWER_ARM:
            break;
        case SHOOT_ARM:
            break;
        case RUN_ARM:
            break;
        default:        // arms are still
            model_view *= RotateZ(-30);
            drawManArm(50, 30);
            model_view *= RotateZ(60);
            drawManArm(50, -30);
            if (withBall) {
                model_view = mvstack.top();
                model_view *= Translate(0, 2, 0);
                model_view *= Translate(0, -2, 0);
                model_view *= RotateX(-50);
                model_view *= Translate(0, -2, 0);
                //model_view *= Scale(0.8, 0.8, 0.8);
                drawBall();
            }
            break;
    }
    
    model_view = mvstack.top(); mvstack.pop();
}

void drawManLeg(float angle, float z_angle)
{
    mvstack.push(model_view);
    // upper leg
    model_view *= Translate(0, -1, 0);
    model_view *= Scale(0.1, 2.0, 0.1);
    drawCube(); // upper arm
    model_view *= Scale(10, 0.5, 10);
    model_view *= Translate(0, -1, 0);
    model_view *= RotateX(angle);   // angle is positive, different than arm
    model_view *= RotateZ(z_angle);
    // lower leg
    model_view *= Translate(0, -1, 0);
    model_view *= Scale(0.1, 2.0, 0.1);
    drawCube();
    model_view = mvstack.top(); mvstack.pop();
}

void drawManBodyLegs(int action, float progress)
{
    // action defines the movement of the leg:
    //    const int BEND_LEG = 1;
    //    const int RUN_LEG = 2;
    set_color(0.8f, 1.0f, 1.0f);
    
    mvstack.push(model_view);
    model_view *= Translate(0, -2, 0);
    
    switch (action) {
        case BEND_LEG:
            model_view *= RotateX(-45*progress);
            model_view *= RotateZ(-30);
            drawManLeg(20+45*progress, 30);
            model_view *= RotateZ(60);
            drawManLeg(20+45*progress, -30);
            break;
        case RUN_LEG:
            
            break;
        default:        // arms are still
            model_view *= RotateX(-20);
            model_view *= RotateZ(-30);
            drawManLeg(20, 30);
            model_view *= RotateZ(60);
            drawManLeg(20, -30);
            break;
    }
    
    model_view = mvstack.top(); mvstack.pop();
}

void drawManBody(float height, int action, float progress)
{
    set_color(0.8f, 1.0f, 1.0f);
    model_view *= Translate(0, height-2, 0);
    model_view *= Scale(0.1, 4, 0.1);
    drawCube();
    model_view *= Scale(10, 0.25, 10);
    // model_view would be at the center of the body
}

void drawManStage(float height, int stage, float progress)
{
    mvstack.push(model_view);
    set_color(0.0f, 0.0f, 0.0f);
    //    model_view *= Translate(0.0f, height/2.0f, 0.0f);
    
    switch (stage)
    {
        case 1: // first wait
            drawManBody(height, 0, progress);
            drawManBodyArms(0, progress, true);
            drawManBodyLegs(0, progress);
            drawHead();
            break;
        case 2: // first shoot
            drawManBody(height, 0, progress);
            drawManBodyArms(RAISE_ARM, progress, true);
            drawManBodyLegs(BEND_LEG, progress);
            drawHead();
            break;
//        case 3:
//            break;
//        case 4:
//            break;
//        case 5:
//            break;
        default:
            drawManBody(height, 0, progress);
            drawManBodyArms(0, progress, true);
            drawManBodyLegs(0, progress);
            drawHead();
            break;
    }
    
    model_view = mvstack.top();  mvstack.pop();
}

void drawMan(float ground_level)
{
        // This function defines a sequence of events of the basketball player
    // 1. waits until time = 5;
    // 2. shoots a basketball;
    // 3. runs towrad the ball;
    // 4. waits until time = 10;
    // 5. reset, and shoots one more time;
    // 6. runs toward the ball;
    // 7. jump;
    // 8. wait in the air;
    // 9. catches the ball and dunk
    // 10. fall to ground
    
    set_color(1.0f, 0.0f, 0.0f);
    
    model_view *= Translate(0, ground_level, 0);
    float height = 7.6f;
    float progress = 0.0f;
    
    float firstWaitEnd = 5.0f;
    float firstShootEnd = 6.5f;
    float firstRunEnd = 9.0f;
    float secondWaitEnd = 11.0f;
    float secondShootEnd = 12.5f;
    float secondRunEnd = 15.0f;
    float jumpEnd = 18.0f;
    float thirdWaitEnd = 20.0f;
    float dunkEnd = 21.5f;
    float fallEnd = 23.0f;
    
    
    float init_z = 10.0f;
    float init_x = 10.0f;
    
    // draw man
//    drawManBody(height, 0, progress);
//    drawHead();
//    drawManBodyArms(0, progress, false);
    
    int stage = 0;
    
    if (TIME <= 0.0f){
        progress = 0;
        model_view *= Translate(init_x, 0, init_z);
        stage = 0;
    }
    
    else if (TIME <= firstWaitEnd)
    {
        progress = TIME/firstWaitEnd;
        stage = 1;
    }
    else if (TIME <= firstShootEnd)
    {
        progress = (TIME - firstWaitEnd)/(firstShootEnd - firstWaitEnd);
        stage = 2;
    }
    else if (TIME <= firstRunEnd)
    {
        progress = (TIME - firstShootEnd)/(firstRunEnd - firstShootEnd);
        stage = 3;
    }
    
    else if (TIME <= secondWaitEnd)
    {
        progress = (TIME - firstRunEnd)/(secondWaitEnd - firstRunEnd);
        stage = 4;
    }
    else if (TIME <= secondShootEnd)
    {
        progress = (TIME - secondWaitEnd)/(secondShootEnd - secondWaitEnd);
        stage = 5;
    }
    else if (TIME <= secondRunEnd)
    {
        progress = (TIME - secondShootEnd)/(secondRunEnd - secondShootEnd);
        stage = 6;
    }
    else if (TIME <= jumpEnd)
    {
        progress = (TIME - secondRunEnd)/(jumpEnd - secondRunEnd);
        stage = 7;
    }
    else if (TIME <= thirdWaitEnd)
    {
        progress = (TIME - jumpEnd)/(thirdWaitEnd - jumpEnd);
        stage = 8;
    }
    else if (TIME <= dunkEnd)
    {
        progress = (TIME - thirdWaitEnd)/(dunkEnd - thirdWaitEnd);
        stage = 9;
    }
    else if (TIME <= fallEnd)
    {
        progress = (TIME - dunkEnd)/(fallEnd - dunkEnd);
        stage = 10;
    }
    
    else // finish animation
    {
        stage = 11;
    }
    drawManStage(height, stage, progress);
    
}



void drawPlanets()
{
    set_color( .8, .0, .0 );	//model sun
    mvstack.push(model_view);
    model_view *= Scale(3);													drawAxes(basis_id++);
    drawSphere();
    model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
    
    set_color( .0, .0, .8 );	//model earth
    model_view *= RotateY	( 10*TIME );									drawAxes(basis_id++);
    model_view *= Translate	( 15, 5*sin( 30*DegreesToRadians*TIME ), 0 );	drawAxes(basis_id++);
    mvstack.push(model_view);
    model_view *= RotateY( 300*TIME );										drawAxes(basis_id++);
    drawCube();
    model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
    
    set_color( .8, .0, .8 );	//model moon
    model_view *= RotateY	( 30*TIME );									drawAxes(basis_id++);
    model_view *= Translate	( 2, 0, 0);										drawAxes(basis_id++);
    model_view *= Scale(0.2);												drawAxes(basis_id++);
    drawCylinder();
	
}

void drawMidterm()
{
	mvstack.push(model_view);
	mvstack.push(model_view);
	model_view *= Translate	( -1, 0, 0 );									drawAxes(basis_id++);
	model_view *= Scale		( 2, 1, 1 );									drawAxes(basis_id++);
	drawCube();
	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
	
	model_view *= Scale		( 2, 1, 1 );									drawAxes(basis_id++);
	model_view *= Translate	( 1, 0, 0 );									drawAxes(basis_id++);
	drawCube();

	
	model_view *= Translate	( 0, 2, 0 );									drawAxes(basis_id++);
	model_view *= RotateZ	( 90 + 360 * TIME );							drawAxes(basis_id++);
	drawCube();
	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
}

double PREV_TIME = 0;
int frameNumber = 0;
double average;
const float GROUND_LEVEL = -10.0;

void display(void) {

    if (TIME != PREV_TIME) {
    double newMeasurement = 1.0 / (TIME - PREV_TIME);
    
    const float decay_period = 10;
    const float a = 2. / ( decay_period - 1.);
    average = a * newMeasurement+ (1 - a) * average;   // smoothing of measuring frame rate
    
    if (frameNumber % 10 == 0) {
    std::cout << "Frame rate: " ;
    std::cout << average << std::endl;
    }
    
        PREV_TIME = TIME;
        frameNumber ++;
    }
    animate = 1;
	basis_id = 0;
    glClearColor( .1, .1, .2, 1 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	set_color( .6, .6, .6 );
	
    vec4 rotatedEye, rotatedRef;
//    
//    float rotationBeginTime = 1;
//    float timeToRotate = 7;
//    float rotationSceneTime = TIME - rotationBeginTime;
//    
//    if( rotationSceneTime > 0 && rotationSceneTime < timeToRotate ) { // 360 scene
//        //eye = RotateY(360/timeToRotate * rotationSceneTime) * unRotatedPoint;
//        rotatedEye = RotateY(360/timeToRotate * rotationSceneTime) * eye;
//        //rotatedEye = Translate(0, 0, -TIME) * eye;
//        model_view = LookAt(rotatedEye, ref, up);
//    }
//    else
//        model_view = LookAt(eye, ref, up);
//    
//    if( 0 < TIME && TIME < rotationBeginTime )
//        rotatedEye = eye;
    
	model_view = LookAt( eye, ref, up );

	model_view *= orientation;
    model_view *= Scale(zoom);												drawAxes(basis_id++);

	//drawMidterm();
    //model_view *= Translate( 0, -6, 0 );									drawAxes(basis_id++);

    
    
    drawGround();
    
    drawMan(GROUND_LEVEL);
    
    glutSwapBuffers();
    
}

void myReshape(int w, int h)	// Handles window sizing and resizing.
{    
    mat4 projection = Perspective( 50, (float)w/h, 1, 1000 );
    glUniformMatrix4fv( uProjection, 1, GL_FALSE, transpose(projection) );
	glViewport(0, 0, g_width = w, g_height = h);	
}		

void instructions() {	 std::cout <<	"Press:"									<< '\n' <<
										"  r to restore the original view."			<< '\n' <<
										"  0 to restore the original state."		<< '\n' <<
										"  a to toggle the animation."				<< '\n' <<
										"  b to show the next basis's axes."		<< '\n' <<
										"  B to show the previous basis's axes."	<< '\n' <<
										"  q to quit."								<< '\n';	}

void myKey(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':   case 27:				// 27 = esc key
            exit(0); 
		case 'b':
			std::cout << "Basis: " << ++basis_to_display << '\n';
			break;
		case 'B':
			std::cout << "Basis: " << --basis_to_display << '\n';
			break;
        case 'a':							// toggle animation           		
            if(animate) std::cout << "Elapsed time " << TIME << '\n';
            animate = 1 - animate ; 
            break ;
		case '0':							// Add code to reset your object here.
			TIME = 0;	TM.Reset() ;											
        case 'r':
			orientation = mat4();			
            break ;
    }
    glutPostRedisplay() ;
}

int main() 
{
	char title[] = "Title";
	int argcount = 1;	 char* title_ptr = title;
	glutInit(&argcount,		 &title_ptr);
	glutInitWindowPosition (230, 70);
	glutInitWindowSize     (g_width, g_height);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow(title);
	#if !defined(__APPLE__) && !defined(EMSCRIPTEN)
		glewExperimental = GL_TRUE;
		glewInit();
	#endif
    std::cout << "GL version " << glGetString(GL_VERSION) << '\n';
	instructions();
	init();

	glutDisplayFunc(display);
    glutIdleFunc(idleCallBack) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCallBack) ;
    glutMotionFunc(myMotionCallBack) ;
    glutPassiveMotionFunc(myPassiveMotionCallBack) ;

	glutMainLoop();
}