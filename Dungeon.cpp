#include <TextureBuilder.h>
#include <Model_3DS.h>
#include <GLTexture.h>
#include <glut.h>
#include <math.h>
#include <iostream>
#include <time.h>
#include <random>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib,"winmm.lib")

using namespace std;

// Important Macros and definitions
#define BETWEEN(x, a, b) ((x >= a) && (x <= b))
#define PI acos(-1)
#define DEGTORAD(a) (a * (PI / 180.0))
#define RADTODEG(a) (a * (180.0 / PI))
#define GAME_PLAYING 1
#define GAME_END 0
#define GLUT_KEY_ESCAPE 27

// Game Variables
int WIDTH = 1280;
int HEIGHT = 720;
float r = 0.f;
GLuint tex;
char title[] = "Dungeon !!";
double anglePistol = 0;
double angle;
int score = 0;
int lives = 5;
int gameMode = GAME_PLAYING;

// Ghost Motion variables
double ghostAngle = 0;
double femAngle = 0;
double AngleVal = 0.5;
double upGhost = 0;
double upGhostVal = 0.5;

// Wall and Ghost Definitions
int walls[27][4];
double ghosts[10][3];

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 1000;

class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
	bool operator <(Vector &vector) {
		return (x < vector.x) && (y < vector.y) && (z < vector.z);
	}
	bool operator <=(Vector &vector) {
		return (*this < vector) || (x == vector.x) && (y == vector.y) && (z == vector.z);
	}
	bool operator >(Vector &vector) {
		return !(*this <= vector);
	}
	bool operator >=(Vector &vector) {
		return !(*this < vector);
	}
	Vector operator +(Vector &vector) {
		return Vector(x + vector.x, y + vector.y, z + vector.z);
	}
	Vector operator -(Vector &vector) {
		return Vector(x - vector.x, y - vector.y, z - vector.z);
	}
	Vector operator/(float n) {
		return Vector(x / n, y / n, z / n);
	}
	Vector operator *(float n) {
		return Vector(n*x, n*y, n*z);
	}
	Vector cross(Vector v) {
		return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
	Vector unit() {
		return *this / sqrt(x*x + y*y + z*z);
	}
	float magnitude() {
		return sqrt(x*x + y*y + z*z);
	}
};

// Camera Params
Vector Eye(145, 2, -20);
Vector At(145, 2, -30);
Vector Up(0, 1, 0);
Vector EyeNew;
Vector AtNew;
int xOld = 640;
int cameraZoom = 0;

// Model Variables
Model_3DS ghost;
Model_3DS female;
Model_3DS pistol;

// Textures
GLTexture tex_ground;
GLTexture tex_grass;

// Font style
GLvoid *font_style = GLUT_BITMAP_HELVETICA_18;

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//=======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround(int x, int z)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	//glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(-0.5 * x, -0.5 * z);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-0.5 * x, 0, -0.5 * z);
	glTexCoord2f(0.5 * x, -0.5 * z);
	glVertex3f(0.5 * x, 0, -0.5 * z);
	glTexCoord2f(0.5 * x, 0.5 * z);
	glVertex3f(0.5 * x, 0, 0.5 * z);
	glTexCoord2f(-0.5 * x, 0.5 * z);
	glVertex3f(-0.5 * x, 0, 0.5 * z);
	glEnd();
	glPopMatrix();

	/*
	glTexCoord2f(-0.5, -0.5);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-0.5, 0, -0.5);
	glTexCoord2f(0.5, -0.5);
	glVertex3f(0.5, 0, -0.5);
	glTexCoord2f(0.5, 0.5);
	glVertex3f(0.5, 0, 0.5);
	glTexCoord2f(-0.5, 0.5);
	glVertex3f(-0.5, 0, 0.5);
	glEnd();
	glPopMatrix();
	*/

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

int randomPositionX()
{
		return rand() % 401;
}

int randomPositionZ() {
	return -1 *(rand() % 321);
}

