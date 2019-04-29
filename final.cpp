#include <GL/glut.h>
#include<iostream>
#include<math.h>
#include<vector>
#include<map>
using namespace std;

int mainmenu, drawmenu,transformmenu,bresenhammenu,freemenu;
int openmenu = 0;
int menuflag = 0;
int menuflag2 = 0;
int menuflag3 = 0;
int xs[100000], ys[100000];
int beginx, beginy;
int total = 0;
bool checkpassive = true;
bool checkkeyboard = false;
bool checkmotion = false;
bool motiondo = false;
bool freeover = false;
double scalerate = 1;
double rotaterate = 1;
double shearx = 0;
double sheary = 0;
int length= 500;
int width = 500;

# define DRAW 1
# define TRANSFORM 2
#define BRESENHAM 3
#define SQUARE 4
#define PENTAGRAM 5
#define HEXAGON 6
#define MOVE 7
#define ROTATE 8
#define SCALE 9
#define SHEAR 10
#define COLOR 11
#define LINE 12
#define CIRCLE 13
#define FREE 14
#define pi 3.14

struct object {
	int objx[1000];
	int objy[1000];
	int objtotal = 0;
};
struct rotatecenter {
	int x0;
	int y0;
	bool done = false;
};

struct bucket {
	int yup;
	float xlow;
	float rk;
	bucket(int* x1, int* y1, int* x2, int* y2) {
		yup = (*y1 > *y2) ? *y1 : *y2;
		if (yup == *y1) {
			xlow = (float)*x2;
		}
		else {
			xlow = (float)*x1;
		}
		rk = (float)(*x2 - *x1) / (float)(*y2 - *y1);
	}
	void show() {
		cout << "xlow " << xlow << endl;
		cout << "yup " << yup << endl;
		cout << "1/k" << rk << endl;
	}
};

object myobj;
object orign;
rotatecenter mycenter;

typedef map<int, vector<bucket>> ET;
ET edgetable;
typedef multimap<float, bucket*>  AET;
AET Aedgetable;
ET::iterator i1;
AET::iterator i2;

void paint(float xbegin, float xend, int y) {  //利用参数区别于无参数的大paint函数
	int x1 = (int)(xbegin + 0.5);
	int x2 = (int)(xend + 0.5);
	glBegin(GL_LINES);  //绘制直线
	{
		glVertex2i(x1, y);
		glVertex2i(x2, y);
	}
	glEnd();
	glFlush();   //单缓存直接输出画像到屏幕
}  

void Polygonfill() {
	i1 = edgetable.begin();
	i2 = Aedgetable.begin();
	int miny = i1->first;
	int y = miny;
	while (Aedgetable.size() > 0) {
		for (i2 = Aedgetable.begin(); i2 != Aedgetable.end(); i2++) {
			float xbegin = i2->first;
			i2++;
			float xend = i2->first;
			paint(xbegin, xend, y);
		}
		for (i2 = Aedgetable.begin(); i2 != Aedgetable.end();) {
			if (i2->second->yup == y + 1) {
				i2 = Aedgetable.erase(i2);
			}
			else {
				i2++;                          //分开写是为了让i2迭代器始终指向有意义的地址。因为当erase后整个容器的数据地址会移动
			}
		}
		vector<bucket*>bs;
		for (i2 = Aedgetable.begin(); i2 != Aedgetable.end(); i2++) {
			i2->second->xlow += i2->second->rk;
			bs.push_back(i2->second);
		}
		Aedgetable.clear();
		for (int i = 0; i < bs.size(); i++) {
			Aedgetable.insert(make_pair(bs[i]->xlow, bs[i]));
		}
		y++;
		i1 = edgetable.find(y);
		if (i1 != edgetable.end()) {
			for (int i = 0; i < i1->second.size(); i++) {
				Aedgetable.insert(make_pair(i1->second[i].xlow, &i1->second[i]));
			}
		}
	}
}

