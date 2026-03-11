/****************************************************************************
** Meta object code from reading C++ file 'glwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../glwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'glwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GLWidget_t {
    QByteArrayData data[57];
    char stringdata0[734];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GLWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GLWidget_t qt_meta_stringdata_GLWidget = {
    {
QT_MOC_LITERAL(0, 0, 8), // "GLWidget"
QT_MOC_LITERAL(1, 9, 22), // "intrinsicMatrixChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 22), // "extrinsicMatrixChanged"
QT_MOC_LITERAL(4, 56, 12), // "setIntrinsic"
QT_MOC_LITERAL(5, 69, 3), // "row"
QT_MOC_LITERAL(6, 73, 3), // "col"
QT_MOC_LITERAL(7, 77, 5), // "value"
QT_MOC_LITERAL(8, 83, 18), // "transposeIntrinsic"
QT_MOC_LITERAL(9, 102, 20), // "setExtrinsicRotation"
QT_MOC_LITERAL(10, 123, 26), // "transposeExtrinsicRotation"
QT_MOC_LITERAL(11, 150, 23), // "setExtrinsicTranslation"
QT_MOC_LITERAL(12, 174, 3), // "idx"
QT_MOC_LITERAL(13, 178, 21), // "setExtrinsicForCamera"
QT_MOC_LITERAL(14, 200, 6), // "camIdx"
QT_MOC_LITERAL(15, 207, 27), // "transposeExtrinsicForCamera"
QT_MOC_LITERAL(16, 235, 26), // "swapExtrinsicRowsForCamera"
QT_MOC_LITERAL(17, 262, 4), // "row1"
QT_MOC_LITERAL(18, 267, 4), // "row2"
QT_MOC_LITERAL(19, 272, 26), // "swapExtrinsicColsForCamera"
QT_MOC_LITERAL(20, 299, 4), // "col1"
QT_MOC_LITERAL(21, 304, 4), // "col2"
QT_MOC_LITERAL(22, 309, 19), // "setCameraExtrinsics"
QT_MOC_LITERAL(23, 329, 23), // "std::vector<QMatrix4x4>"
QT_MOC_LITERAL(24, 353, 10), // "extrinsics"
QT_MOC_LITERAL(25, 364, 5), // "setFx"
QT_MOC_LITERAL(26, 370, 2), // "fx"
QT_MOC_LITERAL(27, 373, 5), // "setFy"
QT_MOC_LITERAL(28, 379, 2), // "fy"
QT_MOC_LITERAL(29, 382, 5), // "setCx"
QT_MOC_LITERAL(30, 388, 2), // "cx"
QT_MOC_LITERAL(31, 391, 5), // "setCy"
QT_MOC_LITERAL(32, 397, 2), // "cy"
QT_MOC_LITERAL(33, 400, 8), // "setViewX"
QT_MOC_LITERAL(34, 409, 1), // "x"
QT_MOC_LITERAL(35, 411, 8), // "setViewY"
QT_MOC_LITERAL(36, 420, 1), // "y"
QT_MOC_LITERAL(37, 422, 8), // "setViewZ"
QT_MOC_LITERAL(38, 431, 1), // "z"
QT_MOC_LITERAL(39, 433, 17), // "swapIntrinsicRows"
QT_MOC_LITERAL(40, 451, 17), // "swapIntrinsicCols"
QT_MOC_LITERAL(41, 469, 8), // "drawRays"
QT_MOC_LITERAL(42, 478, 24), // "setCurrentExtrinsicIndex"
QT_MOC_LITERAL(43, 503, 17), // "resetViewToOrigin"
QT_MOC_LITERAL(44, 521, 12), // "setExtrinsic"
QT_MOC_LITERAL(45, 534, 18), // "transposeExtrinsic"
QT_MOC_LITERAL(46, 553, 17), // "swapExtrinsicRows"
QT_MOC_LITERAL(47, 571, 17), // "swapExtrinsicCols"
QT_MOC_LITERAL(48, 589, 15), // "setCameraImages"
QT_MOC_LITERAL(49, 605, 20), // "std::vector<QString>"
QT_MOC_LITERAL(50, 626, 10), // "imagePaths"
QT_MOC_LITERAL(51, 637, 17), // "clearCameraImages"
QT_MOC_LITERAL(52, 655, 15), // "loadSIFTDataset"
QT_MOC_LITERAL(53, 671, 7), // "dirPath"
QT_MOC_LITERAL(54, 679, 22), // "loadSIFTPointCloudFile"
QT_MOC_LITERAL(55, 702, 8), // "filePath"
QT_MOC_LITERAL(56, 711, 22) // "centerViewOnPointCloud"

    },
    "GLWidget\0intrinsicMatrixChanged\0\0"
    "extrinsicMatrixChanged\0setIntrinsic\0"
    "row\0col\0value\0transposeIntrinsic\0"
    "setExtrinsicRotation\0transposeExtrinsicRotation\0"
    "setExtrinsicTranslation\0idx\0"
    "setExtrinsicForCamera\0camIdx\0"
    "transposeExtrinsicForCamera\0"
    "swapExtrinsicRowsForCamera\0row1\0row2\0"
    "swapExtrinsicColsForCamera\0col1\0col2\0"
    "setCameraExtrinsics\0std::vector<QMatrix4x4>\0"
    "extrinsics\0setFx\0fx\0setFy\0fy\0setCx\0"
    "cx\0setCy\0cy\0setViewX\0x\0setViewY\0y\0"
    "setViewZ\0z\0swapIntrinsicRows\0"
    "swapIntrinsicCols\0drawRays\0"
    "setCurrentExtrinsicIndex\0resetViewToOrigin\0"
    "setExtrinsic\0transposeExtrinsic\0"
    "swapExtrinsicRows\0swapExtrinsicCols\0"
    "setCameraImages\0std::vector<QString>\0"
    "imagePaths\0clearCameraImages\0"
    "loadSIFTDataset\0dirPath\0loadSIFTPointCloudFile\0"
    "filePath\0centerViewOnPointCloud"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GLWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      33,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  179,    2, 0x06 /* Public */,
       3,    0,  180,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    3,  181,    2, 0x0a /* Public */,
       8,    0,  188,    2, 0x0a /* Public */,
       9,    3,  189,    2, 0x0a /* Public */,
      10,    0,  196,    2, 0x0a /* Public */,
      11,    2,  197,    2, 0x0a /* Public */,
      13,    4,  202,    2, 0x0a /* Public */,
      15,    1,  211,    2, 0x0a /* Public */,
      16,    3,  214,    2, 0x0a /* Public */,
      19,    3,  221,    2, 0x0a /* Public */,
      22,    1,  228,    2, 0x0a /* Public */,
      25,    1,  231,    2, 0x0a /* Public */,
      27,    1,  234,    2, 0x0a /* Public */,
      29,    1,  237,    2, 0x0a /* Public */,
      31,    1,  240,    2, 0x0a /* Public */,
      33,    1,  243,    2, 0x0a /* Public */,
      35,    1,  246,    2, 0x0a /* Public */,
      37,    1,  249,    2, 0x0a /* Public */,
      39,    2,  252,    2, 0x0a /* Public */,
      40,    2,  257,    2, 0x0a /* Public */,
      41,    0,  262,    2, 0x0a /* Public */,
      42,    1,  263,    2, 0x0a /* Public */,
      43,    0,  266,    2, 0x0a /* Public */,
      44,    3,  267,    2, 0x0a /* Public */,
      45,    0,  274,    2, 0x0a /* Public */,
      46,    2,  275,    2, 0x0a /* Public */,
      47,    2,  280,    2, 0x0a /* Public */,
      48,    1,  285,    2, 0x0a /* Public */,
      51,    0,  288,    2, 0x0a /* Public */,
      52,    1,  289,    2, 0x0a /* Public */,
      54,    1,  292,    2, 0x0a /* Public */,
      56,    0,  295,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Double,    5,    6,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Double,    5,    6,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Double,   12,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Double,   14,    5,    6,    7,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   14,   17,   18,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   14,   20,   21,
    QMetaType::Void, 0x80000000 | 23,   24,
    QMetaType::Void, QMetaType::Int,   26,
    QMetaType::Void, QMetaType::Int,   28,
    QMetaType::Void, QMetaType::Double,   30,
    QMetaType::Void, QMetaType::Double,   32,
    QMetaType::Void, QMetaType::Double,   34,
    QMetaType::Void, QMetaType::Double,   36,
    QMetaType::Void, QMetaType::Double,   38,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   17,   18,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   20,   21,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Double,    5,    6,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   17,   18,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   20,   21,
    QMetaType::Void, 0x80000000 | 49,   50,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   53,
    QMetaType::Void, QMetaType::QString,   55,
    QMetaType::Void,

       0        // eod
};

void GLWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GLWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->intrinsicMatrixChanged(); break;
        case 1: _t->extrinsicMatrixChanged(); break;
        case 2: _t->setIntrinsic((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 3: _t->transposeIntrinsic(); break;
        case 4: _t->setExtrinsicRotation((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 5: _t->transposeExtrinsicRotation(); break;
        case 6: _t->setExtrinsicTranslation((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 7: _t->setExtrinsicForCamera((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 8: _t->transposeExtrinsicForCamera((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->swapExtrinsicRowsForCamera((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 10: _t->swapExtrinsicColsForCamera((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 11: _t->setCameraExtrinsics((*reinterpret_cast< const std::vector<QMatrix4x4>(*)>(_a[1]))); break;
        case 12: _t->setFx((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->setFy((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->setCx((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 15: _t->setCy((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: _t->setViewX((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 17: _t->setViewY((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->setViewZ((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 19: _t->swapIntrinsicRows((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 20: _t->swapIntrinsicCols((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 21: _t->drawRays(); break;
        case 22: _t->setCurrentExtrinsicIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->resetViewToOrigin(); break;
        case 24: _t->setExtrinsic((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 25: _t->transposeExtrinsic(); break;
        case 26: _t->swapExtrinsicRows((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 27: _t->swapExtrinsicCols((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 28: _t->setCameraImages((*reinterpret_cast< const std::vector<QString>(*)>(_a[1]))); break;
        case 29: _t->clearCameraImages(); break;
        case 30: _t->loadSIFTDataset((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 31: _t->loadSIFTPointCloudFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 32: _t->centerViewOnPointCloud(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GLWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GLWidget::intrinsicMatrixChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GLWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GLWidget::extrinsicMatrixChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GLWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_meta_stringdata_GLWidget.data,
    qt_meta_data_GLWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GLWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GLWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GLWidget.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions_3_3_Core"))
        return static_cast< QOpenGLFunctions_3_3_Core*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int GLWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 33)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 33;
    }
    return _id;
}

// SIGNAL 0
void GLWidget::intrinsicMatrixChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GLWidget::extrinsicMatrixChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