void glDrawCuboid(int x, int y, int z) {

	glPushMatrix();
	glPushMatrix();
	glTranslated(0, -0.5  * y, 0);
	RenderGround(x, z);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 0.5 * y, 0);
	RenderGround(x, z);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 0, -0.5 * z);
	glRotated(90, 1, 0, 0);
	RenderGround(x, y);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 0, 0.5 * z);
	glRotated(90, 1, 0, 0);
	RenderGround(x, y);
	glPopMatrix();

	glPushMatrix();
	glTranslated(-0.5 * x, 0, 0);
	glRotated(90, 0, 1, 0);
	glRotated(90, 1, 0, 0);
	RenderGround(z, y);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.5 * x, 0, 0);
	glRotated(90, 0, 1, 0);
	glRotated(90, 1, 0, 0);
	RenderGround(z, y);
	glPopMatrix();

	glPopMatrix();
}

void drawCube() {
	glutSolidCube(10);
}

void fillWallsArray() {

	walls[1][0] = 0;		walls[1][1] = 0;		walls[1][2] = 130;		walls[1][3] = -40;
	walls[2][0] = 0;		walls[2][1] = -40;		walls[2][2] = 50;		walls[2][3] = -300;
	walls[3][0] = 0;		walls[3][1] = -300;		walls[3][2] = 190;		walls[3][3] = -350;
	walls[4][0] = 80;		walls[4][1] = -230;		walls[4][2] = 130;		walls[4][3] = -300;
	walls[5][0] = 50;		walls[5][1] = -150;		walls[5][2] = 100;		walls[5][3] = -200;
	walls[6][0] = 100;		walls[6][1] = -150;		walls[6][2] = 130;		walls[6][3] = -170;
	walls[7][0] = 130;		walls[7][1] = -150;		walls[7][2] = 200;		walls[7][3] = -200;
	walls[8][0] = 80;		walls[8][1] = -110;		walls[8][2] = 200;		walls[8][3] = -120;
	walls[9][0] = 80;		walls[9][1] = -70;		walls[9][2] = 120;		walls[9][3] = -110;
	walls[10][0] = 120;		walls[10][1] = -70;		walls[10][2] = 160;		walls[10][3] = -80;
	walls[11][0] = 160;		walls[11][1] = -50;		walls[11][2] = 230;		walls[11][3] = -80;
	walls[12][0] = 160;		walls[12][1] = 0;		walls[12][2] = 430;		walls[12][3] = -20;
	walls[13][0] = 230;		walls[13][1] = -50;		walls[13][2] = 250;		walls[13][3] = -120;
	walls[14][0] = 360;		walls[14][1] = -20;		walls[14][2] = 380;		walls[14][3] = -200;
	walls[15][0] = 380;		walls[15][1] = -20;		walls[15][2] = 410;		walls[15][3] = -70;
	walls[16][0] = 410;		walls[16][1] = -20;		walls[16][2] = 430;		walls[16][3] = -320;
	walls[17][0] = 290;		walls[17][1] = -50;		walls[17][2] = 330;		walls[17][3] = -80;
	walls[18][0] = 280;		walls[18][1] = -50;		walls[18][2] = 290;		walls[18][3] = -200;
	walls[19][0] = 230;		walls[19][1] = -150;	walls[19][2] = 280;		walls[19][3] = -200;
	walls[20][0] = 290;		walls[20][1] = -110;	walls[20][2] = 330;		walls[20][3] = -270;
	walls[21][0] = 160;		walls[21][1] = -230;	walls[21][2] = 260;		walls[21][3] = -270;
	walls[22][0] = 220;		walls[22][1] = -270;	walls[22][2] = 260;		walls[22][3] = -290;
	walls[23][0] = 290;		walls[23][1] = -300;	walls[23][2] = 330;		walls[23][3] = -320;
	walls[24][0] = 360;		walls[24][1] = -290;	walls[24][2] = 410;		walls[24][3] = -310;
	walls[25][0] = 360;		walls[25][1] = -230;	walls[25][2] = 410;		walls[25][3] = -260;
	walls[26][0] = 220;		walls[26][1] = -320;	walls[26][2] = 430;		walls[26][3] = -350;

}

