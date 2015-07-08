/****************************************************************************

/* This is a 3D mesh viewer, you can import a . 3D mesh file,you can draw axe, grid and bounding box and choose the display mode such as wireframe or smooth shading
   you can also rotate, translate, zoom in/out the model and change the backgroud color as well as the color
   of the model, you can choose two kind of projection model, Orthogonal or Perspective. 
                             Author:Zhou Guoyan, 2014 

****************************************************************************/
#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glui.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <list>
#include <set>
using namespace std;
GLUquadricObj *quadratic;
float xy_aspect,axe_x=1.5f,axe_y=1.5,axe_z=1.5f; 
int   last_x, last_y;
float rotationX = 0.0, rotationY = 0.0;
GLfloat x_angle_model=0,y_angle_model=0,model_x=0,model_y=0,model_z=0,scale_size_model=1,old_size;
#define PERSPECTIVE          501
#define ORTHOGONAL           502
#define SMOOTH_MODEL		 401
#define FLAT_MODEL			 402
#define POINTS_MODEL		 403
#define WIRE_MODEL			 404
#define VIEW_ROTATE			 600
#define MODEL_ROTATE		 700
/** These are the live variables passed into GLUI ***/
int load=0,reload=0;
int trans_model_type;
int   obj_type=SMOOTH_MODEL;
int   camera_type=PERSPECTIVE;  
int   light0_enabled = 1;
int   light1_enabled = 1;
float light0_intensity = 1.0;
float light1_intensity = .4;
int   main_window;
float scale = 1.0;
int   hide_model=0;
int   show_axes = 1;
int   show_grid=1;
int   show_box=1;

float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
//float lights_rotation[16]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 } ;

float obj_pos[] = { 0.0, 0.0, 0.0 };
int   curr_string = 0;
GLfloat r = 1.0f, g = 0.0f, b = 1.0f;
int transform_view;
int transform_model;
/** Pointers to the windows and some of the controls we'll create **/
GLUI *glui, *glui2;
GLUI_Spinner    *light0_spinner, *light1_spinner;
GLUI_RadioGroup *radio;
GLUI_RadioGroup	*obj_type_radio;
GLUI_RadioGroup	*camera_type_radio;
GLUI_Panel      *obj_panel;
GLUI_Panel      *camera_panel;
typedef struct HE_face;
typedef struct HE_vertex;
typedef struct HE_edge;

/********** User IDs for callbacks ********/
#define COLOR_ID             100
#define IMPORT_ID            150
#define LIGHT0_ENABLED_ID    200
#define LIGHT1_ENABLED_ID    201
#define LIGHT0_INTENSITY_ID  250
#define LIGHT1_INTENSITY_ID  260
#define ENABLE_ID            300
#define DISABLE_ID           301
#define SHOW_ID              302
#define HIDE_ID              303
#define PROPERTY_ID          400
#define CAMERA_ID            500
#define MODEL_ROTATION       60
#define MODEL_TRANSLATION    80
#define MODEL_SCALE          90
#define MODEL_TRANS_NONE     10



/********** Miscellaneous global variables **********/

GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
GLfloat light0_position[] = {.5f, .5f, 1.0f, 0.0f};
GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
GLfloat light1_diffuse[] =  {.9f, .6f, 0.0f, 1.0f};
GLfloat light1_position[] = {-1.0f, -1.0f, 1.0f, 0.0f};
GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
int face[90000][3];
struct adjacentList{
	int num;//the number of adjacent points
	int vertIndex[100];// the index of adjacent points
	int edgeIndex[100];// the index of edge connecting this point and adjacent point
}amap[50000]; 
int nVert=0;
int nFace=0;
int cs=10; // the scale of coordinates
GLfloat xmin=10.f, ymin=10.f, zmin=10.f;
GLfloat xmax=-10000.f, ymax=-100000.f, zmax=-100000.f;//the min and max of the bounding box coordinates
/**************************************** half-edge struct *******************/

struct HE_face{
	HE_edge* edge; //one of the half-edge bordering the face 
}f[90000];
struct HE_vert{
	float x,y,z; //the vertex cordinates
	float nx,ny,nz;//normal vertex
	HE_edge* edge;//one of the half-edge emanating from the vertex
}v[50000];
struct HE_edge{
	HE_vert* vert;//vertex at the end of the half-edge
	HE_edge* pair;//oppositely oriented half-edge
	HE_face* face;//the incident face
	HE_edge* prev;//previous half-edge around the face 
	HE_edge* next;//next half-edge around the face
}e[300000];

int inArray(int ele, int arr[])
{
	for (int i=0;i<3;i++)
	{
		if(arr[i]==ele)
			return i;
	}
	return 3;
}

boolean samePoint (HE_vert* v1, HE_vert* v2){
	float dist = (v1->x-v2->x)*(v1->x-v2->x)+(v1->y-v2->y)*(v1->y-v2->y)+(v1->z-v2->z)*(v1->z-v2->z);
	if (dist<0.00001)
		return true;
	else
		return false;
}

