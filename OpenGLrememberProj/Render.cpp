#include "Render.h"
#include <chrono>
#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"
#include "Render.h"
#include <math.h>
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <iostream>
#include "ObjLoader.h"
#include "Texture.h"

#define PI 3.14159265358979323846   // pi
using std::vector;
class Point {
public:
	double x;
	double y;
	double z;
	void DrawPoint() {
		glVertex3d(this->x, this->y, this->z);
	}
	void Translated() {
		glTranslated(this->x, this->y, this->z);
	}
	Point(double x, double y, double z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};
double SearchVectorLength(Point vector) {
	double length = sqrt(pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2));
	return length;
}
Point VectorNormalization(Point vector) {
	//поиск длины вектора
	double length = SearchVectorLength(vector);

	//нормализация вектора
	vector.x = vector.x / length;
	vector.y = vector.y / length;
	vector.z = vector.z / length;

	return vector;
}
Point VectorProduct(Point vectorA, Point vectorB) {
	Point result(0, 0, 0);
	result.x = vectorA.y * vectorB.z - vectorB.y * vectorA.z;
	result.y = -1 * vectorA.x * vectorB.z + vectorB.x * vectorA.z;
	result.z = vectorA.x * vectorB.y - vectorB.x * vectorA.y;
	return result;
}
double ScalarProduct(Point A, Point B) {
	return A.x * B.x + A.y * B.y + A.z * B.z;
}
static Point SearchVector(Point A, Point B) {
	return Point(B.x - A.x, B.y - A.y, B.z - A.z);
}
double sharpness_t = 0.1;
//координаты точек поверхности Безье
extern vector<vector<Point>> mas;
//точки для кривых Эрмита, Безье 2 и 3 порядков
extern vector<Point> PointHB;
//Рассчет одной точки для кривой Эрмита

//точки для поверхности Безье
vector<vector<Point>> mas = {
	{Point(0, 0, 0), Point(0, 1, 1), Point(6, 2, 1), Point(0, 3, 0)},
	{Point(1, 0, 1), Point(1, 1, 1), Point(1, 2, 1), Point(1, 3, 1)},
	{Point(2, 0, 1), Point(2, 1, 1), Point(2, 2, 1), Point(2, 3, 1)},
	{Point(3, 0, 0), Point(3, 1, 1), Point(3, 2, 1), Point(3, 3, 0)}
};
vector<Point> PointHB = {
	//Рисуем кривые Эрмита
	Point(0, 0, 0), Point(0, 0, 0),  Point(35, 0, 0), Point(10, 3, 9),
	Point(4, 8, 10), Point(9, 3, 6), Point(4, 2, 7), Point(4, 6, 1),
	//Рисуем кривые Безье 2 порядка
	Point(-2, -2, -2), Point(-6, -4, 0), Point(-10, -8, -5),
	Point(-4, -8, -10), Point(-9, -3, -6), Point(-12, -5, -9),
	//Рисуем кривую Безье 3 порядка
	Point(-2, 2, 2), Point(-6, 4, 0), Point(-7, 5, 1), Point(-3, 4, 5),
	Point(-4, 8, 10), Point(-9, 3, 6), Point(-4, 6, 1), Point(-4, 2, 7)
};
vector<Point> Line = {
	Point(1, 0, 0), Point(30, 0, 0)
};