void bresenhamline(int x0, int y0, int x1, int y1) {
	int dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	int x = x0;
	int y = y0;
	int xxs[100000], yys[100000];
	int num=0;
	if (dy > 0 && dx >= dy) {
		num = dx + 1;
		int p1 = 2 * dy - dx;
		for (int i = 0; i < num; i++) {
			xxs[i] = x;
			yys[i] = y;
			if (p1 >= 0) {
				p1 = p1 + 2 * (dy - dx);
				y++;
			}
			else {
				p1 = p1 + 2 * dy;
			}
			x++;
		}
	}
	else if (dy > 0 && dx < dy) {
		num = dy + 1;
		int p1 = 2 * dx - dy;
		for (int i = 0; i < num; i++) {
			xxs[i] = x;
			yys[i] = y;
			if (p1 >= 0) {
				p1 = p1 + 2 * (dx - dy);
				x++;
			}
			else {
				p1 = p1 + 2 * dx;
			}
			y++;
		}
	}
	else if (dy < 0) {
		int ddy = y0 - y1;
		if (dx >= ddy) {
			num = dx + 1;
			int p1 = 2 * dy - dx;
			for (int i = 0; i < num; i++) {
				xxs[i] = x;
				yys[i] = y;
				if (p1 <= 0) {
					p1 = p1 + 2 * (dy + dx);
					y--;
				}
				else {
					p1 = p1 + 2 * dy;
				}
				x++;
			}
		}
		else if (dx < ddy) {
			num = ddy + 1;
			int p1 = 2 * dx - ddy;
			for (int i = 0; i < num; i++) {
				xxs[i] = x;
				yys[i] = y;
				if (p1 >= 0) {
					p1 = p1 + 2 * (dx - ddy);
					x++;
				}
				else {
					p1 = p1 + 2 * dx;
				}
				y--;
			}
		}
	}

	for (int i = 0; i < num; i++) {
		glBegin(GL_POINTS);
			glVertex2f(xxs[i], yys[i]);
		glEnd();
	}
}

void bresenhamcircle(int x0, int y0, int r) {
	int num = 0;
	int xxs[100000], yys[100000];
	int p = 3 - 2 * r;
	int y = r;
	for (int x = 0; x <= y; x++) {
		xxs[num] = x;
		yys[num] = y;
		if (p >= 0) {
			p += 4 * (x - y) + 10;
			y--;
		}
		else {
			p += 4 * x + 6;
		}
		num++;
	}
	for (int i = 0; i < num; i++) {
		xxs[num + i] = yys[i];
		yys[num + i] = xxs[i];
		xxs[2 * num + i] = yys[i];
		yys[2 * num + i] = -xxs[i];
		xxs[3 * num + i] = xxs[i];
		yys[3 * num + i] = -yys[i];
		xxs[4 * num + i] = -xxs[i];
		yys[4 * num + i] = -yys[i];
		xxs[5 * num + i] = -yys[i];
		yys[5 * num + i] = -xxs[i];
		xxs[6 * num + i] = -yys[i];
		yys[6 * num + i] = xxs[i];
		xxs[7 * num + i] = -xxs[i];
		yys[7 * num + i] = yys[i];
	}
	for (int i = 0; i < 8 * num; i++) {
		xxs[i] += x0;
		yys[i] +=y0;
		glBegin(GL_POINTS);
			glVertex2f(xxs[i], yys[i]);
		glEnd();
	}
	/*cout << "1/8 points of this circle are: " << endl;
	for (int i = 0; i < num; i++) {
		cout << xs[i] << "," << ys[i] << endl;
	}*/
	return;
}


void build() {
	for (int i = 0; i < myobj.objtotal - 1; i++) {
		if (myobj.objy[i] == myobj.objy[i + 1]) {
				continue;
			}
			bucket buc(&myobj.objx[i], &myobj.objy[i], &myobj.objx[i + 1], &myobj.objy[i + 1]);
			int ymin = (myobj.objy[i] < myobj.objy[i + 1]) ? myobj.objy[i] : myobj.objy[i + 1];
			if (edgetable.find(ymin) == edgetable.end()) {
				edgetable.insert(make_pair(ymin, vector<bucket>(1, buc))); //make_pair是产生一个Pair(相当于一个有两个元素的结构体）的函数，vector<T>(a,b)是初始化的方式，a数量b初始值
			}
			else {
				edgetable.find(ymin)->second.push_back(buc);
			}
		}
		if (myobj.objy[myobj.objtotal - 1] != myobj.objy[0]) {
		bucket buc(&myobj.objx[myobj.objtotal - 1], &myobj.objy[myobj.objtotal - 1], &myobj.objx[0], &myobj.objy[0]);
		int ymin = (myobj.objy[myobj.objtotal - 1] < myobj.objy[0]) ? myobj.objy[myobj.objtotal - 1] : myobj.objy[0];
		if (edgetable.find(ymin) == edgetable.end()) {
			edgetable.insert(make_pair(ymin, vector<bucket>(1, buc))); //make_pair是产生一个Pair(相当于一个有两个元素的结构体）的函数，vector<T>(a,b)是初始化的方式，a数量b初始值
		}
		else {
			edgetable.find(ymin)->second.push_back(buc);
		}
		}
		if (myobj.objtotal > 2) {    //在至少有三条边的情况下，用Aedgetable开始存ymin最小的所有边
			i1 = edgetable.begin();
			for (int i = 0; i < i1->second.size(); i++) {    //单独取出ymin相同的每一个bucket
				Aedgetable.insert(make_pair(i1->second[i].xlow, &i1->second[i]));  //Aedgetable会自动按第一个参数从小到大排序
			}
		}
	}

