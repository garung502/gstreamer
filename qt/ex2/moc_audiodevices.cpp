/****************************************************************************
** Meta object code from reading C++ file 'audiodevices.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "audiodevices.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audiodevices.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_AudioTest_t {
    QByteArrayData data[13];
    char stringdata0[156];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AudioTest_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AudioTest_t qt_meta_stringdata_AudioTest = {
    {
QT_MOC_LITERAL(0, 0, 9), // "AudioTest"
QT_MOC_LITERAL(1, 10, 11), // "modeChanged"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 3), // "idx"
QT_MOC_LITERAL(4, 27, 13), // "deviceChanged"
QT_MOC_LITERAL(5, 41, 17), // "sampleRateChanged"
QT_MOC_LITERAL(6, 59, 14), // "channelChanged"
QT_MOC_LITERAL(7, 74, 12), // "codecChanged"
QT_MOC_LITERAL(8, 87, 17), // "sampleSizeChanged"
QT_MOC_LITERAL(9, 105, 17), // "sampleTypeChanged"
QT_MOC_LITERAL(10, 123, 13), // "endianChanged"
QT_MOC_LITERAL(11, 137, 4), // "test"
QT_MOC_LITERAL(12, 142, 13) // "populateTable"

    },
    "AudioTest\0modeChanged\0\0idx\0deviceChanged\0"
    "sampleRateChanged\0channelChanged\0"
    "codecChanged\0sampleSizeChanged\0"
    "sampleTypeChanged\0endianChanged\0test\0"
    "populateTable"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AudioTest[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x08 /* Private */,
       4,    1,   67,    2, 0x08 /* Private */,
       5,    1,   70,    2, 0x08 /* Private */,
       6,    1,   73,    2, 0x08 /* Private */,
       7,    1,   76,    2, 0x08 /* Private */,
       8,    1,   79,    2, 0x08 /* Private */,
       9,    1,   82,    2, 0x08 /* Private */,
      10,    1,   85,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AudioTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AudioTest *_t = static_cast<AudioTest *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->modeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->deviceChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sampleRateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->channelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->codecChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->sampleSizeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->sampleTypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->endianChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->test(); break;
        case 9: _t->populateTable(); break;
        default: ;
        }
    }
}

const QMetaObject AudioTest::staticMetaObject = {
    { &AudioDevicesBase::staticMetaObject, qt_meta_stringdata_AudioTest.data,
      qt_meta_data_AudioTest,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *AudioTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioTest::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_AudioTest.stringdata0))
        return static_cast<void*>(const_cast< AudioTest*>(this));
    return AudioDevicesBase::qt_metacast(_clname);
}

int AudioTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AudioDevicesBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