int inAdjList (int i, int j){
	int adj = 0;
	for (int k=0; k<amap[j].num; k++){
		if (i == amap[j].vertIndex[k]){
			adj = amap[j].edgeIndex[k];
			break;
		}
	}
	return adj;
}

void StructGenerate()
{
	for(int index=1; index<=nVert; index++)
	{
		HE_edge* outgoing_he = v[index].edge;
		HE_edge* curr = outgoing_he;
		float sumNormal[3]={0,0,0}; // the sum of face normals
		float fn[100][3];//store face normal;
		float ver[100][3];//the coordinates of vertex
		float center[3]={v[index].x,v[index].y, v[index].z};//the vertex to be assigned normal
		int i=1;
		int boundFlag = 1;// 1 means this is a boundary vertex
		ver[0][0]=outgoing_he->vert->x;
		ver[0][1]=outgoing_he->vert->y;
		ver[0][2]=outgoing_he->vert->z;
		while(curr->pair!=NULL){
			curr=curr->pair->next;
			ver[i][0]=curr->vert->x;
			ver[i][1]=curr->vert->y;
			ver[i][2]=curr->vert->z;
			i++;
			if( samePoint(curr->vert, outgoing_he->vert) ){
				boundFlag=0;
				break;
			}
		}
		if (boundFlag){
			ver[0][0]=curr->vert->x;
			ver[0][1]=curr->vert->y;
			ver[0][2]=curr->vert->z;
			while(curr->prev->pair!=NULL){
				curr=curr->prev->pair;
				ver[i][0]=curr->vert->x;
				ver[i][1]=curr->vert->y;
				ver[i][2]=curr->vert->z;
				i++;
			}
			curr=curr->next;
			ver[i][0]=curr->vert->x;
			ver[i][1]=curr->vert->y;
			ver[i][2]=curr->vert->z;
			i++;
		}
		for(int k=0;k<i-1;k++){
			// cross product: { u1v2-v1u2 , u2v0-v2u0 , u0v1-v0u1, u=ver[k+1]-ver[0], v=ver[k]-ver[k+1] }
			fn[k][0]=(ver[k+1][1]-center[1])*(ver[k][2]-ver[k+1][2])-(ver[k][1]-ver[k+1][1])*(ver[k+1][2]-center[2]);
			fn[k][1]=(ver[k+1][2]-center[2])*(ver[k][0]-ver[k+1][0])-(ver[k][2]-ver[k+1][2])*(ver[k+1][0]-center[0]);
			fn[k][2]=(ver[k+1][0]-center[0])*(ver[k][1]-ver[k+1][1])-(ver[k][0]-ver[k+1][0])*(ver[k+1][1]-center[1]);
			sumNormal[0]=sumNormal[0]+fn[k][0]/(i-1);
			sumNormal[1]=sumNormal[1]+fn[k][1]/(i-1);
			sumNormal[2]=sumNormal[2]+fn[k][2]/(i-1);
		}
		float vnlen = sqrt(sumNormal[0]*sumNormal[0]+sumNormal[1]*sumNormal[1]+sumNormal[2]*sumNormal[2]);
		v[index].nx=sumNormal[0]/vnlen;
		v[index].ny=sumNormal[1]/vnlen;
		v[index].nz=sumNormal[2]/vnlen;
		printf("the normal of %d vertex is %f, %f, %f \n", index, v[index].nx, v[index].ny, v[index].nz);
	}

}