void init() {
	menuflag2 = 0;
	checkpassive = true;
	checkkeyboard = false;
	checkmotion = false;
	mycenter.done = false;
	motiondo = false;
	freeover = false;
	cin.clear();
}

void createobj() {
	switch (menuflag) {
	case SQUARE: {
		myobj.objx[0] = xs[0];
		myobj.objy[0] = ys[0];
		myobj.objx[1] = xs[1];
		myobj.objy[1] = ys[0];
		myobj.objx[2] = xs[1];
		myobj.objy[2] = ys[1];
		myobj.objx[3] = xs[0];
		myobj.objy[3] = ys[1];
		myobj.objtotal = 4;
		break;
	}
	case PENTAGRAM: {
		myobj.objtotal =10;
		for (int i = 0; i < 10; i++) {
			myobj.objx[i] = xs[i+1];
			myobj.objy[i] = ys[i+1];
		}
		break;
	}
	case HEXAGON: {
		myobj.objtotal = 6;
		int xadd = (xs[1] - xs[0])*sin(pi / 6);
		int ymid = (ys[0] + ys[1]) / 2;
		myobj.objx[0] = xs[0];
		myobj.objy[0] = ys[0];
		myobj.objx[1] = xs[1];
		myobj.objy[1] = ys[0];
		myobj.objx[2] = xs[1] + xadd;
		myobj.objy[2] = ymid;
		myobj.objx[3] = xs[1];
		myobj.objy[3] = ys[1];
		myobj.objx[4] = xs[0];
		myobj.objy[4] = ys[1];
		myobj.objx[5] = xs[0] - xadd;
		myobj.objy[5] = ymid;
		break;
	}
	case LINE: {
		if (xs[0] <= xs[1]) {
			myobj.objx[0] = xs[0];
			myobj.objy[0] = ys[0];
			myobj.objx[1] = xs[1];
			myobj.objy[1] = ys[1];
		}
		else {
			myobj.objx[0] = xs[1];
			myobj.objy[0] = ys[1];
			myobj.objx[1] = xs[0];
			myobj.objy[1] = ys[0];
		}
		break;
	}
	case FREE:{
		myobj.objtotal = total;
		for (int i = 0; i < total; i++) {
			myobj.objx[i] = xs[i];
			myobj.objy[i] = ys[i];
		   }
		  break;
	  }
	}
}

void paint() {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0,0.0, 0.0);
	if ((menuflag2 == ROTATE||menuflag2==SCALE||menuflag2==SHEAR)&&mycenter.done==true) {
		glRectf(mycenter.x0,mycenter.y0, mycenter.x0+5, mycenter.y0+5);
	}
	if (menuflag2 == 0&&menuflag!=0) {
		createobj();
	}
	if (menuflag == SQUARE) {
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < myobj.objtotal; i++) {
				glVertex2i(myobj.objx[i], myobj.objy[i]);
			}
			glEnd();
	}
	else if (menuflag == PENTAGRAM) {
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 10; i++) {
			glVertex2i(myobj.objx[i], myobj.objy[i]);
		}
		glEnd();
	}
	else if (menuflag == HEXAGON) {
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < myobj.objtotal; i++) {
				glVertex2i(myobj.objx[i], myobj.objy[i]);
			}
			glEnd();
		}
	else if (menuflag == LINE) {
		bresenhamline(myobj.objx[0], myobj.objy[0], myobj.objx[1], myobj.objy[1]);
	}
	else if (menuflag == CIRCLE) {
		double r;
		r = sqrt((xs[1] - xs[0])*(xs[1] - xs[0]) + (ys[1] - ys[0])*(ys[1] - ys[0]));
		bresenhamcircle(xs[0], ys[0], r);
	}
	else if (menuflag == FREE) {
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < myobj.objtotal; i++) {
			glVertex2i(myobj.objx[i], myobj.objy[i]);
		}
		glEnd();
	}
	glFlush();
}