void fillGhostsArray() {

	ghosts[0][0] = 70;		ghosts[0][1] = -280;	ghosts[0][2] = 0;
	ghosts[1][0] = 210;		ghosts[1][1] = -310;	ghosts[1][2] = 1;
	ghosts[2][0] = 150;		ghosts[2][1] = -60;		ghosts[2][2] = 0;
	ghosts[3][0] = 310;		ghosts[3][1] = -290;	ghosts[3][2] = 1;
	ghosts[4][0] = 350;		ghosts[4][1] = -210;	ghosts[4][2] = 0;
	ghosts[5][0] = 70;		ghosts[5][1] = -110;	ghosts[5][2] = 1;
	ghosts[6][0] = 170;		ghosts[6][1] = -100;	ghosts[6][2] = 0;
	ghosts[7][0] = 220;		ghosts[7][1] = -180;	ghosts[7][2] = 1;
	ghosts[8][0] = 270;		ghosts[8][1] = -100;	ghosts[8][2] = 0;
	ghosts[9][0] = 350;		ghosts[9][1] = -90;		ghosts[9][2] = 1;

}

bool checkInWall(int x, int z, int wall) {
	return (BETWEEN(x, walls[wall][0], walls[wall][2]) && BETWEEN(z, walls[wall][3], walls[wall][1]));
}

bool allowedMove(int x, int z, bool pistol) {
	bool inWall = !(BETWEEN(x, 0, 430) && z <= 0);
	int sp = (pistol) ? 0 : 3;
	for (int i = 1; i <= 26 && !inWall; i++)
		inWall |= (checkInWall(x + sp, z + sp, i) || checkInWall(x + sp, z - sp, i) || checkInWall(x - sp, z + sp, i) || checkInWall(x - sp, z - sp, i));
	return !inWall;
}

int insideMonster(int x, int z) {
	for (int i = 0; i < 10; i++)
		if (sqrt((x - ghosts[i][0])*(x - ghosts[i][0]) + (z - ghosts[i][1])*(z - ghosts[i][1])) <= 9)
			return i;
	return -1;
}

void drawDungeon() {
	glPushMatrix();
	glTranslated(65, 0, -20);
	glPushMatrix();
	glTranslated(0, 0, 0);
	glScaled(10, 10, 10);
	glDrawCuboid(13, 3, 4);
	glPopMatrix();
	glPushMatrix();
	glTranslated(305, 0, -90);
	glScaled(10, 10, 10);
	glDrawCuboid(2, 3, 18);
	glPopMatrix();
	glPushMatrix();
	glTranslated(330, 0, -25);
	glScaled(10, 10, 10);
	glDrawCuboid(3, 3, 5);
	glPopMatrix();
	glPushMatrix();
	glTranslated(355, 0, -150);
	glScaled(10, 10, 10);
	glDrawCuboid(2, 3, 30);
	glPopMatrix();
	glPushMatrix();
	glTranslated(260, 0, -315);
	glScaled(10, 10, 10);
	glDrawCuboid(21, 3, 3);
	glPopMatrix();
	glPushMatrix();
	glPushMatrix();
	glTranslated(320, 0, -225);
	glScaled(10, 10, 10);
	glDrawCuboid(5, 3, 3);
	glPopMatrix();
	glPushMatrix();
	glPushMatrix();
	glTranslated(320, 0, -285);
	glScaled(10, 10, 10);
	glDrawCuboid(5, 3, 3);
	glPopMatrix();
	glPushMatrix();
	glTranslated(230, 0, 10);
	glScaled(10, 10, 10);
	glDrawCuboid(27, 3, 2);
	glPopMatrix();
	glPushMatrix();
	glTranslated(-40, 0, -150);
	glScaled(10, 10, 10);
	glDrawCuboid(5, 3, 26);
	glPopMatrix();
	glPushMatrix();
	glTranslated(30, 0, -305);
	glScaled(10, 10, 10);
	glDrawCuboid(19, 3, 5);
	glPopMatrix();
	glPushMatrix();
	glTranslated(10, 0, -155);
	glScaled(10, 10, 10);
	glDrawCuboid(5, 3, 5);
	glPopMatrix();
	glPushMatrix();
	glTranslated(40, 0, -245);
	glScaled(10, 10, 10);
	glDrawCuboid(5, 3, 7);
	glPopMatrix();
	glPushMatrix();
	glTranslated(50, 0, -140);
	glScaled(10, 10, 10);
	glDrawCuboid(3, 3, 2);
	glPopMatrix();
	glPushMatrix();
	glTranslated(100, 0, -155);
	glScaled(10, 10, 10);
	glDrawCuboid(7, 3, 5);
	glPopMatrix();
	glPushMatrix();
	glTranslated(190, 0, -155);
	glScaled(10, 10, 10);
	glDrawCuboid(5, 3, 5);
	glPopMatrix();
	glPushMatrix();
	glTranslated(220, 0, -105);
	glScaled(10, 10, 10);
	glDrawCuboid(1, 3, 15);
	glPopMatrix();
	glPushMatrix();
	glTranslated(245, 0, -45);
	glScaled(10, 10, 10);
	glDrawCuboid(4, 3, 3);
	glPopMatrix();
	glPushMatrix();
	glTranslated(245, 0, -170);
	glScaled(10, 10, 10);
	glDrawCuboid(4, 3, 16);
	glPopMatrix();
	glPushMatrix();
	glTranslated(245, 0, -290);
	glScaled(10, 10, 10);
	glDrawCuboid(4, 3, 2);
	glPopMatrix();
	glPushMatrix();
	glTranslated(175, 0, -260);
	glScaled(10, 10, 10);
	glDrawCuboid(4, 3, 2);
	glPopMatrix();
	glPushMatrix();
	glTranslated(145, 0, -230);
	glScaled(10, 10, 10);
	glDrawCuboid(10, 3, 4);
	glPopMatrix();
	glPushMatrix();
	glTranslated(35, 0, -70);
	glScaled(10, 10, 10);
	glDrawCuboid(4, 3, 4);
	glPopMatrix();
	glPushMatrix();
	glTranslated(75, 0, -95);
	glScaled(10, 10, 10);
	glDrawCuboid(12, 3, 1);
	glPopMatrix();
	glPushMatrix();
	glTranslated(75, 0, -55);
	glScaled(10, 10, 10);
	glDrawCuboid(4, 3, 1);
	glPopMatrix();
	glPushMatrix();
	glTranslated(130, 0, -45);
	glScaled(10, 10, 10);
	glDrawCuboid(7, 3, 3);
	glPopMatrix();
	glPushMatrix();
	glTranslated(175, 0, -65);
	glScaled(10, 10, 10);
	glDrawCuboid(2, 3, 7);
	glPopMatrix();
	glPopMatrix();

}