void Draw_model(int cs)
{
	int i;
	//draw a point model
	  if( obj_type == 404 )
	  {
		  glDisable(GL_LIGHTING);
		  for(i=1;i<=nVert;i++)
		  {
			  glBegin(GL_POINTS);
			    glVertex3f(v[i].x/cs,v[i].y/cs,v[i].z/cs);
	          glEnd();
		  }
		  glEnable(GL_LIGHTING);
	  } 
	  //draw a flat shading model
      else if( obj_type == 402 ) 
	  {
		  glColor3f(0.0f,0.0f,0.0f);
		  for(i=1;i<=nFace;i++)
		  {
			  glBegin(GL_TRIANGLES);
				glNormal3f(f[i].edge->prev->vert->nx,f[i].edge->prev->vert->ny,f[i].edge->prev->vert->nz);
				glVertex3f(f[i].edge->prev->vert->x/cs,f[i].edge->prev->vert->y/cs,f[i].edge->prev->vert->z/cs);
				glNormal3f(f[i].edge->vert->nx,f[i].edge->vert->ny,f[i].edge->vert->nz);
				glVertex3f(f[i].edge->vert->x/cs,f[i].edge->vert->y/cs,f[i].edge->vert->z/cs);
				glNormal3f(f[i].edge->next->vert->nx,f[i].edge->next->vert->ny,f[i].edge->next->vert->nz);
				glVertex3f(f[i].edge->next->vert->x/cs,f[i].edge->next->vert->y/cs,f[i].edge->next->vert->z/cs);
			  glEnd();
		  }
		  glColor3f(r,g,b);
	  } 
	  //draw a smooth shading model
	  else if( obj_type == 401 ) 
	  {
		  glColor3f(0.0f,0.0f,0.0f);
		  for(i=1;i<=nFace;i++)
		  {
			  glBegin(GL_TRIANGLES);
			    glNormal3f(f[i].edge->prev->vert->nx,f[i].edge->prev->vert->ny,f[i].edge->prev->vert->nz);
				glVertex3f(f[i].edge->prev->vert->x/cs,f[i].edge->prev->vert->y/cs,f[i].edge->prev->vert->z/cs);
				glNormal3f(f[i].edge->vert->nx,f[i].edge->vert->ny,f[i].edge->vert->nz);
				glVertex3f(f[i].edge->vert->x/cs,f[i].edge->vert->y/cs,f[i].edge->vert->z/cs);
				glNormal3f(f[i].edge->next->vert->nx,f[i].edge->next->vert->ny,f[i].edge->next->vert->nz);
				glVertex3f(f[i].edge->next->vert->x/cs,f[i].edge->next->vert->y/cs,f[i].edge->next->vert->z/cs);
			  
			  glEnd();
		  }
		  glColor3f(r,g,b);
	  }
	  //draw a wireframe model
	  else if( obj_type == 403 )
	  {
		 glDisable(GL_LIGHTING);
		 for(i=1;i<=nFace;i++)
		  {
			  glBegin(GL_LINE_LOOP);
			    glVertex3f(f[i].edge->prev->vert->x/cs,f[i].edge->prev->vert->y/cs,f[i].edge->prev->vert->z/cs);
				glVertex3f(f[i].edge->vert->x/cs,f[i].edge->vert->y/cs,f[i].edge->vert->z/cs);
				glVertex3f(f[i].edge->next->vert->x/cs,f[i].edge->next->vert->y/cs,f[i].edge->next->vert->z/cs);
			  glEnd();
		  }
		 glEnable(GL_LIGHTING);
	  } 
	  else
	  {};
}

/**************************************** control_cb() *******************/
/* GLUI control callback                                                 */

