/****************************************************************************
** MyQGL meta object code from reading C++ file 'myqgl.h'
**
** Created: Wed Sep 14 21:05:28 2005
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.4   edited Jan 21 18:14 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "myqgl.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MyQGL::className() const
{
    return "MyQGL";
}

QMetaObject *MyQGL::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MyQGL( "MyQGL", &MyQGL::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MyQGL::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MyQGL", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MyQGL::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MyQGL", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MyQGL::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QGLWidget::staticMetaObject();
    static const QUMethod slot_0 = {"bind", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "bind()", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"MyQGL", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MyQGL.setMetaObject( metaObj );
    return metaObj;
}

void* MyQGL::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MyQGL" ) )
	return this;
    return QGLWidget::qt_cast( clname );
}

bool MyQGL::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: bind(); break;
    default:
	return QGLWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MyQGL::qt_emit( int _id, QUObject* _o )
{
    return QGLWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MyQGL::qt_property( int id, int f, QVariant* v)
{
    return QGLWidget::qt_property( id, f, v);
}

bool MyQGL::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
