// anim.cpp version 5.0 -- Template code for drawing an articulated figure.  CS 174A.
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
const int EXT_LEG   = 3;

const int BEND_BODY = 1;
const int EXT_BODY  = 2;
const int RUN_BODY  = 3;
const int JUMP_BODY = 4;

const int DISTANCE_RUN  = 35;
const int DISTANCE_JUMP = 10;
const int HEIGHT_JUMP   = 20;
const int HEIGHT_BALL1  = 10;
const int HEIGHT_BALL2  = 25;
const int HEIGHT_BASKET = 10;

const unsigned X = 0, Y = 1, Z = 2;

vec4 eye(70, 0, 0, 1), ref( 0, 0, 0, 1 ), up( 0, 1, 0, 0 );	// The eye point and look-at point.

mat4	orientation, model_view;
ShapeData cubeData, sphereData, coneData, cylData, hoopData;				// Structs that hold the Vertex Array Object index and number of vertices of each shape.
GLuint	texture_cube, texture_basketball, texture_jersey, texture_dunk;
GLint   uModelView, uProjection, uView,
		uAmbient, uDiffuse, uSpecular, uLightPos, uShininess,
		uTex, uEnableTex;
vec4 unRotatedPoint;

void init()
{
#ifdef EMSCRIPTEN
    GLuint program = LoadShaders( "vshader.glsl", "fshader.glsl" );								// Load shaders and use the resulting shader program
    TgaImage courtImage ("UCLA.tga");
    TgaImage basketballImage("earth.tga");
    TgaImage jerseyImage("jersey.tga");
    TgaImage dunkImage("spritedunk.tga");

#else
	GLuint program = LoadShaders( "../my code/vshader.glsl", "../my code/fshader.glsl" );		// Load shaders and use the resulting shader program
    TgaImage courtImage ("../my code/UCLA.tga");
    TgaImage basketballImage("../my code/basketball.tga");
    TgaImage jerseyImage("../my code/jersey.tga");
    TgaImage dunkImage("../my code/spritedunk.tga");
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
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, courtImage.width, courtImage.height, 0,
                 (courtImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, courtImage.data );
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
    
    glGenTextures( 1, &texture_dunk );
    glBindTexture( GL_TEXTURE_2D, texture_dunk );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, dunkImage.width, dunkImage.height, 0,
                 (dunkImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, dunkImage.data );
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

void drawHoop()
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( hoopData.vao );
    glDrawArrays( GL_TRIANGLES, 0, hoopData.numVertices );
}
void drawTextCube() // draw a cube with dimension 1,1,1 with texture
{
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
}

void drawDunk() // draw a cube with dimension 1,1,1 with texture
{
    glBindTexture( GL_TEXTURE_2D, texture_dunk );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(model_view) );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
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

void drawGround(float level)
{
    mvstack.push(model_view);
    set_color( .0, .8, .0 );
    model_view *= Translate	(0, level, 0);									drawAxes(basis_id++);
    model_view *= Scale	(284/2, 0.2, 423/2);								drawAxes(basis_id++);
    drawTextCube();
    model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
}

//void drawShapes()
//{
//	mvstack.push(model_view);
//    model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
//    model_view *= Scale		( 3, 3, 3 );									drawAxes(basis_id++);
//    set_color( .8, .0, .8 );
//    drawCube();
//
//    model_view *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);
//    model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
//    set_color( 0, 1, 0 );
//    drawCone();
//
//    model_view *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
//    set_color( 1, 1, 0 );
//    drawCylinder();
//
//	model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
//	
//    model_view *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);
//
//	drawGround();
//}