void groundGrass() {

	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_grass.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-500, -16, 420);
	glTexCoord2f(5, 0);
	glVertex3f(500, -16, -420);
	glTexCoord2f(5, 5);
	glVertex3f(500, -16, 420);
	glTexCoord2f(0, 5);
	glVertex3f(-500, -16, -420);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.

}

void printString(float x, float y, char* format, ...)
{
	va_list args;
	int len;
	int i;
	char * text;

	va_start(args, format);
	len = _vscprintf(format, args) + 1;
	text = (char*)malloc(len * sizeof(char));
	vsprintf_s(text, len, format, args);
	va_end(args);

	glDisable(GL_TEXTURE_2D); 
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2d(x, y);

	for (i = 0; text[i] != '\0'; i++)
		glutBitmapCharacter(font_style, text[i]);

	glMatrixMode(GL_PROJECTION); 
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW); 
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);

	free(text);
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Pistol
	glPushMatrix();
	glTranslated(Eye.x, Eye.y - 6, Eye.z);
	glRotated(anglePistol, 0, 1, 0);
	glTranslated(0, 0, -10);
	glRotated(180, 0, 1, 0);
	glRotated(75, 1, 0, 0);
	glScaled(10.0, 10.0, 10.0);
	pistol.Draw();
	glPopMatrix();

	// Draw Monsters in ghost Array !
	for (int i = 0; i < 10; i++) {
		glPushMatrix();
		{
			bool ghostOrFemale = ghosts[i][2];
			float scale = (ghostOrFemale) ? 0.025 : 4.5;
			glTranslated(ghosts[i][0], ((ghostOrFemale) ? (-1.0 + upGhost) : (-12.0)), ghosts[i][1]);
			glScaled(scale, scale, scale);
			double lookingAngle = RADTODEG(atan2(Eye.x - ghosts[i][0], Eye.z - ghosts[i][1]));
			glRotated(40.0 + lookingAngle + (ghostOrFemale) ? (ghostAngle) : (femAngle), 0, 1, 0);

			if (ghostOrFemale) ghost.Draw();
			else female.Draw();

		}
		glPopMatrix();
	}

	// Draw Dungeon
	drawDungeon();

	// Draw Ground
	glPushMatrix();
	glTranslated(200, 0, -175);
	glScaled(10, 10, 10);
	glPushMatrix();
	glTranslated(0, -0.5 * 3, 0);
	RenderGround(43, 35);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0, 0.5 * 3, 0);
	RenderGround(43, 35);
	glPopMatrix();
	glPopMatrix();
	
	// Sky Box
	glPushMatrix();

	GLUquadricObj * qobj;
	qobj = gluNewQuadric();
	glTranslated(200,0,-160);
	glScaled(0.4, 0.4, 0.4);
	glRotated(90,1,0,1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj,true);
	gluQuadricNormals(qobj,GL_SMOOTH);
	gluSphere(qobj,1000,100,100);
	gluDeleteQuadric(qobj);

	glPopMatrix();
	groundGrass();

	// Show score and lives
	printString((gameMode)?30.0:340.0, (gameMode)?675.0:340.0, "Score = %d, Lives = %d. %s", 
		score, lives, (gameMode)?"":"YOU WIN !! To Play Again press 'P'; To Quit press ESC.");
	
	/*
	* // Draw Ghost
	* glPushMatrix();
	* glTranslated(145.0, -1.0, -30.0);
	* glScaled(0.025, 0.025, 0.025);
	* glRotated(40.0, 0, 1, 0);
	* ghost.Draw();
	* glPopMatrix();
	*
	* // Draw Female
	* glPushMatrix();
	* glTranslated(145.0, -15.0, -60.0);
	* glScaled(6.5, 6.5, 6.5);
	* glRotated(40.0, 0, 1, 0);
	* female.Draw();
	* glPopMatrix();
	*
	*/

	glutSwapBuffers();
}

