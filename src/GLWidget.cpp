/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "GLWidget.h"

#include "MainWindow.h"

using namespace Avogadro;

GLWidget::GLWidget(QWidget *parent ) : QGLWidget(parent), defaultGLEngine(NULL)
{
  printf("Constructor\n");
  loadGLEngines();
}

GLWidget::GLWidget(const QGLFormat &format, QWidget *parent) : QGLWidget(format, parent), defaultGLEngine(NULL)
{
  printf("Constructor\n");
  loadGLEngines();
}

void GLWidget::initializeGL()
{
  printf("Initializing\n");

  glClearColor(0.0, 0.0, 0.0, 1.0);

  //  glShadeModel( GL_SMOOTH );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glEnable( GL_CULL_FACE );

  GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 30.0 };

  glEnable(GL_COLOR_MATERIAL);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  // Used to display semi-transparent relection rectangle
  glBlendFunc(GL_ONE, GL_ONE);

  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );

  GLfloat ambientLight[] = { 0.4, 0.4, 0.4, 1.0 };
  GLfloat diffuseLight[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat position[] = { 0.8, 0.7, 1.0, 0.0 };

  glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
  glLightfv( GL_LIGHT0, GL_SPECULAR, specularLight );
  glLightfv( GL_LIGHT0, GL_POSITION, position );
  glEnable( GL_LIGHT0 );

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glOrtho(10.0, -10.0, 10.0, -10.0, -30.0, 30.0);

  _TranslationVector[0] = 0.0;
  _TranslationVector[1] = 0.0;
  _TranslationVector[2] = 0.0;

  _Scale = 1.0;

}

void GLWidget::resizeGL(int width, int height)
{
  printf("Resizing.\n");
  int side = qMax(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(10.0, -10.0, 10.0, -10.0, -30.0, 30.0);
  glMatrixMode(GL_MODELVIEW);
}

void GLWidget::paintGL()
{
  printf("Painting.\n");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
  glTranslated(_TranslationVector[0], _TranslationVector[1], _TranslationVector[2]);
  glScaled(_Scale, _Scale, _Scale);
  glMultMatrixd(_RotationMatrix);

  view->render();
//X   ((MainWindow*)parent())->getMolecule()->render();

  /*
  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    glCallList(*i);
    */
  glPopMatrix();

  glFlush();
}

void GLWidget::addDisplayList(GLuint dl)
{
  _displayLists.push_back(dl);
  updateGL();
}

void GLWidget::deleteDisplayList(GLuint dl)
{
  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    if (*i == dl)
      _displayLists.erase(i);
}

void GLWidget::mousePressEvent( QMouseEvent * event )
{
  //   if ( event->buttons() & Qt::LeftButton ) {
  //     _leftButtonPressed = true;
  //   }
  //   else if ( event->buttons() & Qt::RightButton ) {
  //     _rightButtonPressed = true;
  //   }
  //   else if ( event->buttons() & Qt::MidButton ) {
  //     _midButtonPressed = true;
  //   }

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos ();
  _initialDraggingPosition = event->pos ();
}

void GLWidget::mouseReleaseEvent( QMouseEvent * event )
{
  //   if( !( event->buttons() & Qt::LeftButton ) ) {
  //       _leftButtonPressed = false;
  //   }
  //   else if ( !( event->buttons() & Qt::RightButton ) ) {
  //     _rightButtonPressed = false;
  //   }
  //   else if ( !( event->buttons() & Qt::MidButton ) ) {
  //     _midButtonPressed = false;
  //   }
}

void GLWidget::mouseMoveEvent( QMouseEvent * event )
{
  QPoint deltaDragging = event->pos() - _lastDraggingPosition;
  _lastDraggingPosition = event->pos();
  if( ( event->pos()
        - _initialDraggingPosition ).manhattanLength() > 2 )
    _movedSinceButtonPressed = true;

  if( event->buttons() & Qt::LeftButton )
  {      
    glPushMatrix();
    glLoadIdentity();
    glRotated( deltaDragging.x(), 0.0, -1.0, 0.0 );
    glRotated( deltaDragging.y(), -1.0, 0.0, 0.0 );
    glMultMatrixd( _RotationMatrix );
    glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix );
    glPopMatrix();
  }
  else if ( event->buttons() & Qt::RightButton )
  {
    deltaDragging = _initialDraggingPosition - event->pos();

    _TranslationVector[0] = deltaDragging.x() / 5.0;
    _TranslationVector[1] = - deltaDragging.y() / 5.0;
  }
  else if ( event->buttons() & Qt::MidButton )
  {
    deltaDragging = _initialDraggingPosition - event->pos();
    int xySum = deltaDragging.x() + deltaDragging.y();

    if (xySum < 0)
      _Scale = deltaDragging.manhattanLength() / 5.0;
    else if (xySum > 0)
      _Scale = 1.0 / deltaDragging.manhattanLength();
  }

  updateGL();
}

void GLWidget::startScreenCoordinates() const
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width(), height(), 0, 0.0, -1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void GLWidget::stopScreenCoordinates() const
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void GLWidget::setView(View *v)
{
  view = v;
}

void GLWidget::setDefaultGLEngine(int i) 
{
  setDefaultGLEngine(glEngines.at(i));
}

void GLWidget::setDefaultGLEngine(GLEngine *e) 
{
  if(e)
  {
    defaultGLEngine = e;
    updateGL();
  }
}

void GLWidget::loadGLEngines()
{
  QDir pluginsDir = QDir(qApp->applicationDirPath());

  if (!pluginsDir.cd("engines") && getenv("AVOGADRO_PLUGINS") != NULL)
    {
      pluginsDir.cd(getenv("AVOGADRO_PLUGINS"));
    }

  qDebug() << "pluginsDir:" << pluginsDir.absolutePath() << endl;
  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    qDebug() << " plugin " << fileName << endl;
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    GLEngineFactory *factory = qobject_cast<GLEngineFactory *>(loader.instance());
    if (factory) {
      GLEngine *engine = factory->createInstance();
      qDebug() << "Found Plugin: " << engine->name() << " - " << engine->description(); 
      if (!defaultGLEngine)
      {
        qDebug() << "Setting Default GLEngine: " << engine->name() << " - " << engine->description(); 
        defaultGLEngine = engine;
      }
      glEngines.append(engine);
    }
  }
  /* this is for static plugins - ignoring it for now.
  foreach (QObject *plugin, QPluginLoader::staticInstances())
  {
    GLEngine *r = qobject_cast<GLEngine *>(plugin);
    if (r)
    {
      qDebug() << "Loaded GLEngine: " << r->name() << endl;
      if( MainWindow::defaultGLEngine == NULL )
        MainWindow::defaultGLEngine = r;
    }
  }
  */
}