void drawBallStage(float height, int stage, float progress)
{
    mvstack.push(model_view);
    model_view *= Translate(0, height, 0);
    // This function defines a sequence of events of the ball
    switch (stage) {
        case 1:
        case 2:
            break;
        case 3: // run toward the basket
            // from player's hand, fly toward basket
            // inital position
            model_view *= Translate(0, HEIGHT_BALL1 - HEIGHT_BALL1 * 4 * (progress - 0.5)*(progress - 0.5), 40 * progress);
            model_view *= RotateX(-120 );
            model_view *= Translate(0, -4, 0);
            model_view *= RotateX(-30*progress);
            drawBasketball();
            break;
        case 6:
            model_view *= Translate(0, HEIGHT_BALL2 * sin(progress*95*DegreesToRadians), DISTANCE_RUN * progress);
            model_view *= RotateX(-120 );
            model_view *= Translate(0, -4, 0);
            model_view *= RotateX(-30*progress);
            drawBasketball();
            break;
        case 7:
            model_view *= Translate(0, HEIGHT_BALL2 * sin((95 + 35*progress)*DegreesToRadians), DISTANCE_RUN + DISTANCE_JUMP * progress);
            model_view *= RotateX(-120 );
            model_view *= Translate(0, -4, 0);
            model_view *= RotateX(-30*progress);
            drawBasketball();
            break;
        default:
            break;
    }
    model_view = mvstack.top(); mvstack.pop();
}