using namespace std;
bool textureMode = true;
bool lightMode = true;
void Change_Texture();
//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
		delete POINT;
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


	if (!OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON)) {
		LPPOINT POINT = new tagPOINT();
		//Извлекает положение курсора мыши в координатах экрана.
		GetCursorPos(POINT);
		//Функция ScreenToClient переделывает экранные координаты указанной точки на экране в координаты рабочей области
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		// параметры viewport-a
		GLint viewport[4];
		// матрица проекции
		GLdouble projection[16];
		// видовая матрица
		GLdouble modelview[16];

		// узнаём параметры viewport-a
		glGetIntegerv(GL_VIEWPORT, viewport);
		//узнаём матрицу проекции
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		// узнаём видовую матрицу
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		double delta = 10;
		for (auto& v : mas)
		{
			for (auto& elem : v)
			{
				double tempPoint[3];
				// Получаем экранные координаты обьекта
				gluProject(elem.x, elem.y, elem.z, modelview, projection, viewport, &tempPoint[0], &tempPoint[1], &tempPoint[2]);
				if (tempPoint[0] > POINT->x - delta && tempPoint[0] < POINT->x + delta &&
					tempPoint[1] > POINT->y - delta && tempPoint[1] < POINT->y + delta)
				{
					tempPoint[0] -= dx;
					tempPoint[1] += dy;
					// Переводим экранные координаты обратно в объектные
					gluUnProject(tempPoint[0], tempPoint[1], tempPoint[2], modelview, projection, viewport, &elem.x, &elem.y, &elem.z);
				}
			}
		}
		for (auto& elem : PointHB) {
			double tempPoint[3];
			// Получаем экранные координаты обьекта
			gluProject(elem.x, elem.y, elem.z, modelview, projection, viewport, &tempPoint[0], &tempPoint[1], &tempPoint[2]);
			if (tempPoint[0] > POINT->x - delta && tempPoint[0] < POINT->x + delta &&
				tempPoint[1] > POINT->y - delta && tempPoint[1] < POINT->y + delta)
			{
				tempPoint[0] -= dx;
				tempPoint[1] += dy;
				// Переводим экранные координаты обратно в объектные
				gluUnProject(tempPoint[0], tempPoint[1], tempPoint[2], modelview, projection, viewport, &elem.x, &elem.y, &elem.z);
			}
		}
		delete POINT;
	}
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}
int swap1 = 0;
bool IsGun = false;

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
	if (key == 'C') {
		Change_Texture();
	}
	if (key == 'N') {
		IsGun = true;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}
GLuint skin, mish, flag, red, skala, kuku;
void Change_Texture() {
	static int k = 0;
	if (k % 2 == 0) {
		RGBTRIPLE* texarray;
		char* texCharArray;
		int texW, texH;
		OpenGL::LoadBMP("textures\\howl.bmp", &texW, &texH, &texarray);
		OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



		//генерируем ИД для текстуры
		glGenTextures(1, &skin);
		//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
		glBindTexture(GL_TEXTURE_2D, skin);

		//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

		//отчистка памяти
		free(texCharArray);
		free(texarray);
		//наводим шмон
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		k++;
	}
	else {
		RGBTRIPLE* texarray;
		char* texCharArray;
		int texW, texH;
		OpenGL::LoadBMP("textures\\zzz.bmp", &texW, &texH, &texarray);
		OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



		//генерируем ИД для текстуры
		glGenTextures(1, &skin);
		//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
		glBindTexture(GL_TEXTURE_2D, skin);

		//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

		//отчистка памяти
		free(texCharArray);
		free(texarray);
		//наводим шмон
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		k++;
	}
}