void change(int w,int h) {
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluOrtho2D(0, length, width,0);
//	gluPerspective(45, ratio,5, 500);
	glMatrixMode(GL_MODELVIEW);
}

void mouseButton(int button, int state, int x, int y) {
	if (menuflag != 0 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			if (total == 0) {
				xs[total] = x;
				ys[total] = y;
				total++;
				checkkeyboard = true;
			}
			else {
				if ((menuflag2 == ROTATE||menuflag2==SCALE||menuflag2==SHEAR)&&mycenter.done==false) {
					mycenter.x0 = x;
					mycenter.y0 = y;
					mycenter.done = true;
					glutPostRedisplay();
				}
				checkpassive = false;
				checkkeyboard = false;
				if (menuflag == FREE&&freeover==false) {
					xs[total] = x;
					ys[total] = y;
					total++;
					glutPostRedisplay();
					checkkeyboard = true;
				}
			/*	if (menuflag == LINE || menuflag == CIRCLE) {
					glutPostRedisplay();
				}*/
			}
		}
	 if (checkmotion == true) {
		 if (state == GLUT_DOWN) {
			 if (motiondo == false) {
				 motiondo = true;
				 orign.objtotal = myobj.objtotal;
				 for (int i = 0; i < myobj.objtotal; i++) {
					 orign.objx[i] = myobj.objx[i];
					 orign.objy[i] = myobj.objy[i];
				 }
				 beginx = x;
				 beginy = y;
			 }
		 }
		 else if (state == GLUT_UP) {
			 if (motiondo == true) {
				 scalerate = 1;
				 rotaterate = 1;
				 motiondo = false;
			 }
		 }
	}
	}

void processmenustatus(int status, int x, int y) {
	if (status == GLUT_MENU_IN_USE) {
		openmenu = 1;
	}
	else {
		openmenu = 0;
	}
}

void processpassive(int x, int y) {   
	if (menuflag== SQUARE&&checkpassive==true) {
		if (total==1) {
				if (y <ys[0]) {
					xs[1] = x;
					ys[1] = ys[0] - abs(x - xs[0]);
				}
				else {
					xs[1] = x;
					ys[1] = ys[0] + abs(x - xs[0]);
				}
				if (ys[1] < 0) {
					ys[1] = 0;
					if (x > xs[0]) {
						xs[1] = xs[0] + ys[0];
					}
					else{
						xs[1] = xs[0] - ys[0];
					}
				}
				else if (ys[1] > 500) {
					ys[1] =width;
					if (x > xs[0]) {
						xs[1] = xs[0] +ys[1]-ys[0];
					}
					else {
						xs[1] = xs[0] - (ys[1]-ys[0]);
					}
				}
			glutPostRedisplay();
		}
	}

	else if (menuflag == PENTAGRAM && checkpassive == true) {
		if (total == 1) {
			double r = abs(x - xs[0]);
			double rr = r / 2*cos(pi/5);
			for (int i = 0; i < 10;i+=2) {
				xs[i + 1] = xs[0]-r * sin(2 * pi*i / 10);
				xs[i + 2] = xs[0] - rr * sin(2 * pi*i / 10 + pi / 5);
				ys[i + 1] = ys[0]-r * cos(2 * pi*i / 10);
				ys[i + 2] =ys[0] - rr *cos(2 * pi*i / 10 + pi / 5);
			 }
			glutPostRedisplay();
		}
	}

	else if (menuflag == HEXAGON && checkpassive == true) {
		if (total == 1) {
			xs[1] = x;
		 if (y > ys[0]) {
				ys[1] = ys[0]+2*cos(pi/6)*abs(xs[1] - xs[0]);
			}
			else {
				ys[1] = ys[0] -2*cos(pi/6)* abs(xs[1] - xs[0]);
			}
			glutPostRedisplay();
		}
	}
	else if ((menuflag == LINE||menuflag==CIRCLE) && checkpassive == true) {
		if (total == 1) {
			xs[1] = x;
			ys[1] = y;
			glutPostRedisplay();
		}
	}

}  