void drawHead()
{
    set_color(1.0f, 0, 0);
    mvstack.push(model_view);
    model_view *= Translate(0, 2.8, 0);
    model_view *= Scale(0.8, 0.8, 0.8);
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
    
    set_color(1.0f, 1.0f, 1.0f);
    
    // Right upper arm
    mvstack.push(model_view);
    model_view *= Translate(0, 2, 0);
    
    switch (action) {
        case RAISE_ARM:
            model_view *= RotateX(-100 * progress);
            model_view *= RotateZ(-30);
            drawManArm(50, 30);
            
            model_view = mvstack.top();
            model_view *= Translate(0, 2, 0);
            model_view *= RotateX(-100 * progress);
            model_view *= RotateZ(30);
            drawManArm(50, -30);
            if (withBall) {
                model_view = mvstack.top();
                model_view *= Translate(0, 2, 0);
                model_view *= RotateX(-100 * progress);
                model_view *= Translate(0, -2, 0);
                model_view *= RotateX(-50);
                model_view *= Translate(0, -2, 0);
                //model_view *= Scale(0.8, 0.8, 0.8);
                drawBasketball();
            }
            break;
        case LOWER_ARM:
            model_view *= RotateX(-120 + 120 * progress);
            model_view *= RotateZ(-30);
            drawManArm(50 * progress, 30);
            
            model_view = mvstack.top();
            model_view *= Translate(0, 2, 0);
            model_view *= RotateX(-120 + 120 * progress);
            model_view *= RotateZ(30);
            drawManArm(50 * progress, -30);
            if (withBall) {
                model_view = mvstack.top();
                model_view *= Translate(0, 2, 0);
                model_view *= RotateX(-120 + 120 * progress);
                model_view *= Translate(0, -2, 0);
                model_view *= RotateX(-50 * progress);
                model_view *= Translate(0, -2, 0);
                //model_view *= Scale(0.8, 0.8, 0.8);
                drawBasketball();
            }
            break;
        case SHOOT_ARM:
            model_view *= RotateX(-100 - 20 * progress);
            model_view *= RotateZ(-30);
            drawManArm(50 - 50 * progress, 30);
            
            model_view = mvstack.top();
            model_view *= Translate(0, 2, 0);
            model_view *= RotateX(-100 - 20 * progress);
            model_view *= RotateZ(30);
            drawManArm(50 - 50 * progress, -30);
            if (withBall) {
                model_view = mvstack.top();
                model_view *= Translate(0, 2, 0);
                model_view *= RotateX(-100 - 20 * progress);
                model_view *= Translate(0, -2, 0);
                model_view *= RotateX(50 * progress - 50);
                model_view *= Translate(0, -2, 0);
                //model_view *= Scale(0.8, 0.8, 0.8);
                drawBasketball();
            }
            break;
        case RUN_ARM:
            model_view *= RotateX(10 - 60 * cos(progress*360*2*DegreesToRadians));
            model_view *= RotateZ(-30);
            drawManArm(50, 30);
            
            model_view = mvstack.top();
            model_view *= Translate(0, 2, 0);
            model_view *= RotateX(10 - 60 * cos(progress*360*2*DegreesToRadians + 180*DegreesToRadians));
            model_view *= RotateZ(30);
            drawManArm(50, -30);
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
                drawBasketball();
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
    drawCube();
    model_view *= Scale(10, 0.5, 10);
    model_view *= Translate(0, -1, 0);
    model_view *= RotateZ(z_angle);
    model_view *= RotateX(angle);   // angle is positive, different than arm
    
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
    set_color(1.0f, 1.0f, 1.0f);
    
    mvstack.push(model_view);
    model_view *= Translate(0, -2, 0);
    
    switch (action) {
        case BEND_LEG:
            model_view *= RotateX(-10 - 35*progress);
            model_view *= RotateZ(-30);
            drawManLeg(20+70*progress, 30);
            model_view *= RotateZ(60);
            drawManLeg(20+70*progress, -30);
            break;
        case RUN_LEG:
            model_view *= RotateX(-10 - 60 * cos(progress*360*2*DegreesToRadians + 180*DegreesToRadians));
            model_view *= RotateZ(-30);
            drawManLeg(50, 30);
            
            model_view = mvstack.top();
            model_view *= Translate(0, -2, 0);
            model_view *= RotateX(-10 - 60 * cos(progress*360*2*DegreesToRadians));
            model_view *= RotateZ(30);
            drawManLeg(20, -30);
            break;
        case EXT_LEG:
            model_view *= RotateX(-45 + 35*progress);
            model_view *= RotateZ(-30);
            drawManLeg(90-70*progress, 30);
            model_view *= RotateZ(60);
            drawManLeg(90-70*progress, -30);
            break;
        default:        // arms are still
            model_view *= RotateX(-10);
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
    set_color(1.0f, 1.0f, 1.0f);
    model_view *= Translate(0, height-2, 0);
    switch (action) {
        case BEND_BODY:
            model_view *= Translate(0, 4 * cos((10 + 35 * progress)*DegreesToRadians) - 3.99, 0);
            break;
        case EXT_BODY:
            model_view *= Translate(0, 4 * cos((45 - 35 * progress)*DegreesToRadians) - 3.99, 0);
            break;
        case RUN_BODY:
            model_view *= Translate(0, 0, DISTANCE_RUN * progress);
            break;
        case JUMP_BODY:
            model_view *= Translate(0, HEIGHT_JUMP * progress, DISTANCE_RUN + DISTANCE_JUMP * progress);
            break;
        default:
            break;
    }
    model_view *= Scale(0.1, 4, 0.1);
    drawCube();
    model_view *= Scale(10, 0.25, 10);
    // model_view would be at the center of the body
}

const bool WITHBALL     = true;
const bool WITHOUTBALL  = false;
void drawManStage(float height, int stage, float progress)
{
    mvstack.push(model_view);
    set_color(0.0f, 0.0f, 0.0f);
    //    model_view *= Translate(0.0f, height/2.0f, 0.0f);
    
    switch (stage)
    {
        case 1: // first wait
            drawManBody(height, 0, progress);
            drawManBodyArms(0, progress, WITHBALL);
            drawManBodyLegs(0, progress);
            drawHead();
            break;
        case 2: // first shoot
                // raise arm, bend leg for first half
                // shoot arm, extend leg for second half
        case 5: //shoots one more time
            if (progress < 0.7) {
                drawManBody(height, BEND_BODY, progress / 0.7);
                drawManBodyArms(RAISE_ARM, progress / 0.7, WITHBALL);
                drawManBodyLegs(BEND_LEG, progress / 0.7);
                drawHead();
            }
            else {
                progress = progress - 0.7;
                drawManBody(height, EXT_BODY, progress / 0.3);
                drawManBodyArms(SHOOT_ARM, progress / 0.3, WITHBALL);
                drawManBodyLegs(EXT_LEG, progress / 0.3);
                drawHead();
            }
            break;
        case 3: // run toward basket:
        case 6: // second run
            if (progress < 0.3) {   // lower arm
                drawManBody(height, 0, progress * 3.33);
                drawManBodyArms(LOWER_ARM, progress * 3.33, WITHOUTBALL);
                drawManBodyLegs(0, progress * 3.33);
                drawHead();
            }
            else { // run arm
                progress = progress - 0.3;
                drawManBody(height, RUN_BODY, progress / 0.7);
                drawManBodyArms(RUN_ARM, progress / 0.7, WITHOUTBALL);
                drawManBodyLegs(RUN_LEG, progress / 0.7);
                drawHead();
            }
            break;
        case 4: // wait
            
            if (progress<0.5) {
                model_view *= Translate(0, 0, DISTANCE_RUN);
            }
            drawManBody(height, 0, progress);
            drawManBodyArms(0, progress, WITHBALL);
            drawManBodyLegs(0, progress);
            drawHead();
            break;
        case 7: // jump
            drawManBody(height, JUMP_BODY, progress);
            drawManBodyArms(RAISE_ARM, progress, WITHOUTBALL);
            drawManBodyLegs(EXT_LEG, progress);
            drawHead();
            break;
        case 8: // wait in the air
            model_view *= Translate(0, HEIGHT_JUMP, DISTANCE_RUN+DISTANCE_JUMP);
            drawManBody(height, 0, progress);
            drawManBodyArms(SHOOT_ARM, progress, WITHBALL);
            drawManBodyLegs(EXT_LEG, progress);
            drawHead();
            break;
        case 9: // dunk!!!!!!!!
            model_view *= Translate(0, HEIGHT_JUMP*(1-progress/3), DISTANCE_RUN+DISTANCE_JUMP+10*progress);
            drawManBody(height, 0, progress);
            drawManBodyArms(LOWER_ARM, progress/2, WITHBALL);
            drawManBodyLegs(0, progress);
            drawHead();
            break;
        case 10:    // fall
            model_view *= Translate(0, HEIGHT_JUMP*(0.66 - 0.66*progress), DISTANCE_JUMP+DISTANCE_RUN+10);
            drawManBody(height, 0, progress);
            drawManBodyArms(LOWER_ARM, 0.5+progress/2, WITHBALL);
            drawManBodyLegs(BEND_LEG, progress);
            drawHead();
            break;
        default:
            drawManBody(height, 0, progress);
            drawManBodyArms(0, progress, WITHBALL);
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
    float firstShootEnd = 7.0f;
    float firstRunEnd = 10.0f;
    float secondWaitEnd = 14.0;
    float secondShootEnd = 16.0f;
    float secondRunEnd = 19.0f;
    float jumpEnd = 20.0f;
    float thirdWaitEnd = 26.0f;
    float dunkEnd = 29.0f;
    float fallEnd = 31.0f;
    
    
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
    drawBallStage(height, stage, progress);
    
}



//void drawPlanets()
//{
//    set_color( .8, .0, .0 );	//model sun
//    mvstack.push(model_view);
//    model_view *= Scale(3);													drawAxes(basis_id++);
//    drawSphere();
//    model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
//    
//    set_color( .0, .0, .8 );	//model earth
//    model_view *= RotateY	( 10*TIME );									drawAxes(basis_id++);
//    model_view *= Translate	( 15, 5*sin( 30*DegreesToRadians*TIME ), 0 );	drawAxes(basis_id++);
//    mvstack.push(model_view);
//    model_view *= RotateY( 300*TIME );										drawAxes(basis_id++);
//    drawCube();
//    model_view = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
//    
//    set_color( .8, .0, .8 );	//model moon
//    model_view *= RotateY	( 30*TIME );									drawAxes(basis_id++);
//    model_view *= Translate	( 2, 0, 0);										drawAxes(basis_id++);
//    model_view *= Scale(0.2);												drawAxes(basis_id++);
//    drawCylinder();
//	
//}


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
    
    float rotationBeginTime = 1;
    float timeToRotate = 5;
    float rotationSceneTime = TIME - rotationBeginTime;
    
    float phantomBeginTime = 21;
    float timeToPhantom = 5;
    float phantomSceneTime = TIME - phantomBeginTime;
    
//    if( rotationSceneTime > 0 && rotationSceneTime < timeToRotate ) { // 360 scene
//        //eye = RotateY(360/timeToRotate * rotationSceneTime) * unRotatedPoint;
//        rotatedEye = RotateY(360/timeToRotate * rotationSceneTime) * eye;
//        //rotatedEye = Translate(0, 0, -TIME) * eye;
//        model_view = LookAt(rotatedEye, ref, up);
//    }
//    else if (phantomSceneTime > 0 && phantomSceneTime < timeToPhantom){ // phantom cam scene
//        vec4 target (0, 10, 0, 1);
//        vec4 camera (30, 10, 0, 1);
//        rotatedEye = RotateY(360/timeToPhantom * phantomSceneTime) * camera;
//        model_view = LookAt(rotatedEye, target, up);
//        model_view *= Translate(0, 0, -85);
//    }
//    else model_view = LookAt(eye, ref, up);
//    
//    if( 0 < TIME && TIME < rotationBeginTime )
//        rotatedEye = eye;
//    
//	// model_view = LookAt( eye, ref, up );
//    
//	model_view *= orientation;
//    model_view *= Scale(zoom);												drawAxes(basis_id++);
//    drawGround(GROUND_LEVEL);                                                           drawAxes(basis_id++);
//    
//    // draw table:
//    set_color(1, 1, 1);
//    mvstack.push(model_view);
//    model_view *= Translate(-10, GROUND_LEVEL+2, 20);
//    model_view *= RotateY(30);
//    model_view *= Scale(15,4,4);
//    drawCube();
//    model_view *= Translate(0, 0, 0.5);
//    model_view *= Scale(1, 1, 0.01);
//    drawDunk();
//    model_view = mvstack.top(); mvstack.pop();
//    
//    // Banner:
//    mvstack.push(model_view);
//    model_view *= Translate(-40, GROUND_LEVEL+30, 50);
//    model_view *= RotateY(90);
//    model_view *= Scale(45, 12, 0.2);
//    drawDunk();
//    model_view *= Translate(0, 0, -0.5);
//    drawCube();
//    model_view = mvstack.top(); mvstack.pop();
//    
//    // Basketball Hoop
//    mvstack.push(model_view);
//    model_view *= Translate(0, GROUND_LEVEL+10, 100);
//    model_view *= Scale(0.5, 20, 0.5);
//    set_color(0.753, 0.753, 0.753);
//    drawCube();
//    model_view *= Translate(0, 0.5, 0);
//    //model_view *= Scale(2, 1/20, 2);
//    model_view *= Scale(20, 0.4, 1);
//    model_view *= Translate(0, 0.5, 0);
//    set_color(0.957, 0.643, 0.376);
//    drawCube();
//    model_view = mvstack.top(); mvstack.pop();
//    
//    
//    model_view *= Translate(0, 0, 40);
//    drawMan(GROUND_LEVEL);
    
    model_view = LookAt(eye, ref, up);
    model_view *= orientation;
    model_view *= Scale(zoom);
    
    set_color(1, 1, 1);
    drawCone();
    model_view *= Scale(10, 10, 10);
    drawHoop();
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