void monsterMotion(int val) {

	if (gameMode == GAME_END) return;

	ghostAngle += AngleVal;
	femAngle += AngleVal;
	upGhost += upGhostVal;

	if (ghostAngle >= 50 || ghostAngle <= -50 || femAngle >= 50 || femAngle <= -50) AngleVal = -AngleVal;
	if (upGhost >= 1 || upGhost <= -1) upGhostVal = -upGhostVal;

	for (int i = 0; i < 10; i++) {
		Vector monster(ghosts[i][0], 0, ghosts[i][1]);
		Vector monsterMoved = monster + (Eye - monster).unit() * 0.18;
		ghosts[i][0] = monsterMoved.x;
		ghosts[i][1] = monsterMoved.z;
	}

	int m = insideMonster(Eye.x, Eye.z);
	if (m != -1) {
		score--;
		lives--;

		if (lives == -1) {
			Eye = Vector(145, 2, -20);
			At = Vector(145, 2, -30);
			gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
			score = 0;
			lives = 5;
			anglePistol = 0;
		}

		// Monster is regenerated at random place
		ghosts[m][0] = randomPositionX();
		ghosts[m][1] = randomPositionZ();
		ghosts[m][2] = randomPositionX()%2;

		PlaySound(TEXT("sounds/PlayerPain.wav"), NULL, SND_ASYNC | SND_FILENAME);

	}

	glutPostRedisplay();
	glutTimerFunc(100, monsterMotion, 0);


}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(int button, int x, int y)
{
	if (gameMode == GAME_END) return;

	if (Eye.z < -350) {
		gameMode = GAME_END; 
		glutPostRedisplay();
		return;
	}

	Vector forward = At - Eye;
	Vector right = forward.cross(Vector(0, 1, 0));
	Vector backward = Eye - At;
	Vector left = Vector(0, 1, 0).cross(forward);

	bool allowed = allowedMove(EyeNew.x, EyeNew.z, false) && allowedMove(AtNew.x, AtNew.z, false);

	float moveMagnitude = 0.09;

	switch (button)
	{
	case GLUT_KEY_UP:
		EyeNew = Eye + (forward * moveMagnitude);
		AtNew = At + (forward * moveMagnitude);
		Eye = (allowed) ? EyeNew : Eye;
		At = (allowed) ? AtNew : At;
		break;
	case GLUT_KEY_DOWN:
		EyeNew = Eye + (backward * moveMagnitude);
		AtNew = At + (backward * moveMagnitude);
		Eye = (allowed) ? EyeNew : Eye;
		At = (allowed) ? AtNew : At;
		break;
	case GLUT_KEY_LEFT:
		EyeNew = Eye + (left * moveMagnitude);
		AtNew = At + (left * moveMagnitude);
		Eye = (allowed) ? EyeNew : Eye;
		At = (allowed) ? AtNew : At;
		break;
	case GLUT_KEY_RIGHT:
		EyeNew = Eye + (right * moveMagnitude);
		AtNew = At + (right * moveMagnitude);
		Eye = (allowed) ? EyeNew : Eye;
		At = (allowed) ? AtNew : At;
		break;
	default:
		break;
	}
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	glutPostRedisplay();
}

