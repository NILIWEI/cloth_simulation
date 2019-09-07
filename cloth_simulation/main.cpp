/*
	20190521
	��һ��--�������������ҵ
	�����ˣ�nlw
	��Ʒ�����ڵ�������ģ�͵Ĳ��Ϸ��涯��
*/
#ifndef GLUT_DISABLE_ATEXIT_HACK
#define GLUT_DISABLE_ATEXIT_HACK
#endif
#define GLEW_STATIC

#include <GLEW/glew.h>
#include <GLEW/wglew.h>
#include <GLUT/freeglut.h>
#include <vector>
#include<cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //for matrices
#include <glm/gtc/type_ptr.hpp>
//undefine if u want to use the default bending constraint of pbd
#include<iostream>
using namespace std;
//using namespace glm;

GLfloat rtx = 0.0f, rty = 0.0f, rtz = 0.0;

glm::vec3 Up = glm::vec3(0, 1, 0), Right, viewDir;
const int GRID_SIZE = 10;  //�ذ�ש�߳�  
GLdouble MV[16];
GLint viewport[4];
GLdouble PP[16];
bool isfix[1000];

int dist = -23;

//�������  
int iStacks = 30;//ά�ȷ�����߸���
int iSlices = 30;//���ȷ�����߸���
float fRadius = 1;//����ĳ��뾶

float radius = 1;                    //object space radius of ellipsoid  
//float exp1 = 1e-3;  

const int numX = 20, numY = 20; //һ����numx+1����  
const int total_points = (numX + 1)*(numY + 1); //�ܵ���  
//���϶���λ�� �ٶ�  

glm::vec3 Pos[total_points];
glm::vec3 Veloc[total_points];
glm::vec3 force[total_points];
glm::mat4 ellipsoid, invreseEllipsoid;

int size = 4;
float hsize = 4 / 2.0f;
const float frameTime = 1.0f / 60.0f;

const float mass = 1.0 / total_points;
const float globalDamp = 0.98;  //�ٶ�˥������  
const glm::vec3 gvat = glm::vec3(0, -9.8, 0);  //�������ٶ�  
const float Spring_K = 2.5;  //����ϵ��  
const float len0 = 4.0 / numX; //���߳���  
const float tolera = 1.08;   //�����޶�  
float ballMove;//�����˶�  

//�ӽ�����  
int oldX = 0, oldY = 0;
const int width = 1024, height = 1024;
GLdouble P[16];
int selected_index = -1;
int state = 1;
float rX = 15, rY = 0;


void initGL()
{
	//��ʼ������λ��  
	memset(Pos, 0, sizeof(Pos));
	memset(Veloc, 0, sizeof(Veloc));
	memset(force, 0, sizeof(force));
	//fill in positions  
	int count1 = 0;
	int u = numX + 1;
	int v = numY + 1;
	for (int j = 0; j <= numY; j++) {
		for (int i = 0; i <= numX; i++) {
			Pos[count1++] = glm::vec3(((float(i) / (u - 1)) * 2 - 1)* hsize, 4 + 1, ((float(j) / (v - 1)) * 4));
			printf("(%.1lf ,%.1lf)", ((float(i) / (u - 1)) * 2 - 1)* hsize, ((float(j) / (v - 1)) * 4));
		}printf("\n");
		//���ҵ�ΪX[0] �� X[numX]  
	}
	memset(isfix, 0, sizeof(isfix));
	isfix[0] = isfix[numX] = 1;
	ballMove = 0;


}
void DrawEllipsoid() {//����ɫ����  
	//��������  
	ellipsoid = glm::translate(glm::mat4(1), glm::vec3(0, ballMove, 0));
	ballMove += 0.01;
	if (ballMove >= 4.0)ballMove = -1;
	ellipsoid = glm::rotate(ellipsoid, 45.0f, glm::vec3(1, 0, 0));
	ellipsoid = glm::scale(ellipsoid, glm::vec3(fRadius, fRadius, fRadius / 2));
	invreseEllipsoid = glm::inverse(ellipsoid);

	ballMove += 0.005;

	//��������  
	glColor3f(0, 1, 0);
	glPushMatrix();
	glMultMatrixf(glm::value_ptr(ellipsoid));
	//�뾶������������ά������  
	glutWireSphere(fRadius, iSlices, iStacks);
	glPopMatrix();
}
void DrawGrid()  //���ذ�  
{
	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f);
	for (int i = -GRID_SIZE; i <= GRID_SIZE; i++)
	{
		glVertex3f((float)i, 0, (float)-GRID_SIZE);
		glVertex3f((float)i, 0, (float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE, 0, (float)i);
		glVertex3f((float)GRID_SIZE, 0, (float)i);
	}
	glEnd();
}