void control_cb( int control )
{
	// the open file dialog
    OPENFILENAME ofn; 
	// the color dialog
	CHOOSECOLOR CC; 
    static char szFile[256];
    static char myfile[256];
	COLORREF CustColor[16];
	HWND hwnd=NULL;
	//initialize ofn
    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize=sizeof(ofn);
    ofn.hwndOwner=hwnd;
	ofn.lpstrFilter="meshes(*.m)\0*.m;\0\0";
    ofn.nFilterIndex=1;
    ofn.lpstrFile=szFile;
    ofn.nMaxFile=sizeof(szFile);
    ofn.lpstrFileTitle=myfile;
    ofn.nMaxFileTitle=sizeof(myfile);
    ofn.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_EXPLORER;
	//initialize Ipcc
	CC.lStructSize=sizeof(CC);
    CC.hwndOwner=hwnd;
	CC.hInstance=NULL; 
	CC.lpCustColors=CustColor;
    CC.Flags=CC_FULLOPEN;
	if(control==IMPORT_ID){
    if(GetOpenFileName(&ofn))
		{
			int fir, sec, thi;
			char first;
			FILE* file;
			file=fopen(myfile,"r");
			if (file != NULL){
				while((first=fgetc(file))!=EOF){
				   switch(first)
				   {
				   case'V':
				   nVert++;
					fscanf(file,"ertex %d %f %f %f",
						  &nVert,&v[nVert].x,&v[nVert].y,&v[nVert].z);
					//printf ("vertex %d %f %f %f\n",vertex_index,v[nVert].x,v[nVert].y,v[nVert].z);
					break;
				   case'F':
					nFace++;
					 fscanf(file,"ace %d %d %d %d ",
							&nFace,&face[nFace][0],&face[nFace][1],&face[nFace][2]);
					v[face[nFace][0]].edge = &e[3*nFace-2];
					v[face[nFace][1]].edge = &e[3*nFace-1];
					v[face[nFace][2]].edge = &e[3*nFace];
					e[3*nFace-2].vert = &v[face[nFace][1]];
					e[3*nFace-1].vert = &v[face[nFace][2]];
					e[3*nFace].vert = &v[face[nFace][0]];
					f[nFace].edge = &e[3*nFace-2];
					e[3*nFace-2].face = &f[nFace];
					e[3*nFace-1].face = &f[nFace];
					e[3*nFace].face = &f[nFace];
					e[3*nFace-2].next = &e[3*nFace-1];
					e[3*nFace-1].next = &e[3*nFace];
					e[3*nFace].next = &e[3*nFace-2];
					e[3*nFace-2].prev = &e[3*nFace];
					e[3*nFace-1].prev = &e[3*nFace-2];
					e[3*nFace].prev = &e[3*nFace-1];

					fir = inAdjList(face[nFace][0],face[nFace][1]);
					if ( !fir ){
						amap[face[nFace][0]].num++;
						amap[face[nFace][0]].vertIndex[amap[face[nFace][0]].num-1]=face[nFace][1];
						amap[face[nFace][0]].edgeIndex[amap[face[nFace][0]].num-1]=3*nFace-2;
						amap[face[nFace][1]].num++;
						amap[face[nFace][1]].vertIndex[amap[face[nFace][1]].num-1]=face[nFace][0];
						amap[face[nFace][1]].edgeIndex[amap[face[nFace][1]].num-1]=3*nFace-2;
					}
					else{
						e[3*nFace-2].pair = &e[fir];
						e[fir].pair = &e[3*nFace-2];
						printf ("edge %d pair with edge %d \n",3*nFace-2,fir);
					}
					sec = inAdjList(face[nFace][1],face[nFace][2]);
					if ( !sec ){
						amap[face[nFace][1]].num++;
						amap[face[nFace][1]].vertIndex[amap[face[nFace][1]].num-1]=face[nFace][2];
						amap[face[nFace][1]].edgeIndex[amap[face[nFace][1]].num-1]=3*nFace-1;
						amap[face[nFace][2]].num++;
						amap[face[nFace][2]].vertIndex[amap[face[nFace][2]].num-1]=face[nFace][1];
						amap[face[nFace][2]].edgeIndex[amap[face[nFace][2]].num-1]=3*nFace-1;
					}
					else{
						e[3*nFace-1].pair = &e[sec];
						e[sec].pair = &e[3*nFace-1];
						printf ("edge %d pair with edge %d \n",3*nFace-1,sec);
					}
					thi = inAdjList(face[nFace][2],face[nFace][0]);
					if ( !thi ){
						amap[face[nFace][2]].num++;
						amap[face[nFace][2]].vertIndex[amap[face[nFace][2]].num-1]=face[nFace][0];
						amap[face[nFace][2]].edgeIndex[amap[face[nFace][2]].num-1]=3*nFace;
						amap[face[nFace][0]].num++;
						amap[face[nFace][0]].vertIndex[amap[face[nFace][0]].num-1]=face[nFace][2];
						amap[face[nFace][0]].edgeIndex[amap[face[nFace][0]].num-1]=3*nFace;
					}
					else{
						e[3*nFace].pair = &e[thi];
						e[thi].pair = &e[3*nFace];
						printf ("edge %d pair with edge %d \n",3*nFace,thi);
					}
					 break;
				   case '#': do
					  { first =fgetc(file);
					  }while( first != '\n' && first != EOF);
					  break;
				   }
				}
             StructGenerate();
			}
			reload = load = 1;
		}
	}
  if ( control == LIGHT0_ENABLED_ID ) {
    if ( light0_enabled ) {
      glEnable( GL_LIGHT0 );
      light0_spinner->enable();
    }
    else {
      glDisable( GL_LIGHT0 ); 
      light0_spinner->disable();
    }
  }
  else if ( control == LIGHT1_ENABLED_ID ) {
    if ( light1_enabled ) {
      glEnable( GL_LIGHT1 );
      light1_spinner->enable();
    }
    else {
      glDisable( GL_LIGHT1 ); 
      light1_spinner->disable();
    }
  }
  else if ( control == LIGHT0_INTENSITY_ID ) {
    float v[] = { 
      light0_diffuse[0],  light0_diffuse[1],
      light0_diffuse[2],  light0_diffuse[3] };
    
    v[0] *= light0_intensity;
    v[1] *= light0_intensity;
    v[2] *= light0_intensity;

    glLightfv(GL_LIGHT0, GL_DIFFUSE, v );
  }
  else if ( control == LIGHT1_INTENSITY_ID ) {
    float v[] = { 
      light1_diffuse[0],  light1_diffuse[1],
      light1_diffuse[2],  light1_diffuse[3] };
    
    v[0] *= light1_intensity;
    v[1] *= light1_intensity;
    v[2] *= light1_intensity;

    glLightfv(GL_LIGHT1, GL_DIFFUSE, v );
  }
  else if ( control == ENABLE_ID )
  {
    glui2->enable();
  }
  else if ( control == DISABLE_ID )
  {
    glui2->disable();
  }
  else if ( control == SHOW_ID )
  {
    glui2->show();
  }
  else if ( control == HIDE_ID )
  {
    glui2->hide();
  }
 /* else if (control == PROPERTY_ID)
  {
	  obj_type = 400 + obj_type_radio->get_int_val()+1;
  }*/
  else if (control==CAMERA_ID)
  {
	  camera_type=500+camera_type_radio->get_int_val()+1;
  }
  //click on the object color botton
 else if ( control == COLOR_ID )
		//open the choose color dialog
		
			if(ChooseColor( &CC ))
		{
			//set the diffuse light and r,g,b values to change the color of the 3D model
			light1_diffuse[0] = (GLfloat) GetRValue(CC.rgbResult)/(GLfloat) 255;
			light1_diffuse[1] = (GLfloat) GetGValue(CC.rgbResult)/(GLfloat) 255;
			light1_diffuse[2] = (GLfloat) GetBValue(CC.rgbResult)/(GLfloat) 255;
			r = light1_diffuse[0];
			g = light1_diffuse[1];
			b = light1_diffuse[2];
		}
		if( control == PROPERTY_ID )
	{
		//get the current value of the radio button
		obj_type=400+obj_type_radio->get_int_val()+1;
	}
	
}

