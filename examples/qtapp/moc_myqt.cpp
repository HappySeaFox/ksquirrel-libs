/****************************************************************************
** MyQT meta object code from reading C++ file 'myqt.h'
**
** Created: Wed Sep 14 21:04:33 2005
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.4   edited Jan 21 18:14 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "myqt.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MyQT::className() const
{
    return "MyQT";
}

QMetaObject *MyQT::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MyQT( "MyQT", &MyQT::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MyQT::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MyQT", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MyQT::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MyQT", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MyQT::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QLabel::staticMetaObject();
    static const QUMethod slot_0 = {"bind", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "bind()", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"MyQT", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MyQT.setMetaObject( metaObj );
    return metaObj;
}

void* MyQT::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MyQT" ) )
	return this;
    return QLabel::qt_cast( clname );
}

bool MyQT::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: bind(); break;
    default:
	return QLabel::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MyQT::qt_emit( int _id, QUObject* _o )
{
    return QLabel::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MyQT::qt_property( int id, int f, QVariant* v)
{
    return QLabel::qt_property( id, f, v);
}

bool MyQT::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