void drawTextile() {  //������  
	//draw polygons  
	int k = 0;
	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	for (int i = 0; i <= numX; i++) {
		for (int j = 0; j <= numY; j++) {
			//                cout<<i<<" "<<j<<";";  
			if (j != numX) {
				glm::vec3 p1 = Pos[k];
				glm::vec3 p2 = Pos[k + 1];
				glVertex3f(p1.x, p1.y, p1.z);
				glVertex3f(p2.x, p2.y, p2.z);
				//                printf("add %d %d\n",k,k+1);  
			}
			if (i != numY) {
				glm::vec3 p1 = Pos[k];
				glm::vec3 p2 = Pos[k + numX + 1];
				glVertex3f(p1.x, p1.y, p1.z);
				glVertex3f(p2.x, p2.y, p2.z);
			}
			k++;
		}
		//        cout<<endl;  
	}
	glEnd();

	glPointSize(3);
	glBegin(GL_POINTS);
	glColor3f(1, 0, 0);
	for (size_t i = 0; i < total_points; i++) {
		glm::vec3 p = Pos[i];
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();
}

void OnRender(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//�����ӽ�  
	glTranslatef(0, 0, dist);
	//  glRotatef(15,1,0,0);  
	glRotatef(rX, 1, 0, 0);
	glRotatef(rY, 0, 1, 0);
	glGetDoublev(GL_MODELVIEW_MATRIX, MV);
	viewDir.x = (float)-MV[2];
	viewDir.y = (float)-MV[6];
	viewDir.z = (float)-MV[10];
	Right = glm::cross(viewDir, Up);
	//��������  
	DrawGrid();
	drawTextile();
	DrawEllipsoid();
	//    drawblock();  
	glutSwapBuffers();
}
//���b���a���������  
glm::vec3 SpringForce(int a, int b, int is) {
	float inilen;
	if (is == 1)inilen = len0;
	if (is == 2) inilen = len0 * 2;
	if (is == 3) inilen = len0 * 1.414213;
	glm::vec3 res = glm::vec3(0);
	glm::vec3 tmp = Pos[b] - Pos[a];
	float dis = glm::length(tmp);
	res = tmp / dis;
	res *= (dis - inilen);
	res *= Spring_K;
	return res;
}
void ComputeForces() {
	//����  
	for (size_t i = 0; i < total_points; i++) {
		force[i] = glm::vec3(0);
		if (!isfix[i])
			force[i] += mass * gvat;
	}
	int i, j, k = 0;
	//�ṹ���� ��������  
	for (i = 0; i <= numY; i++) {
		for (j = 0; j <= numX; j++) {
			if (i != 0) {//��  
				force[k] += SpringForce(k, k - numX - 1, 1);
			}
			if (j != numX) {//��  
				force[k] += SpringForce(k, k + 1, 1);
			}
			if (i != numY) {//��  
				force[k] += SpringForce(k, k + numX + 1, 1);
			}
			if (j != 0) {//��  
				force[k] += SpringForce(k, k - 1, 1);
			}
			k++;
		}
	}
	//���Ե���  
	k = 0;
	for (i = 0; i <= numY; i++) {
		for (j = 0; j <= numX; j++) {
			if (i > 1) {//��  
				force[k] += SpringForce(k, k - 2 * numX - 2, 2);
			}
			if (j < numX - 1) {//��  
				force[k] += SpringForce(k, k + 2, 2);
			}
			if (i < numY - 1) {//��  
				force[k] += SpringForce(k, k + 2 * numX + 2, 2);
			}
			if (j > 1) {//��  
				force[k] += SpringForce(k, k - 2, 2);
			}
			k++;
		}
	}
	//���е���  
	k = 0;
	for (i = 0; i <= numY; i++) {
		for (j = 0; j <= numX; j++) {
			if (i > 0 && j > 0) {  //����  
				force[k] += SpringForce(k, k - numX - 2, 3);
			}
			if (i > 0 && j < numX) {  //����  
				force[k] += SpringForce(k, k - numX, 3);
			}
			if (i < numY&&j < numX) {  //����  
				force[k] += SpringForce(k, k + numX + 2, 3);
			}
			if (i < numY&&j>0) {   //����  
				force[k] += SpringForce(k, k + numX, 3);
			}
			k++;
		}
	}
}

void EllipsoidCollision() { //����봦���Ϻ�����֮�����ײ  
	for (size_t i = 0; i < total_points; i++) {
		glm::vec3 tPos(Pos[i]);
		glm::vec4 X_0 = (invreseEllipsoid*glm::vec4(tPos, 1));
		glm::vec3 delta0 = glm::vec3(X_0.x, X_0.y, X_0.z);
		float distance = glm::length(delta0);
		if (distance < 1.0f) {
			delta0 = (radius - distance) * delta0 / distance;
			// Transform the delta back to original space  
			glm::vec3 delta;
			glm::vec3 transformInv;
			transformInv = glm::vec3(ellipsoid[0].x, ellipsoid[1].x, ellipsoid[2].x);
			transformInv /= glm::dot(transformInv, transformInv);
			delta.x = glm::dot(delta0, transformInv);
			transformInv = glm::vec3(ellipsoid[0].y, ellipsoid[1].y, ellipsoid[2].y);
			transformInv /= glm::dot(transformInv, transformInv);
			delta.y = glm::dot(delta0, transformInv);
			transformInv = glm::vec3(ellipsoid[0].z, ellipsoid[1].z, ellipsoid[2].z);
			transformInv /= glm::dot(transformInv, transformInv);
			delta.z = glm::dot(delta0, transformInv);
			tPos += delta;
			Veloc[i] += (tPos - Pos[i]) / frameTime;
			Veloc[i] *= globalDamp;
			Pos[i] = tPos;
		}
	}
}

void CalcPos() {  //�����µ�λ��  
//  frameTime/=1000;  
//  printf("%.3lf\n",frameTime);  
	glm::vec3 acc = glm::vec3(0);
	//�õ���frame���Ǽ��ʱ�� ����  
	for (size_t i = 0; i < total_points; i++) {
		if (isfix[i])continue;
		acc = force[i] / mass;  //�õ����ٶ�����  
		Veloc[i] = Veloc[i] + acc * frameTime;  //�õ��µ��ٶ�ֵ  
		Veloc[i] *= globalDamp;
		Pos[i] = Pos[i] + Veloc[i] * frameTime;
	}
}

void StepPhysics() {
	ComputeForces();
	CalcPos();
	EllipsoidCollision();
	glutPostRedisplay();
	Sleep(5);
}


void OnReshape(int nw, int nh) {
	glViewport(0, 0, nw, nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 1.f, 100.0f);

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_PROJECTION_MATRIX, PP);

	glMatrixMode(GL_MODELVIEW);
}