/**************************************** find max and min vertex of model() *******************/

void getCorners()
{  
	for (int i=1; i<=nVert; i++)
	{
		if (v[i].x/cs>xmax)
			xmax= v[i].x/cs;
		if (v[i].x/cs<xmin)
			xmin= v[i].x/cs;
		if (v[i].y/cs>ymax)
			ymax = v[i].y/cs;
		if (v[i].y/cs<ymin)
			ymin = v[i].y/cs;
		if (v[i].z/cs>zmax)
			zmax = v[i].z/cs;
		if (v[i].z/cs<zmin)
			zmin = v[i].z/cs;
	}
	float box_v=ymax-ymin;
	for(int i=1; i<=nVert; i++)
	{
		v[i].x=0.3*v[i].x/box_v;
		v[i].y=0.3*v[i].y/box_v;
		v[i].z=0.3*v[i].z/box_v;
	}
	
}
void setting(int cs)
{
    model_x=(xmax+xmin)/2;
	model_y=(ymax+ymin)/2;
	model_z=(zmax+zmin)/2;

}
/**************************************** myGlutKeyboard() **********/

void myGlutKeyboard(unsigned char Key, int x, int y)
{
  switch(Key)
  {
  case 27: 
  case 'q':
    exit(0);
    break;
  };
  
  glutPostRedisplay();
}
/***************************************** myGlutMouse() **********/
void myGlutMouse(int button, int button_state, int x, int y )
{
	
	if (button_state== GLUT_DOWN) 
	{
		last_x = x; last_y = y; 
		//press left button,set mode to rotate
		// if (button == GLUT_LEFT_BUTTON)
		//	trans_model_type = MODEL_ROTATION; 
		//press middle button,set mode to translate
	     if (button == GLUT_MIDDLE_BUTTON) 
			trans_model_type = MODEL_TRANSLATION;
		 else if (button == GLUT_LEFT_BUTTON)
		    trans_model_type = MODEL_ROTATION; 
		//press right button,set mode to scale
		else if (button == GLUT_RIGHT_BUTTON) 
			trans_model_type = MODEL_SCALE;
	}
	//no button is pressed
	else if (button_state == GLUT_UP) 
	{
		trans_model_type =MODEL_TRANS_NONE; 
	}
	glutPostRedisplay();

}
/***************************************** myGlutMenu() ***********/

/*void myGlutMenu( int value )
{
  myGlutKeyboard( value, 0, 0 );
  myGlutMouse(value, value, 0, 0 );
}*/


/***************************************** myGlutIdle() ***********/
void myGlutIdle( void )
{
  /* According to the GLUT specification, the current window is 
     undefined during an idle callback.  So we need to explicitly change
     it if necessary */
    if ( glutGetWindow() != main_window ) 
     glutSetWindow(main_window);  

  /*  GLUI_Master.sync_live_all();  -- not needed - nothing to sync in this
                                       application  */

  glutPostRedisplay();
}
/***************************************** myGlutMotion() **********/

void myGlutMotion(int x, int y )
{
    if(trans_model_type== MODEL_ROTATION)
	{
	    x_angle_model+=(GLfloat)(x-last_x);
	    if(x_angle_model>180)
		{x_angle_model-=360;}
	    else if(x_angle_model+=360);
		    {last_x=x;
	          x_angle_model+=(GLfloat)(x-last_x);
			}
		 if(y_angle_model>180)
		 { y_angle_model-=360;}
	    else if(y_angle_model+=360);
		    {
			  // y_angle += 360.0; 
			    last_y=y;
			}
	}
	
	else if(trans_model_type==MODEL_TRANSLATION)
	{      model_x+=(GLfloat)(x-last_x)/20;
	       last_x=x;
		   model_y+=(GLfloat)(y-last_y)/20;
		   last_y=y;
	}
	else if(trans_model_type==MODEL_SCALE)
	{    
	    old_size = scale_size_model;
		scale_size_model *= (1 + (y - last_y)/30.0); 
		if (scale_size_model <0) 
			{
				scale_size_model = old_size; 
		       last_y = y; 
		}
	}
	glutPostRedisplay(); 
}

/**************************************** myGlutReshape() *************/

void myGlutReshape( int x, int y )
{
 int tx, ty, tw, th;
  GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );
  xy_aspect = (float)tw / (float)th;
  glutPostRedisplay();
}