ObjFile pistol, darts, patr;
Texture pistolTex;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray, *texarray2, *texarray3, *texarray4, *texarray5;

	//1
	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("textures\\howl.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	//генерируем ИД для текстуры
	glGenTextures(1, &skin);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, skin);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);
	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//2
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("textures\\dart.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);

	//генерируем ИД для текстуры
	glGenTextures(1, &mish);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, mish);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);
	//отчистка памяти
	free(texCharArray2);
	free(texarray2);
	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//3
	char* texCharArray3;
	int texW3, texH3;
	OpenGL::LoadBMP("textures\\rus.bmp", &texW3, &texH3, &texarray3);
	OpenGL::RGBtoChar(texarray3, texW3, texH3, &texCharArray3);

	//генерируем ИД для текстуры
	glGenTextures(1, &flag);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, flag);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW3, texH3, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray3);
	//отчистка памяти
	free(texCharArray3);
	free(texarray3);
	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//4
	char* texCharArray4;
	int texW4, texH4;
	OpenGL::LoadBMP("textures\\red.bmp", &texW4, &texH4, &texarray4);
	OpenGL::RGBtoChar(texarray4, texW4, texH4, &texCharArray4);

	//генерируем ИД для текстуры
	glGenTextures(1, &red);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, red);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW4, texH4, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray4);
	//отчистка памяти
	free(texCharArray4);
	free(texarray4);
	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//5
	char* texCharArray5;
	int texW5, texH5;
	OpenGL::LoadBMP("textures\\skala.bmp", &texW5, &texH5, &texarray5);
	OpenGL::RGBtoChar(texarray5, texW5, texH5, &texCharArray5);

	//генерируем ИД для текстуры
	glGenTextures(1, &skala);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, skala);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW5, texH5, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray5);
	//отчистка памяти
	free(texCharArray5);
	free(texarray5);
	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	loadModel("models\\darboard.obj", &darts);
	loadModel("models\\bullet.obj", &patr);
	loadModel("models\\m4a1.obj", &pistol);

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}
void krug(float centerX, float centerY, float centerZ) {
	float xcos, ysin, tx, ty, angle, radian, x, y, r = 2;       // values needed by drawCircleOutline

	glBegin(GL_POLYGON);
	//glNormal3d(0, 0, 29);
	for (angle = 0.0; angle < 360.0; angle += 2.0) {
		radian = angle * (PI / 180.0f);
		xcos = (float)cos(radian);
		ysin = (float)sin(radian);
		x = xcos * r + centerX;  // Используем centerX
		y = ysin * r + centerY;  // Используем centerY
		tx = xcos * 0.5 + 0.5;
		ty = ysin * 0.5 + 0.5;
		glTexCoord2f(tx, ty);
		glVertex3f(x, y, centerZ);  // Используем centerZ
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnd();
}
bool isPointOnCircle(double pointX, double pointY, double pointZ, double centerX, double centerY, double centerZ, double radius) {
	// Вычисляем расстояние от заданной точки до центра окружности
	if (pow(pointX - centerX, 2) + pow(pointY - centerY, 2) <= pow(radius, 2)) {
		return 1;
	}
	else {
		return 0;
	}
}

Point HermitCurve(Point P1, Point R1, Point P2, Point R2, double t) {
	double t_2 = t * t;
	double t_3 = t * t * t;

	double x = P1.x * (2 * t_3 - 3 * t_2 + 1) + P2.x * (-2 * t_3 + 3 * t_2) + R1.x * (t_3 - 2 * t_2 + t) + R2.x * (t_3 - t_2);
	double y = P1.y * (2 * t_3 - 3 * t_2 + 1) + P2.y * (-2 * t_3 + 3 * t_2) + R1.y * (t_3 - 2 * t_2 + t) + R2.y * (t_3 - t_2);
	double z = P1.z * (2 * t_3 - 3 * t_2 + 1) + P2.z * (-2 * t_3 + 3 * t_2) + R1.z * (t_3 - 2 * t_2 + t) + R2.z * (t_3 - t_2);

	return Point(x, y, z);
}
Point LinearInterpolation(Point P1, Point P2, double t) {
	double x = P1.x * (1 - t) + P2.x * t;
	double y = P1.y * (1 - t) + P2.y * t;
	double z = P1.z * (1 - t) + P2.z * t;

	return Point(x, y, z);
}
vector<Point> PointLinearInterpolation(Point P1, Point P2, double t_max = 1) {
	vector<Point> points;
	t_max = 1 + sharpness_t * 0.001;

	for (double i = 0; i <= t_max; i += sharpness_t) {
		points.push_back(LinearInterpolation(P1, P2, i));
	}
	return points;
}
//Рассчитать точки кривой Эрмита
//P1 - точка начала кривой, P2 - точка конца кривой
//R1 - вектор произвольной кривой при t = 0, R2 - вектор проивольной кривой при t = 1
//t - шаг
//t_max - максимальное значение t
vector<Point> PointHermitCurve(Point P1, Point R1, Point P2, Point R2, double t_max = 1) {
	vector<Point> points;
	t_max = 1 + sharpness_t * 0.001;

	for (double i = 0; i <= t_max; i += sharpness_t) {
		points.push_back(HermitCurve(P1, R1, P2, R2, i));
	}
	return points;
}
//Рассет одной точки для кривой Безье второго порядка
Point BezierCurve2(Point P1, Point P2, Point P3, double t) {
	double t_2 = t * t;
	double var = 1 - t;
	double var_2 = var * var;

	double x = P1.x * var_2 + 2 * P2.x * t * var + P3.x * t_2;
	double y = P1.y * var_2 + 2 * P2.y * t * var + P3.y * t_2;
	double z = P1.z * var_2 + 2 * P2.z * t * var + P3.z * t_2;

	return Point(x, y, z);
}

//Рассчитать точки кривой Безье второго порядка
//P1, P2, P3 - точки, t - шаг, t_max - максимальное значение t 
vector<Point> PointBezierCurve2(Point P1, Point P2, Point P3, double t_max = 1) {
	vector<Point> points;
	t_max = 1 + sharpness_t * 0.001;

	for (double i = 0; i <= t_max; i += sharpness_t) {
		points.push_back(BezierCurve2(P1, P2, P3, i));
	}
	return points;
}

//Рассчитать точки кривой Безье третьего порядка
Point BezierCurve3(Point P0, Point P1, Point P2, Point P3, double t) {
	double t_2 = t * t;
	double t_3 = t_2 * t;
	double var = 1 - t;
	double var_2 = var * var;
	double var_3 = var_2 * var;

	double x = var_3 * P0.x + 3 * t * var_2 * P1.x + 3 * t_2 * var * P2.x + t_3 * P3.x;
	double y = var_3 * P0.y + 3 * t * var_2 * P1.y + 3 * t_2 * var * P2.y + t_3 * P3.y;
	double z = var_3 * P0.z + 3 * t * var_2 * P1.z + 3 * t_2 * var * P2.z + t_3 * P3.z;

	return Point(x, y, z);
}

//Рассчитать точки кривой Безье третьего порядка
//Начинается в точке P0, стремится приблизится к P1, затем к P2 и заканчивается в точке P3
//t - шаг, t_max - максимальное значение t
vector<Point> PointBezierCurve3(Point P0, Point P1, Point P2, Point P3, double t_max = 1) {
	vector<Point> points;
	t_max = 1 + sharpness_t * 0.001;

	for (double i = 0; i <= t_max; i += sharpness_t) {
		points.push_back(BezierCurve3(P0, P1, P2, P3, i));
	}
	return points;
}
//Рисуем кривую Эрмита
void DrawHermitCurve(Point P1, Point R1, Point P2, Point R2) {
	//размер точки
	glPointSize(10);

	//рисуем точки
	glColor3d(1, 0, 0);
	glBegin(GL_POINTS);
	P1.DrawPoint();
	P2.DrawPoint();
	R1.DrawPoint();
	R2.DrawPoint();
	glEnd();

	//размер точки
	glPointSize(1);


	//рисуем вектора
	glColor3d(0, 0, 1);
	glBegin(GL_LINES);
	P1.DrawPoint();
	R1.DrawPoint();
	P2.DrawPoint();
	R2.DrawPoint();
	glEnd();

	glLineWidth(3);

	R1 = SearchVector(P1, R1);
	R2 = SearchVector(P2, R2);

	//Рисуем кривую Эрмита
	glColor3d(1, 0, 0);
	glBegin(GL_LINE_STRIP);
	vector<Point> points = PointHermitCurve(P1, R1, P2, R2);
	for (int i = 0; i < (int)points.size(); i++) {
		points[i].DrawPoint();
	}
	glEnd();
}
//Рисуем прямую y = const
void DrawLine(Point P1, Point P2) {
	//размер точки
	glPointSize(10);

	//рисуем точки
	glColor3d(1, 0, 0);
	glBegin(GL_POINTS);
	P1.DrawPoint();
	P2.DrawPoint();
	glEnd();

	//размер точки
	glPointSize(1);


	//рисуем вектора
	glColor3d(0, 0, 1);
	glBegin(GL_LINES);
	P1.DrawPoint();
	P2.DrawPoint();
	glEnd();

	glLineWidth(3);

	//R1 = SearchVector(P1, R1);
	//R2 = SearchVector(P2, R2);

	//Рисуем прямую
	glColor3d(1, 0, 0);
	glBegin(GL_LINE_STRIP);
	vector<Point> points = PointLinearInterpolation(P1, P2);
	for (int i = 0; i < (int)points.size(); i++) {
		points[i].DrawPoint();
	}
	glEnd();

}

void DrawBezierCurve2(Point A, Point B, Point C) {
	//размер точки
	glPointSize(10);

	//рисуем точки
	glColor3d(1, 0, 1);
	glBegin(GL_POINTS);
	A.DrawPoint();
	B.DrawPoint();
	C.DrawPoint();
	glEnd();

	//размер точки
	glPointSize(1);

	//рисуем ломанную ABC
	glColor3d(0.8, 1, 0.3);
	glBegin(GL_LINE_STRIP);
	A.DrawPoint();
	B.DrawPoint();
	C.DrawPoint();
	glEnd();

	//ширина линии
	glLineWidth(3);

	//рисуем кривую Безье второго порядка
	glColor3d(0, 1, 0);
	glBegin(GL_LINE_STRIP);
	vector<Point> points = PointBezierCurve2(A, B, C);
	for (int i = 0; i < (int)points.size(); i++) {
		points[i].DrawPoint();
	}
	glEnd();

	//ширина линии
	glLineWidth(1);

}

//Рисуем кривую Безье третьего порядка
void DrawBezierCurve3(Point A, Point B, Point C, Point D) {
	//размер точки
	glPointSize(10);

	//рисуем точки
	glColor3d(1, 0, 1);
	glBegin(GL_POINTS);
	A.DrawPoint();
	B.DrawPoint();
	C.DrawPoint();
	D.DrawPoint();
	glEnd();

	//размер точки
	glPointSize(1);

	//рисуем ломанную ABCD
	glColor3d(0.7, 0.2, 0.6);
	glBegin(GL_LINE_STRIP);
	A.DrawPoint();
	B.DrawPoint();
	C.DrawPoint();
	D.DrawPoint();
	glEnd();

	// ширина линии
	glLineWidth(3);

	//рисуем кривую Безье второго порядка
	glColor3d(0, 0, 1);
	glBegin(GL_LINE_STRIP);
	vector<Point> points = PointBezierCurve3(A, B, C, D);
	for (int i = 0; i < (int)points.size(); i++) {
		points[i].DrawPoint();
	}
	glEnd();

	//ширина линии
	glLineWidth(1);

}
enum Method {
	Hermit,
	Bezier2,
	Bezier3,
	Linein
};

void MoveAxis(Point point, Point next_point) {
	Point dir = SearchVector(point, next_point);
	dir = VectorNormalization(dir);

	Point orig(1, 0, 0);
	Point rotX(dir.x, dir.y, 0);
	rotX = VectorNormalization(rotX);
	double cosU = ScalarProduct(orig, rotX);
	Point vecpr = VectorProduct(orig, rotX);

	double sinSign = vecpr.z / abs(vecpr.z);
	double U = acos(cosU) * 180.0 / PI * sinSign;
	double ZU = acos(dir.z) * 180.0 / PI - 90;

	glRotated(U, 0, 0, 1);
	glRotated(ZU, 0, 1, 0);
}

void AnimationObjectCurve(vector<Point> points, double t, double l, Method method) {
	vector<Point> PointsCurve;
	Point point(0, 0, 0);
	Point next_point(0, 0, 0);
	Point last_point(0, 0, 0);

	float s1 = 0.4, s2 = 0.5;

	double t_max = t;

	double centerX = 35; // Центр окружности
	double centerY = 0;
	double centerZ = 0;
	double radius = 2;
	Point item = HermitCurve(points[0], points[1], points[2], points[3], t_max);
	switch (method) {
	case Hermit:
			if (isPointOnCircle(item.x, item.y, item.z, centerX, centerY, centerZ, radius)) {
				glPushMatrix();
				glBindTexture(GL_TEXTURE_2D, skala);
				glScalef(s2, s2, s2);
				glTranslatef(30.0f, 0.0f, 0.0f);
				glRotatef(90, 1.0f, 0.0f, 0.0f);
				glRotatef(90, 0.0f, -1.0f, 0.0f);
				darts.DrawObj();
				glBindTexture(GL_TEXTURE_2D, 0);
				glPopMatrix();
			}
			glPushMatrix();

			point = HermitCurve(points[0], SearchVector(points[0], points[1]), points[2], SearchVector(points[2], points[3]), t_max);
			//next_point = HermitCurve(points[0], SearchVector(points[0], points[1]), points[2], SearchVector(points[2], points[3]), t_max_next);
			point.Translated();
			//MoveAxis(point, next_point);
			glBindTexture(GL_TEXTURE_2D, flag);
			glScalef(s1, s1, s1);
			glRotatef(90, 0.0f, 0.0f, -1.0f);
			patr.DrawObj();
			glBindTexture(GL_TEXTURE_2D, 0);

			glPopMatrix();

		//glBegin(GL_LINES);
		////glBindTexture(GL_TEXTURE_2D, red);
		////glEnable(GL_BLEND);
		////glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glColor4f(1.0, 0.0f, 0.0f, 0.1f);
		//glColor3d(1, 0, 0);
		//glVertex3d(0, 0, 2);
		//glVertex3d(30, 0, 2);
		////glBindTexture(GL_TEXTURE_2D, 0);
		//glEnd();


		break;

	case Bezier2:
		DrawBezierCurve2(points[0], points[1], points[2]);

		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, skin);
		glScalef(s2, s2, s2);
		glTranslatef(20.0f, 0.0f, -17.0f);
		glRotatef(90, 0.0f, 1.0f, 0.0f);
		//darts.DrawObj();

		glPopMatrix();;

		glPushMatrix();
		glTranslatef(-33.0f, 0.0f, -12.5f);
		glRotatef(90, 0.0f, 1.0f, 0.0f);
		glRotatef(90, 0.0f, 0.0f, 1.0f);
		//glColor4f(5.0f, 0.2f, 0.7f, 0.9f);
		//pistol.DrawObj();

		glPopMatrix();

		glPushMatrix();
		//point = BezierCurve2(points[0], points[1], points[2], t_max);
		//next_point = BezierCurve2(points[0], points[1], points[2], t_max_next);

		point.Translated();
		//MoveAxis(point, next_point);
		//pistol.DrawObj();

		glPopMatrix();
		break;

	case Bezier3:
		DrawBezierCurve3(points[0], points[1], points[2], points[3]);

		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, skin);
		glScalef(s2, s2, s2);
		glTranslatef(20.0f, 0.0f, -17.0f);
		glRotatef(90, 0.0f, 1.0f, 0.0f);
		//darts.DrawObj();

		glPopMatrix();;

		glPushMatrix();
		glTranslatef(-33.0f, 0.0f, -12.5f);
		glRotatef(90, 0.0f, 1.0f, 0.0f);
		glRotatef(90, 0.0f, 0.0f, 1.0f);
		//glColor4f(5.0f, 0.2f, 0.7f, 0.9f);
		//pistol.DrawObj();

		glPopMatrix();

		glPushMatrix();

		//point = BezierCurve3(points[0], points[1], points[2], points[3], t_max);
		//next_point = BezierCurve3(points[0], points[1], points[2], points[3], t_max_next);

		point.Translated();
		//MoveAxis(point, next_point);
		//patr.DrawObj();

		glPopMatrix();
		break;

	case Linein:
		DrawLine(points[0], points[1]);

		float s1 = 0.4, s2 = 0.1;

		glPushMatrix();

		glBindTexture(GL_TEXTURE_2D, skin);
		glScalef(s2, s2, s2);
		glTranslatef(20.0f, 0.0f, -17.0f);
		glRotatef(90, 0.0f, 1.0f, 0.0f);
		//darts.DrawObj();

		glPopMatrix();;

		glPushMatrix();
		glTranslatef(-33.0f, 0.0f, -12.5f);
		glRotatef(90, 0.0f , 1.0f, 0.0f);
		glRotatef(90, 0.0f, 0.0f, 1.0f);
		//glColor4f(5.0f, 0.2f, 0.7f, 0.9f);
		//pistol.DrawObj();

		glPopMatrix();


		glPushMatrix();

		//point = LinearInterpolation(points[0], points[1], t_max);
		//next_point = LinearInterpolation(points[0], points[1], t_max_next);

		point.Translated();
		//MoveAxis(point, next_point);
		glScalef(s1, s1, s1);
		glRotatef(90, 0.0f, 0.0f, -1.0f);
		//patr.DrawObj();

		glPopMatrix();
		break;
	}
}