void processmotion(int x, int y) {
	if (checkmotion == true) {
		switch (menuflag2) {
		case MOVE: {
			     int deltax = x - myobj.objx[0];
				 int deltay = y - myobj.objy[0];
				 int rate =abs(deltax);
				 if (abs(deltay) <abs(deltax)) {
					 rate = abs(deltay);
				 }
				 if (rate == 0) {
					 rate = 1;
				 }
				for (int i = 0; i < myobj.objtotal; i++) {
					//myobj.objx[i] += deltax/rate;
					//myobj.objy[i] += deltay/rate;
					myobj.objx[i] += deltax ;
					myobj.objy[i] += deltay;
				}
			glutPostRedisplay();
			break;
		}
		case ROTATE: {
			if (mycenter.done == false) {
				return;
			}
			if (x > beginx)
				rotaterate++;
			else
				rotaterate--;
			double a = rotaterate * pi / 72;
			for (int i = 0; i < myobj.objtotal; i++) {
				int oldx = orign.objx[i];
				int oldy = orign.objy[i];
		    	myobj.objx[i] = oldx*cos(a) - oldy*sin(a) +mycenter.x0*(1-cos(a))+mycenter.y0*sin(a);
		       myobj.objy[i] =oldx*sin(a)+oldy*cos(a)+mycenter.y0*(1-cos(a))-mycenter.x0*sin(a);
			}
			glutPostRedisplay();
			break;
		}
		case SCALE: {
			if (mycenter.done == false) {
				return;
			}
			if (x >beginx) {
				scalerate += 0.01;
			}
			else {
				scalerate -= 0.01;
			}
			for (int i = 0; i < myobj.objtotal; i++) {
				int oldx = orign.objx[i];
				int oldy = orign.objy[i];
				myobj.objx[i] = oldx * scalerate + mycenter.x0*(1 - scalerate);
				myobj.objy[i] = oldy * scalerate+mycenter.y0 * (1-scalerate) ;
			}
			glutPostRedisplay();
			break;
			break;
		}
		case SHEAR: {
			if (mycenter.done == false) {
				return;
			}
			shearx = (double)(x - beginx)/200;
			sheary = (double)(y - beginy) / 200;
			for (int i = 0; i < myobj.objtotal; i++) {
				int oldx = orign.objx[i];
				int oldy = orign.objy[i];
				myobj.objx[i] = oldx + shearx * oldy-mycenter.y0*shearx;
				myobj.objy[i] = oldx * sheary + oldy-mycenter.x0*sheary;
			}
			glutPostRedisplay();
			break;
		}
		}

	}
}

void processmainmenu(int option) {
	switch (option) {
	case COLOR: {
		if (menuflag != 0) {
			if (menuflag == LINE || menuflag == CIRCLE||myobj.objtotal<3) {
				cout << "can not color!" << endl;
				return;
			}
			cout << "color" << endl;
			edgetable.clear();
			Aedgetable.clear();
			build();
			Polygonfill();
		}
		break;
	}
	case FREE: {
		init();
		total = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		cout << "free" << endl;
		menuflag =FREE;
		myobj.objtotal = 0;
		break;
	}
	}
}

void processdrawmenu(int option) {
	switch (option) {
	case SQUARE: {
		init();
		total = 0;
		glClear(GL_COLOR_BUFFER_BIT); 
		glFlush();
		cout << "square" << endl;
		menuflag = SQUARE;
		myobj.objtotal = 0;
		break;
	}
	case PENTAGRAM: {
		init();
		total = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		cout << "pentagram" << endl;
		menuflag = PENTAGRAM;
		myobj.objtotal = 0;
		break;
	}
	case HEXAGON: {
		init();
		total = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		cout << "hexagon" << endl;
		menuflag = HEXAGON;
		myobj.objtotal = 0;
		break;
	}
	}
}