/************************************************** draw_axes() **********/
/* Disables lighting, then draws xyz axes                                */
void draw_axes( float scale )
{
  
	 glDisable(GL_LIGHTING);
	  //draw axe z
	  glColor3f( 1.0f,0.0f,0.0f );
	  quadratic=gluNewQuadric();				
	  gluQuadricNormals(quadratic, GLU_SMOOTH);		
	  gluQuadricTexture(quadratic, GL_FALSE);
	  gluCylinder(quadratic,0.02f,0.02f,axe_z,32,32);
	  glPushMatrix();
	  glTranslatef( 0.0f,0.0f,axe_z);
	  gluCylinder(quadratic,0.04f,0.0f,0.1f,32,32);
	  glPopMatrix();

	  //draw axe x
	  glColor3f( 0.0f,1.0f,0.0f);
	  glPushMatrix();
	  glRotatef(90.0f, 0.0f,1.0f,0.0f);
	  quadratic=gluNewQuadric();				
	  gluQuadricNormals(quadratic, GLU_SMOOTH);		
	  gluQuadricTexture(quadratic, GL_FALSE);
	  gluCylinder(quadratic,0.02f,0.02f,axe_x,32,32);
	  glPushMatrix();
	  glTranslatef( 0.0f,0.0f,axe_x);
	  gluCylinder(quadratic,0.04f,0.0f,0.1f,32,32);
	  glPopMatrix();
	  glPopMatrix();

	  //draw axe y
	  glColor3f( 0.0f,0.0f,1.0f);
	  glPushMatrix();
	  glRotatef(-90.0f, 1.0f,0.0f,0.0f);
	  quadratic=gluNewQuadric();				
	  gluQuadricNormals(quadratic, GLU_SMOOTH);		
	  gluQuadricTexture(quadratic, GL_FALSE);
	  gluCylinder(quadratic,0.02f,0.02f,axe_y,32,32);
	  glPushMatrix();
	  glTranslatef( 0.0f,0.0f,axe_y);
	  gluCylinder(quadratic,0.04f,0.0f,0.1f,32,32);
	  glPopMatrix();
	  glPopMatrix();
	  glColor3f(r,g,b);
	  glEnable(GL_LIGHTING);
}
/************************************************** draw_ground() **********/
void draw_grid(int scale)
{
	glDisable( GL_LIGHTING );

     glPushMatrix();
     glScalef( scale, scale, scale );

      glBegin( GL_LINES );
	  glColor3f( 0.0, 0.0, 0.0 );
	  glLineWidth(2);  
	  int i=0;
	  for(i=0;i<=20;i++)
	  {
		   glVertex3f(-10, 0.5*i, 0 ); 
		   glVertex3f( 10, 0.5*i, 0 );
		   glVertex3f(0.5*i, -10, 0 ); 
		   glVertex3f(0.5*i,  10, 0 );
	  }
	  for(i=0;i<20;i++)
	  {
		   glVertex3f(-10, -0.5*(1+i), 0 ); 
		   glVertex3f( 10, -0.5*(1+i), 0 ); 
		   glVertex3f(-0.5*(1+i), -10, 0 ); 
		   glVertex3f(-0.5*(1+i),  10, 0 );
	  }
	  glEnd();
     glPopMatrix();
     glEnable( GL_LIGHTING );

}
// determine the min and max x,y,z of the model, and use them as the corners of bounding box


void draw_box(int cs)
{
	 glDisable(GL_LIGHTING);
	  glColor3f( 0.5f,0.5f,0.5f );
	   glPushMatrix();
	   getCorners();
      glBegin( GL_LINE_LOOP );
	  glLineWidth(10.0f );
		 glVertex3f(xmin,ymin,zmax);
		 glVertex3f(xmax,ymin,zmax);
		 glVertex3f(xmax,ymax,zmax);
		 glVertex3f(xmin,ymax,zmax);
	 glEnd();
     glBegin( GL_LINE_LOOP );
	  glLineWidth(10.0f );
		 glVertex3f(xmin,ymin,zmin);
		 glVertex3f(xmax,ymin,zmin);
		 glVertex3f(xmax,ymax,zmin);
		 glVertex3f(xmin,ymax,zmin);
	 glEnd();
	 glBegin(GL_LINES);
		 glVertex3f(xmin,ymin,zmin);
		 glVertex3f(xmin,ymin,zmax);
		 glVertex3f(xmax,ymin,zmin);
		 glVertex3f(xmax,ymin,zmax);
		 glVertex3f(xmax,ymax,zmin);
		 glVertex3f(xmax,ymax,zmax);
		 glVertex3f(xmin,ymax,zmin);
		 glVertex3f(xmin,ymax,zmax);
	 glEnd();
	 glPopMatrix();
	 glEnable(GL_LIGHTING);
}

/***************************************** myGlutDisplay() *****************/