//измеряем промежутки времени между отрисовкой
double Search_delta_time() {
	static auto end_render = std::chrono::steady_clock::now();
	auto cur_time = std::chrono::steady_clock::now();
	auto deltatime = cur_time - end_render;
	double delta = 1.0 * std::chrono::duration_cast<std::chrono::microseconds>(deltatime).count() / 1000000;
	end_render = cur_time;
	return delta;
}


double t_gun = 0;

void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	static double t_max = 0;
	static double t_max_next = 0;
	static bool flag_tmax = true;

	//настройка времени
	double delta_time1 = Search_delta_time();
	double go = delta_time1 / 5; //t_max становится = 1 за 5 секунд
	//t_max сама по себе изменяется от 0 до 1 постепенно от кадра к кадру
	//if (flag_tmax) {
		t_max += go;
		t_max_next = t_max + go;
		if (t_max > 1) {
			t_max = 0;
			flag_tmax = false;
		}
		//if (t_max_next > 1) {
		//	t_max_next = 1;
		//}
	//}
	//else {
	//	t_max -= go;
	//	t_max_next = t_max - go;
	//	if (t_max < 0) {
	//		t_max = 0;
	//		flag_tmax = true;
	//	}
	//	if (t_max_next < 0) {
	//		t_max_next = 0;
	//	}
	//}

	//vector<double> t = { t_max, t_max_next };
	double l = 1, s2 = 0.5;
	//Рисуем кривые Эрмита
	//AnimationObjectCurve(vector<Point> {Line[0], Line[1]}, t, l, Linein);

	DrawHermitCurve(PointHB[0], PointHB[1], PointHB[2], PointHB[3]);

	glPushMatrix();

	glBindTexture(GL_TEXTURE_2D, mish);
	//glScalef(s2, s2, s2);
	glTranslatef(35.0f, 0.0f, 0.0f);
	glRotatef(90, 0.0f, -1.0f, 0.0f);
	darts.DrawObj();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();;

	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, skin);
	glTranslatef(-33.0f, 0.0f, -12.5f);
	glRotatef(90, 0.0f, 1.0f, 0.0f);
	glRotatef(90, 0.0f, 0.0f, 1.0f);
	//glColor4f(5.0f, 0.2f, 0.7f, 0.9f);
	pistol.DrawObj();
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();
	if (IsGun) {
		AnimationObjectCurve(vector<Point> {PointHB[0], PointHB[1], PointHB[2], PointHB[3]}, t_gun, l, Hermit);
		t_gun += 0.01;
		if (t_gun >= 1) {
			t_gun = 0;
			IsGun = false;
		}
	}
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	ss << "С - Переключить текстуру" << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}