void myKeyboardEnd(unsigned char k, int x, int y) {

	if(k == GLUT_KEY_ESCAPE) exit(EXIT_SUCCESS);

	if (gameMode == GAME_PLAYING) return;

	if (k == 'P' || k == 'p') {
		Eye = Vector(145, 2, -20);
		At = Vector(145, 2, -30);
		score = 0;
		lives = 5;
		anglePistol = 0;
		gameMode = GAME_PLAYING;
		fillGhostsArray();
		glutTimerFunc(0, monsterMotion, 0);
	}

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	glLoadIdentity();
	glutPostRedisplay();

}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int x, int y)
{
	if (gameMode == GAME_END) return;

	angle = 360 * ((xOld - x)*1.0 / WIDTH);

	Vector view = (At - Eye).unit();
	float mag = (At - Eye).magnitude();
	Vector right = Up.cross(view).unit();
	view = view * (cos(angle*0.0174532925)) + right*(sin(angle*0.017453292));
	right = view.cross(Up);


	// Handling Pistol overlap with corners when rotating with mouse
	// But commented due to performance and slow game issues
	
	Vector AtNew = Eye + view * mag;
	Vector pistolVector = AtNew + ((AtNew - Eye).unit()) * 10;
	boolean allowed = allowedMove(pistolVector.x, pistolVector.z, true);
	At = (allowed) ? AtNew : At;
	anglePistol += (allowed) ? angle:0;
	if (!allowed) glutWarpPointer(xOld, y);
	else xOld = x;
	

	// Comment the following three lines if you want to use the previous commented logic
	/*anglePistol += angle;
	At = Eye + view * mag;
	xOld = x;*/

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	glutPostRedisplay();

}

void shootingMouse(int button, int state, int x, int y) {

	if (gameMode == GAME_END) return;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

		PlaySound(TEXT("sounds/Pistol.wav"), NULL, SND_ASYNC | SND_FILENAME);
		Vector loop(Eye.x, Eye.y, Eye.z);

		while (allowedMove(loop.x, loop.z, true)) {
			loop = (loop + (At - Eye).unit());
			int m = insideMonster(loop.x, loop.z);
			
			if (m != -1) {
				// Regenerate & score increment
				score++;
				// Monster is regenerated at random place
				ghosts[m][0] = randomPositionX();
				ghosts[m][1] = randomPositionZ();
				ghosts[m][2] = randomPositionX()%2;
				PlaySound(TEXT("sounds/MonsterDeath.wav"), NULL, SND_ASYNC | SND_FILENAME);
				return;
			}
		}
		glutPostRedisplay();
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	ghost.Load("Models/little_ghost/little_ghost.3ds");
	female.Load("Models/Lambent_Female/Lambent_Female.3ds");
	pistol.Load("Models/Pistol/Pistol.3ds");

	// Loading Texture files
	tex_ground.Load("Textures/dungeon_ground.tga");
	tex_grass.Load("Textures/ground.tga");
	
	loadBMP(&tex, "Textures/sky4-jpg.bmp", true);

}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	fillWallsArray();
	fillGhostsArray();

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(0, 0);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboardEnd);
	glutSpecialFunc(myKeyboard);

	glutPassiveMotionFunc(myMouse);
	glutMouseFunc(shootingMouse);

	glutTimerFunc(0, monsterMotion, 0);

	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
	srand(time(NULL));
	glutMainLoop();
}