void myGlutDisplay( void )
{
  glClearColor( .9f, .9f, .9f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glLoadIdentity();
  glMultMatrixf( lights_rotation );
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glMatrixMode( GL_PROJECTION );
  glEnable(GL_DEPTH_TEST);
  glLoadIdentity();
   {
	   if(camera_type==501){
		    gluPerspective(60.0f, xy_aspect, 0.1f, 100.0f); 
			glMultMatrixf( view_rotate );
	  
	}
   else if(camera_type==502){
	glFrustum( -xy_aspect*.06, xy_aspect*.06, -.06, .06, .1, 100.0 );
	gluLookAt(0.000001, 5, 0, 0, 0, 0, 0, 1, 0);
	glRotatef(-30, 1.0f,  0.0f, 0.0f);
   }
    glMatrixMode( GL_MODELVIEW );
   }
  glLoadIdentity();
  glTranslatef( 0.0f, 0.0f, -2.6f );
  glTranslatef( obj_pos[0], obj_pos[1], -obj_pos[2] ); 
  glRotatef(-75,1.0f,0.0f,0.0f);
  glRotatef(-135,0.0f,0.0f,1.0f);
  if( obj_type == 402)
			  glShadeModel(GL_FLAT);
  else if(  obj_type == 401 )
			  glShadeModel(GL_SMOOTH);
  glColor3f(r,g,b);
  if ( show_axes )
    {
		draw_axes(.52f);
  }
  if(show_grid)
   {
	   draw_grid(1);
  }
  glScalef( scale, scale, scale );
 
  

  if( load == 1 )  
  {
	  if( reload == 1 )
	  {
		  model_x=model_y=0.0f;
		  x_angle_model=y_angle_model=0.0f;
		  scale_size_model=1.0f;
	  }
	   
  
  /*** Now we render object, using the variables 'obj_type', 'segments', and
    'wireframe'.  These are _live_ variables, which are transparently 
    updated by GLUI ***/
 
  glPushMatrix();
    glTranslatef(model_x,model_y,0.0f);
    glRotatef(x_angle_model, 0, 1,0); 
    glRotatef(y_angle_model, 1,0,0);
	glScalef(scale_size_model,scale_size_model,scale_size_model);
   Draw_model(cs);
   if(show_box)
		{
	    draw_box(cs);
		}
  glPopMatrix();
 //printf( "text: %s\n", text ); 
  glEnable( GL_LIGHTING );
	  }
   if(hide_model)
  {
	load=0;
	glutPostRedisplay();
  }
   else{
	   load=1;
   }
   
  reload=0;
  glutPostRedisplay();
  glutSwapBuffers(); 
}


/**************************************** main() ********************/

int main(int argc, char* argv[])
{
  /****************************************/
  /*   Initialize GLUT and create window  */
  /****************************************/

  glutInit(&argc, argv);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition( 50, 50 );
  glutInitWindowSize( 400, 400);
 
  main_window = glutCreateWindow( "mesh" );
  glutDisplayFunc( myGlutDisplay );
  GLUI_Master.set_glutReshapeFunc( myGlutReshape );  
  //GLUI_Master.set_glutKeyboardFunc( myGlutKeyboard );
  //GLUI_Master.set_glutSpecialFunc( NULL );
  GLUI_Master.set_glutMouseFunc( myGlutMouse );
   //glutMouseFunc(Mouse);
  glutMotionFunc( myGlutMotion );

  /****************************************/
  /*       Set up OpenGL lights           */
  /****************************************/

  glEnable(GL_LIGHTING);
  glEnable( GL_NORMALIZE );

  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

  glEnable(GL_LIGHT1);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

  /****************************************/
  /*          Enable z-buferring          */
  /****************************************/

  glEnable(GL_DEPTH_TEST);

  /****************************************/
  /*         Here's the GLUI code         */
  /****************************************/

  //printf( "GLUI version: %3.2f\n", GLUI_Master.get_version() );

  /*** Create the side subwindow ***/
  glui = GLUI_Master.create_glui_subwindow( main_window, 
					    GLUI_SUBWINDOW_RIGHT );
  //file_panel = new GLUI_Panel( glui, "", 1);/*creat file open file panel*/
  new GLUI_Button( glui, "Import", IMPORT_ID, control_cb );
  new GLUI_Button( glui, "Color", COLOR_ID, control_cb );
  obj_panel = new GLUI_Rollout(glui, "Properties", false );

  /***** Control for object params *****/
 
	obj_type_radio = new GLUI_RadioGroup( obj_panel,0,PROPERTY_ID,control_cb); 
	new GLUI_RadioButton( obj_type_radio,"Smooth shading");
	new GLUI_RadioButton( obj_type_radio, "Flat shading");
	new GLUI_RadioButton( obj_type_radio, "Wireframe");
	new GLUI_RadioButton( obj_type_radio, "Points");
	obj_type_radio->set_int_val(0);
   /*** Add option rollout ***/
  GLUI_Rollout *options = new GLUI_Rollout(glui, "Options", true );
  new GLUI_Checkbox( options, "Hide model", & hide_model);
  new GLUI_Checkbox( options, "Draw axes", &show_axes );
   new GLUI_Checkbox ( options, "Draw grid", &show_grid );
   new GLUI_Checkbox ( options, "AABB", &show_box );

   /*** Add camera rollout ***/
   camera_panel = new GLUI_Rollout(glui, "Camera", false );
  camera_type_radio = new GLUI_RadioGroup( camera_panel,0,CAMERA_ID,control_cb); 
	new GLUI_RadioButton( camera_type_radio,"PERSPECTIVE");
	new GLUI_RadioButton( camera_type_radio, "ORTHOGONAL");
	camera_type_radio->set_int_val(0);
  /******** Add some controls for lights ********/
  GLUI_Rollout *roll_lights = new GLUI_Rollout(glui, "Lights", false );
  GLUI_Panel *light0 = new GLUI_Panel( roll_lights, "Light 1" );
  GLUI_Panel *light1 = new GLUI_Panel( roll_lights, "Light 2" );

  new GLUI_Checkbox( light0, "Enabled", &light0_enabled,
                     LIGHT0_ENABLED_ID, control_cb );
  light0_spinner = 
    new GLUI_Spinner( light0, "Intensity:", 
                      &light0_intensity, LIGHT0_INTENSITY_ID,
                      control_cb );
  light0_spinner->set_float_limits( 0.0, 1.0 );
  GLUI_Scrollbar *sb;
  sb = new GLUI_Scrollbar( light0, "Red",GLUI_SCROLL_HORIZONTAL,
                           &light0_diffuse[0],LIGHT0_INTENSITY_ID,control_cb);
  sb->set_float_limits(0,1);
  sb = new GLUI_Scrollbar( light0, "Green",GLUI_SCROLL_HORIZONTAL,
                           &light0_diffuse[1],LIGHT0_INTENSITY_ID,control_cb);
  sb->set_float_limits(0,1);
  sb = new GLUI_Scrollbar( light0, "Blue",GLUI_SCROLL_HORIZONTAL,
                           &light0_diffuse[2],LIGHT0_INTENSITY_ID,control_cb);
  sb->set_float_limits(0,1);
  new GLUI_Checkbox( light1, "Enabled", &light1_enabled,
                     LIGHT1_ENABLED_ID, control_cb );
  light1_spinner = 
    new GLUI_Spinner( light1, "Intensity:",
                      &light1_intensity, LIGHT1_INTENSITY_ID,
                      control_cb );
  light1_spinner->set_float_limits( 0.0, 1.0 );
  sb = new GLUI_Scrollbar( light1, "Red",GLUI_SCROLL_HORIZONTAL,
                           &light1_diffuse[0],LIGHT1_INTENSITY_ID,control_cb);
  sb->set_float_limits(0,1);
  sb = new GLUI_Scrollbar( light1, "Green",GLUI_SCROLL_HORIZONTAL,
                           &light1_diffuse[1],LIGHT1_INTENSITY_ID,control_cb);
  sb->set_float_limits(0,1);
  sb = new GLUI_Scrollbar( light1, "Blue",GLUI_SCROLL_HORIZONTAL,
                           &light1_diffuse[2],LIGHT1_INTENSITY_ID,control_cb);
  sb->set_float_limits(0,1);
  /*** Disable/Enable buttons ***/
  new GLUI_Button( glui, "Disable movement", DISABLE_ID, control_cb );
  new GLUI_Button( glui, "Enable movement", ENABLE_ID, control_cb );
  new GLUI_Button( glui, "Hide", HIDE_ID, control_cb );
  new GLUI_Button( glui, "Show", SHOW_ID, control_cb );
  /****** A 'quit' button *****/
  new GLUI_Button( glui, "Quit", 0,(GLUI_Update_CB)exit );

  /**** Link windows to GLUI, and register idle callback ******/
  
  glui->set_main_gfx_window( main_window );
  /*** Create the bottom subwindow ***/
  glui2 = GLUI_Master.create_glui_subwindow( main_window, 
                                             GLUI_SUBWINDOW_BOTTOM );
  glui2->set_main_gfx_window( main_window );
  /*GLUI_Rotation *view_rot = new GLUI_Rotation(glui2, "GLOBAL", view_rotate );
  view_rot->set_spin( 1.0 );*/
  new GLUI_Column( glui2, false );
  GLUI_Rotation *sph_rot = new GLUI_Rotation(glui2, "Rotation", view_rotate );
  sph_rot->set_spin( .98 );
  new GLUI_Column( glui2, false );
  new GLUI_Column( glui2, false );
  GLUI_Rotation *lights_rot = new GLUI_Rotation(glui2, "Blue Light", lights_rotation );
  lights_rot->set_spin( .82 );
  new GLUI_Column( glui2, false );
  GLUI_Translation *trans_xy = 
    new GLUI_Translation(glui2, "Objects XY", GLUI_TRANSLATION_XY, obj_pos );
  trans_xy->set_speed( .005 );
  new GLUI_Column( glui2, false );
  GLUI_Translation *trans_x = 
    new GLUI_Translation(glui2, "Objects X", GLUI_TRANSLATION_X, obj_pos  );
  trans_x->set_speed( .005 );
  new GLUI_Column( glui2, false );
  GLUI_Translation *trans_y = 
    new GLUI_Translation( glui2, "Objects Y", GLUI_TRANSLATION_Y, &obj_pos[1]);
  trans_y->set_speed( .005 );
 new GLUI_Column( glui2, false );
  GLUI_Translation *trans_z = 
    new GLUI_Translation( glui2, "Objects Z", GLUI_TRANSLATION_Z, &obj_pos[2]);
  trans_z->set_speed( .005 );

#if 0
sphere_pos[2]ister the idle callback with GLUI,not* with GLUT ****/
  GLUI_Master.set_glutIdleFunc( myGlutIdle );

#endif

  /**** Regular GLUT main loop ****/
  glutMainLoop();

  return EXIT_SUCCESS;
}