void processtransformmenu(int option) {
	if (menuflag == LINE || menuflag == CIRCLE) {
		cout << "can not transform" << endl;
		return;
	}
	switch (option) {
	case MOVE: {
		init();
		checkmotion = true;
		cout << "move" << endl;
		menuflag2 = MOVE;
		break;
	}
	case ROTATE: {
		init();
		checkmotion = true;
		cout << "rotate" << endl;
		menuflag2 = ROTATE;
		break;
	}
	case SCALE: {
		init();
		checkmotion = true;
		cout << "scale" << endl;
		menuflag2 = SCALE;
		break;
	}
	case SHEAR: {
		init();
		checkmotion = true;
		cout << "shear" << endl;
		menuflag2 = SHEAR;
		break;
	}
	}
}

void processbresenham(int option) {
	switch (option) {
	case LINE: {
		init();
		total = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		cout << "line" << endl;
		menuflag = LINE;
		break;
	}
	case CIRCLE: {
		init();
		total = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		cout << "circle" << endl;
		menuflag = CIRCLE;
		break;
	}
	}
}

void menus() {
	drawmenu = glutCreateMenu(processdrawmenu);
	glutAddMenuEntry("square", SQUARE);
	glutAddMenuEntry("pentagram", PENTAGRAM);
	glutAddMenuEntry("hexagon", HEXAGON);

	transformmenu = glutCreateMenu(processtransformmenu);
	glutAddMenuEntry("move", MOVE);
	glutAddMenuEntry("rotate", ROTATE);
	glutAddMenuEntry("scale", SCALE);
	glutAddMenuEntry("shear", SHEAR);

	bresenhammenu = glutCreateMenu(processbresenham);
	glutAddMenuEntry("line", LINE);
	glutAddMenuEntry("circle", CIRCLE);

	mainmenu = glutCreateMenu(processmainmenu);
	glutAddSubMenu("drawmenu", DRAW);
	glutAddSubMenu("transformmenu", TRANSFORM);
	glutAddSubMenu("bressenham", BRESENHAM);
	glutAddMenuEntry("free", FREE);
	glutAddMenuEntry("color", COLOR);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutMenuStatusFunc(processmenustatus);
}

void processNormalKeys(unsigned char key, int x, int y) {
	if (key == 27) {
		exit(0);
	}
}

void processSpecialKeys(int key, int x, int y) {
	if (key == GLUT_KEY_F1 &&checkkeyboard==true) {
		switch (menuflag) {
		case SQUARE:
			cout << "please enter the length of the square:" << endl;
			break;
		case PENTAGRAM:
			cout << "please enter the distance from the center point:" << endl;
			break;
		case HEXAGON:
			cout << "please enter the length of the hexagon:" << endl;
			break;
		}
			int a ;
			cin >> a;
			switch (menuflag) {
			case SQUARE: {
				xs[1] = xs[0] + a;
				ys[1] = ys[0] - a;
				glutPostRedisplay();
				break;
			}
			case PENTAGRAM: {
				double r = a;
				double rr = r / 2 * cos(pi / 5);
				for (int i = 0; i < 10; i += 2) {
					xs[i + 1] = xs[0] - r * sin(2 * pi*i / 10);
					xs[i + 2] = xs[0] - rr * sin(2 * pi*i / 10 + pi / 5);
					ys[i + 1] = ys[0] - r * cos(2 * pi*i / 10);
					ys[i + 2] = ys[0] - rr * cos(2 * pi*i / 10 + pi / 5);
				}
				glutPostRedisplay();
				break;
			}
			case HEXAGON: {
				xs[1] = xs[0] + a;
				ys[1] = ys[0] - 2 * a*cos(pi / 6);
				glutPostRedisplay();
				break;
			}
			}
		checkkeyboard = false;
		checkpassive = false;
	}
	else if (key == GLUT_KEY_F2 && checkkeyboard == true) {
		freeover = true;
		//glutPostRedisplay();
		checkkeyboard = false;
		checkpassive = false;
	}
}

int main(int argc,char** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100); //窗口位置的设置
	glutInitWindowSize(length, width);  //窗口大小的设置
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);  //设置为单缓存模式
	glutCreateWindow("final");
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glutReshapeFunc(change);
	glutDisplayFunc(paint);

	glutMouseFunc(mouseButton);
	glutPassiveMotionFunc(processpassive);
	glutMotionFunc(processmotion);

	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);

	menus();

	glutMainLoop();
	return 0;
}