void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
		int window_y = (height - y);
		float norm_y = float(window_y) / float(height / 2.0);
		int window_x = x;
		float norm_x = float(window_x) / float(width / 2.0);

		float winZ = 0;
		glReadPixels(x, height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		if (winZ == 1)
			winZ = 0;
		double objX = 0, objY = 0, objZ = 0;
		gluUnProject(window_x, window_y, winZ, MV, P, viewport, &objX, &objY, &objZ);
		glm::vec3 pt(objX, objY, objZ);
		size_t i = 0;
		for (i = 0; i < total_points; i++) {
			if (glm::distance(Pos[i], pt) < 0.1) {
				selected_index = i;
				printf("Intersected at %d\n", i);
				break;
			}
		}
	}

	if (button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;

	if (s == GLUT_UP) {
		selected_index = -1;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void OnMouseMove(int x, int y)
{
	if (selected_index == -1) {
		if (state == 0)
			dist *= (1 + (y - oldY) / 60.0f);
		else
		{
			rY += (x - oldX) / 5.0f;
			rX += (y - oldY) / 5.0f;
		}
	}
	else {
		float delta = 1500 / abs(dist);
		float valX = (x - oldX) / delta;
		float valY = (oldY - y) / delta;
		if (abs(valX) > abs(valY))
			glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
		else
			glutSetCursor(GLUT_CURSOR_UP_DOWN);

		Veloc[selected_index] = glm::vec3(0);
		Pos[selected_index].x += Right[0] * valX;
		float newValue = Pos[selected_index].y + Up[1] * valY;
		if (newValue > 0)
			Pos[selected_index].y = newValue;
		Pos[selected_index].z += Right[2] * valX + Up[2] * valY;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}
int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("zf1821212_����ΰ");
	
	initGL();
	glutDisplayFunc(OnRender);
	//ָ��������״�仯ʱ�Ļص�����  
	glutReshapeFunc(OnReshape);
	//ָ���������ʱ���ú���  
	glutIdleFunc(StepPhysics);
	glEnable(GL_DEPTH_TEST);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutMainLoop();
	return 0;
}